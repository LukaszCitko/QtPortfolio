#include "controllercore.h"
#include "controllerprotocol.h"

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

namespace
{

constexpr char LogTag[] = "MixingController";
constexpr std::uint32_t ControllerTaskStackSize = 4096;
constexpr UBaseType_t ControllerTaskPriority = 5;
constexpr std::uint32_t ControllerCycleMilliseconds = 1000;

void controllerTask(void *)
{
    MixingStation::Firmware::ControllerCore controller;

    TickType_t lastWakeTime = xTaskGetTickCount();

    while (true) {
        controller.tick();

        const auto currentSnapshot = controller.snapshot();

        ESP_LOGI(
            LogTag,
            "Controller heartbeat: %u",
            static_cast<unsigned>(currentSnapshot.heartbeat));

        xTaskDelayUntil(
            &lastWakeTime,
            pdMS_TO_TICKS(ControllerCycleMilliseconds));
    }
}

} // namespace

extern "C" void app_main()
{
    ESP_LOGI(LogTag, "Mixing controller firmware started");
    ESP_LOGI(
        LogTag,
        "Protocol version: %u",
        static_cast<unsigned>(ControllerProtocol::ProtocolVersion));

    const BaseType_t taskCreationResult = xTaskCreate(
        controllerTask,
        "controller",
        ControllerTaskStackSize,
        nullptr,
        ControllerTaskPriority,
        nullptr);

    if (taskCreationResult != pdPASS) {
        ESP_LOGE(LogTag, "Failed to create the controller task.");
    }
}