#include "wifistation.h"

#include "esp_event.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "nvs_flash.h"

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

#include <cstring>
#include <cstddef>
#include <cstdint>

namespace
{

constexpr char LogTag[] = "WifiStation";

constexpr EventBits_t ConnectedBit = BIT0;
constexpr EventBits_t FailedBit = BIT1;

EventGroupHandle_t connectionEvents{nullptr};
std::size_t retryCount{0};

void handleWifiEvent(
    void *,
    esp_event_base_t eventBase,
    std::int32_t eventId,
    void *eventData)
{
    if (eventBase == WIFI_EVENT
        && eventId == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
        return;
    }

    if (eventBase == WIFI_EVENT
        && eventId == WIFI_EVENT_STA_DISCONNECTED) {
        if (retryCount
            < static_cast<std::size_t>(
                CONFIG_MIXING_WIFI_MAXIMUM_RETRY)) {
            ++retryCount;
            esp_wifi_connect();

            ESP_LOGW(
                LogTag,
                "Wi-Fi connection retry: %u",
                static_cast<unsigned>(retryCount));
        } else {
            xEventGroupSetBits(connectionEvents, FailedBit);
        }

        return;
    }

    if (eventBase == IP_EVENT
        && eventId == IP_EVENT_STA_GOT_IP) {
        const auto *event =
            static_cast<const ip_event_got_ip_t *>(eventData);

        retryCount = 0;

        ESP_LOGI(
            LogTag,
            "Wi-Fi connected, IP address: " IPSTR,
            IP2STR(&event->ip_info.ip));

        xEventGroupSetBits(connectionEvents, ConnectedBit);
    }
}

void initializeNonVolatileStorage()
{
    esp_err_t result = nvs_flash_init();

    if (result == ESP_ERR_NVS_NO_FREE_PAGES
        || result == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        result = nvs_flash_init();
    }

    ESP_ERROR_CHECK(result);
}

} // namespace

namespace MixingStation::Firmware
{

esp_netif_t *connectWifiStation()
{
    initializeNonVolatileStorage();

    connectionEvents = xEventGroupCreate();
    ESP_ERROR_CHECK(connectionEvents == nullptr
                        ? ESP_ERR_NO_MEM
                        : ESP_OK);

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    esp_netif_t *networkInterface =
        esp_netif_create_default_wifi_sta();

    ESP_ERROR_CHECK(networkInterface == nullptr
                        ? ESP_FAIL
                        : ESP_OK);

    wifi_init_config_t initializationConfiguration =
        WIFI_INIT_CONFIG_DEFAULT();

    ESP_ERROR_CHECK(
        esp_wifi_init(&initializationConfiguration));

    ESP_ERROR_CHECK(
        esp_event_handler_register(
            WIFI_EVENT,
            ESP_EVENT_ANY_ID,
            &handleWifiEvent,
            nullptr));

    ESP_ERROR_CHECK(
        esp_event_handler_register(
            IP_EVENT,
            IP_EVENT_STA_GOT_IP,
            &handleWifiEvent,
            nullptr));

    wifi_config_t wifiConfiguration{};

    std::strncpy(
        reinterpret_cast<char *>(
            wifiConfiguration.sta.ssid),
        CONFIG_MIXING_WIFI_SSID,
        sizeof(wifiConfiguration.sta.ssid) - 1);

    std::strncpy(
        reinterpret_cast<char *>(
            wifiConfiguration.sta.password),
        CONFIG_MIXING_WIFI_PASSWORD,
        sizeof(wifiConfiguration.sta.password) - 1);

    wifiConfiguration.sta.threshold.authmode =
        WIFI_AUTH_WPA2_PSK;

    wifiConfiguration.sta.pmf_cfg.capable = true;
    wifiConfiguration.sta.pmf_cfg.required = false;

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));

    ESP_ERROR_CHECK(
        esp_wifi_set_config(
            WIFI_IF_STA,
            &wifiConfiguration));

    ESP_ERROR_CHECK(esp_wifi_start());

    const EventBits_t result = xEventGroupWaitBits(
        connectionEvents,
        ConnectedBit | FailedBit,
        pdFALSE,
        pdFALSE,
        portMAX_DELAY);

    if ((result & ConnectedBit) != 0) {
        return networkInterface;
    }

    ESP_LOGE(
        LogTag,
        "Unable to connect to the configured Wi-Fi network.");

    return nullptr;
}

} // namespace MixingStation::Firmware
