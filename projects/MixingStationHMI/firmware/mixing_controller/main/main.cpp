#include "controllercore.h"
#include "controllerprotocol.h"
#include "modbusserver.h"
#include "wifistation.h"

#include "esp_err.h"
#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

namespace
{

constexpr char LogTag[] = "MixingController";
constexpr std::uint32_t ControllerTaskStackSize = 4096;
constexpr UBaseType_t ControllerTaskPriority = 5;
constexpr std::uint32_t ControllerCycleMilliseconds = 1000;

struct ControllerTaskContext
{
    MixingStation::Firmware::ControllerCore &controller;
    MixingStation::Firmware::ModbusServer &modbusServer;
};

void controllerTask(void *argument)
{
    auto *context =
        static_cast<ControllerTaskContext *>(argument);

    if (context == nullptr) {
        ESP_LOGE(LogTag, "Controller task context is missing.");
        vTaskDelete(nullptr);
        return;
    }

    TickType_t lastWakeTime = xTaskGetTickCount();
    const auto commandRequest =
    context->modbusServer.takeCommandRequest();

if (commandRequest.has_value()) {
    const ControllerProtocol::CommandResult result =
        context->controller.handleCommand(
            commandRequest->command,
            commandRequest->sequence);

    ESP_LOGI(
        LogTag,
        "Handled command %u with sequence %u, result %u.",
        static_cast<unsigned>(commandRequest->command),
        static_cast<unsigned>(commandRequest->sequence),
        static_cast<unsigned>(result));
}
    while (true) {

     const auto commandRequest =
        context->modbusServer.takeCommandRequest();

    if (commandRequest.has_value()) {
        const ControllerProtocol::CommandResult result =
                context->controller.handleCommand(
                commandRequest->command,
                    commandRequest->sequence);

        ESP_LOGI(
            LogTag,
                "Handled command %u with sequence %u, result %u.",
                static_cast<unsigned>(commandRequest->command),
                 static_cast<unsigned>(commandRequest->sequence),
               static_cast<unsigned>(result));
    }

        context->controller.tick();

        const auto currentSnapshot =
            context->controller.snapshot();

        context->modbusServer.publishSnapshot(
            currentSnapshot);

        ESP_LOGI(
            LogTag,
            "Controller heartbeat: %u",
            static_cast<unsigned>(
                currentSnapshot.heartbeat));

        xTaskDelayUntil(
            &lastWakeTime,
            pdMS_TO_TICKS(
                ControllerCycleMilliseconds));
    }
}

} // namespace

extern "C" void app_main()
{
    ESP_LOGI(LogTag, "Mixing controller firmware started");

    ESP_LOGI(
        LogTag,
        "Protocol version: %u",
        static_cast<unsigned>(
            ControllerProtocol::ProtocolVersion));

    static MixingStation::Firmware::ControllerCore controller;
    static MixingStation::Firmware::ModbusServer modbusServer;

    esp_netif_t *networkInterface =
        MixingStation::Firmware::connectWifiStation();

    if (networkInterface == nullptr) {
        ESP_LOGE(
            LogTag,
            "Wi-Fi unavailable. Controller will run without Modbus TCP.");
    } else {
        const esp_err_t modbusResult =
            modbusServer.start(networkInterface);

        if (modbusResult == ESP_OK) {
            ESP_LOGI(
                LogTag,
                "Modbus TCP server started on port %u.",
                static_cast<unsigned>(
                    ControllerProtocol::TcpPort));
        } else {
            ESP_LOGE(
                LogTag,
                "Unable to start Modbus TCP server: %s",
                esp_err_to_name(modbusResult));
        }
    }

    static ControllerTaskContext taskContext{
        controller,
        modbusServer
    };

    const BaseType_t taskCreationResult = xTaskCreate(
        controllerTask,
        "controller",
        ControllerTaskStackSize,
        &taskContext,
        ControllerTaskPriority,
        nullptr);

    if (taskCreationResult != pdPASS) {
        ESP_LOGE(
            LogTag,
            "Failed to create the controller task.");
    }
}