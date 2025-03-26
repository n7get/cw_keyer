#include "button.h"
#include "config.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/FreeRTOSConfig.h"
#include "freertos/task.h"
#include "gpio.h"
#include "http.h"
#include "index.h"
#include "message.h"
#include "morse.h"
#include "morse_code_characters.h"
#include "network.h"
#include "nvs_flash.h"
#include "settings.h"
#include "status.h"
#include <stdio.h>

void app_main(void) {
    ESP_LOGI("MAIN", "Starting application");

    load_settings();
    ESP_LOGI("MAIN", "Loaded settings: WPM=%d, AP SSID=%s, STA SSID=%s", wpm,
             ap_ssid, sta_ssid);

    wifi_init();

    if (!start_webserver()) {
        ESP_LOGE("MAIN", "Failed to start web server");
        return;
    }

    button_init();

    register_index_page();
    register_message_endpoints();
    register_morse_endpoints();
    register_settings_endpoints();
    register_status_endpoints();

    morse_code_init();

    queue_morse_code("READY", false);

    ESP_LOGI("MAIN", "Application started");
}
