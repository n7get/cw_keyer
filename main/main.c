#include "button.h"
#include "esp_log.h"
#include "esp_spiffs.h"
#include "freertos/FreeRTOS.h"
#include "http.h"
#include "message.h"
#include "morse.h"
#include "network.h"
#include "radio.h"
#include "settings.h"
#include "status.h"

#define TAG "MAIN"

void mount_spiffs(void) {
    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/spiffs",
        .partition_label = "html",
        .max_files = 5,
        .format_if_mount_failed = true
    };

    esp_err_t ret = esp_vfs_spiffs_register(&conf);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount or format filesystem");
        } else if (ret == ESP_ERR_NOT_FOUND) {
            ESP_LOGE(TAG, "Failed to find SPIFFS partition");
        } else {
            ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
        }
        return;
    }

    size_t total = 0, used = 0;
    ret = esp_spiffs_info(conf.partition_label, &total, &used);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get SPIFFS partition information (%s)", esp_err_to_name(ret));
    } else {
        ESP_LOGI(TAG, "SPIFFS partition size: total: %d, used: %d", total, used);
    }
}

void app_main(void) {
    ESP_LOGI(TAG, "Starting application");
    ESP_LOGI(TAG, "FreeRTOS version: %s", tskKERNEL_VERSION_NUMBER);

    mount_spiffs();

    load_settings();
    ESP_LOGI(TAG, "Loaded settings: WPM=%d, AP SSID=%s, STA SSID=%s", wpm,
             ap_ssid, sta_ssid);

    wifi_init();

    if (!start_webserver()) {
        ESP_LOGE(TAG, "Failed to start web server");
        return;
    }

    button_init();

    register_message_endpoints();
    register_morse_endpoints();
    register_settings_endpoints();
    register_status_endpoints();

    morse_code_init();

    queue_morse_code("READY", false);

    ESP_LOGI("MAIN", "Application started");
    init_radio(radio_model);
}
