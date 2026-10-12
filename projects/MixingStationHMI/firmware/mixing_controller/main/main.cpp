#include "controllercore.h"
#include "hardwareinputs.h"
#include "hardwareoutputs.h"
#include "controllerprotocol.h"
#include "modbusserver.h"
#include "processconfiguration.h"
#include "simulatedprocessinputs.h"
#include "temperaturesensor.h"
#include "wifistation.h"


#include "esp_err.h"
#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include <cmath>

namespace
{

constexpr char LogTag[] = "MixingController";
constexpr std::uint32_t ControllerTaskStackSize = 4096;
constexpr UBaseType_t ControllerTaskPriority = 5;
constexpr std::uint32_t HeartbeatLogIntervalCycles = 1000U / ProcessConfiguration::SimulationStepMilliseconds;

constexpr std::uint32_t TemperatureTaskStackSize = 4096;
constexpr UBaseType_t TemperatureTaskPriority = 4;
constexpr std::uint32_t TemperatureReadIntervalMilliseconds = 1000;

constexpr UBaseType_t TemperatureQueueLength = 1;
constexpr float MinimumSupportedTemperatureCelsius = 0.0F;
constexpr float MaximumSupportedTemperatureCelsius = 125.0F;

struct TemperatureTaskContext
{
    MixingStation::Firmware::TemperatureSensor &temperatureSensor;
    QueueHandle_t temperatureQueue;
};

struct ControllerTaskContext
{
    MixingStation::Firmware::ControllerCore &controller;
    MixingStation::Firmware::ModbusServer &modbusServer;
    MixingStation::Firmware::HardwareInputs &hardwareInputs;
    MixingStation::Firmware::HardwareOutputs &hardwareOutputs;
    MixingStation::Firmware::SimulatedProcessInputs &simulatedInputs;
    QueueHandle_t temperatureQueue;
    std::uint16_t latestTemperatureTenthsCelsius{250};
    bool lastReportedMixerConnected{false};
};

void temperatureTask(void *argument)
{
    auto *context = static_cast<TemperatureTaskContext *>(argument);

    if (context == nullptr || context->temperatureQueue == nullptr)
    {
        ESP_LOGE(LogTag, "Temperature task context is invalid.");
        vTaskDelete(nullptr);
        return;
    }

    TickType_t lastWakeTime = xTaskGetTickCount();

    while (true)
    {
        float temperatureCelsius = 0.0F;

        const esp_err_t readResult = context->temperatureSensor.readTemperature(temperatureCelsius);

        if (readResult == ESP_OK && temperatureCelsius >= MinimumSupportedTemperatureCelsius && temperatureCelsius <= MaximumSupportedTemperatureCelsius)
        {
            const std::uint16_t temperatureTenthsCelsius = static_cast<std::uint16_t>(std::lround(temperatureCelsius * 10.0F));
            const BaseType_t queueResult = xQueueOverwrite(context->temperatureQueue, &temperatureTenthsCelsius);

            if (queueResult == pdPASS)
            {
                ESP_LOGI(LogTag, "Tank temperature: %.2f C", static_cast<double>(temperatureCelsius));
            }
            else
            {
                ESP_LOGE(LogTag, "Unable to send temperature to the controller task.");
            }
        }
        else if (readResult != ESP_OK)
        {
            ESP_LOGE(LogTag, "Unable to read tank temperature: %s", esp_err_to_name(readResult));
        }
        else
        {
            ESP_LOGE(LogTag, "Temperature is outside the supported process range: %.2f C", static_cast<double>(temperatureCelsius));
        }

        xTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(TemperatureReadIntervalMilliseconds));
    }
}

void controllerTask(void *argument)
{
    auto *context = static_cast<ControllerTaskContext *>(argument);

    if (context == nullptr)
    {
        ESP_LOGE(LogTag, "Controller task context is missing.");
        vTaskDelete(nullptr);
        return;
    }

    TickType_t lastWakeTime = xTaskGetTickCount();

    while (true)
    {
        context->hardwareInputs.tick(ProcessConfiguration::SimulationStepMilliseconds);

        const bool mixerConnected = context->hardwareInputs.mixerConnected();

        if (mixerConnected != context->lastReportedMixerConnected)
        {
            context->lastReportedMixerConnected = mixerConnected;
            ESP_LOGI(LogTag, "Mixer connection changed: %s", mixerConnected ? "CONNECTED" : "DISCONNECTED");
        }

        const auto setpoints = context->modbusServer.readSetpoints();

        if (setpoints.has_value())
        {
            context->controller.updateSetpoints(*setpoints);
        }

        const auto commandRequest = context->modbusServer.takeCommandRequest();

        if (commandRequest.has_value())
        {
            const ControllerProtocol::CommandResult result =
                context->controller.handleCommand(commandRequest->command, commandRequest->sequence);

            ESP_LOGI(LogTag, "Handled command %u with sequence %u, result %u.",
                static_cast<unsigned>(commandRequest->command),
                static_cast<unsigned>(commandRequest->sequence),
                static_cast<unsigned>(result));
        }
        const auto controllerOutputs = context->controller.snapshot();
        context->simulatedInputs.tick(controllerOutputs, ProcessConfiguration::SimulationStepMilliseconds);

        std::uint16_t measuredTemperatureTenthsCelsius = 0;

        if (context->temperatureQueue != nullptr && xQueueReceive(context->temperatureQueue, &measuredTemperatureTenthsCelsius, 0) == pdTRUE)
        {
            context->latestTemperatureTenthsCelsius = measuredTemperatureTenthsCelsius;
        }

        auto processInputs = context->simulatedInputs.inputs();
        processInputs.temperatureTenthsCelsius = context->latestTemperatureTenthsCelsius;
        processInputs.mixerConnected = mixerConnected;

        context->controller.updateInputs(processInputs);
        context->controller.tick();

        const auto currentSnapshot = context->controller.snapshot();
        context->modbusServer.publishSnapshot(currentSnapshot);

        const esp_err_t outputResult = context->hardwareOutputs.applyEquipmentStatus(currentSnapshot.equipmentStatusMask);

        if (outputResult != ESP_OK)
        {
            ESP_LOGE(LogTag, "Unable to update hardware outputs: %s", esp_err_to_name(outputResult));
        }

        const esp_err_t faultOutputResult = context->hardwareOutputs.applyFaultStatus(currentSnapshot.faultMask);

        if (faultOutputResult != ESP_OK)
        {
            ESP_LOGE(LogTag, "Unable to update fault outputs: %s", esp_err_to_name(faultOutputResult));
        }

        if ((currentSnapshot.heartbeat % HeartbeatLogIntervalCycles) == 0U)
        {
            ESP_LOGI(LogTag, "Controller heartbeat: %u", static_cast<unsigned>(currentSnapshot.heartbeat));
        }
        xTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(ProcessConfiguration::SimulationStepMilliseconds));
    }
}

} // namespace

extern "C" void app_main()
{
    ESP_LOGI(LogTag, "Mixing controller firmware started");
    ESP_LOGI(LogTag,  "Protocol version: %u", static_cast<unsigned>(ControllerProtocol::ProtocolVersion));

    static MixingStation::Firmware::ControllerCore controller;
    static MixingStation::Firmware::HardwareInputs hardwareInputs;
    static MixingStation::Firmware::HardwareOutputs hardwareOutputs;
    static MixingStation::Firmware::ModbusServer modbusServer;
    static MixingStation::Firmware::SimulatedProcessInputs simulatedInputs;
    static MixingStation::Firmware::TemperatureSensor temperatureSensor;

    static QueueHandle_t temperatureQueue = xQueueCreate(TemperatureQueueLength, sizeof(std::uint16_t));

    if (temperatureQueue == nullptr)
    {
        ESP_LOGE(LogTag, "Unable to create the temperature queue.");
    }

    const esp_err_t hardwareInitializationResult = hardwareOutputs.initialize();
    if (hardwareInitializationResult != ESP_OK)
    {
        ESP_LOGE(LogTag, "Unable to initialize hardware outputs: %s", esp_err_to_name(hardwareInitializationResult));
        return;
    }
    ESP_LOGI(LogTag, "Hardware outputs initialized.");

    const esp_err_t hardwareInputInitializationResult = hardwareInputs.initialize();
    if (hardwareInputInitializationResult != ESP_OK)
    {
        ESP_LOGE(LogTag, "Unable to initialize hardware inputs: %s", esp_err_to_name(hardwareInputInitializationResult));
        return;
    }
    ESP_LOGI(LogTag, "Hardware inputs initialized.");

    const esp_err_t temperatureSensorResult = temperatureSensor.initialize();
    if (temperatureSensorResult != ESP_OK)
    {
        ESP_LOGE(LogTag, "Unable to initialize temperature sensor: %s", esp_err_to_name(temperatureSensorResult));
    }
    else
    {
        ESP_LOGI(LogTag, "Temperature sensor initialized.");

        static TemperatureTaskContext temperatureTaskContext{temperatureSensor, temperatureQueue};
        const BaseType_t temperatureTaskCreationResult = xTaskCreate(temperatureTask, "temperature", TemperatureTaskStackSize, &temperatureTaskContext, TemperatureTaskPriority, nullptr);

        if (temperatureTaskCreationResult != pdPASS)
        {
            ESP_LOGE(LogTag, "Failed to create the temperature sensor task.");
        }
    }

    esp_netif_t *networkInterface = MixingStation::Firmware::connectWifiStation();

    if (networkInterface == nullptr) {
        ESP_LOGE(LogTag, "Wi-Fi unavailable. Controller will run without Modbus TCP.");
    }
    else {
        const esp_err_t modbusResult = modbusServer.start(networkInterface);

        if (modbusResult == ESP_OK) {
            ESP_LOGI(LogTag,  "Modbus TCP server started on port %u.", static_cast<unsigned>(ControllerProtocol::TcpPort));
        }
        else {
            ESP_LOGE(LogTag, "Unable to start Modbus TCP server: %s", esp_err_to_name(modbusResult));
        }
    }

    static ControllerTaskContext taskContext{controller, modbusServer, hardwareInputs, hardwareOutputs, simulatedInputs, temperatureQueue};
    const BaseType_t taskCreationResult = xTaskCreate(controllerTask, "controller", ControllerTaskStackSize,  &taskContext, ControllerTaskPriority, nullptr);

    if (taskCreationResult != pdPASS) {
        ESP_LOGE(LogTag, "Failed to create the controller task.");
    }
}