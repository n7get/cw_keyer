#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/FreeRTOSConfig.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "config.h"
#include "http.h"
#include "index.h"
#include "message.h"
#include "morse.h"
#include "morse_code_characters.h"
#include "network.h"
#include "settings.h"
#include "status.h"

#define MORSE_GPIO_PIN 2 // Define the GPIO pin for Morse code output

void app_main(void)
{
    ESP_LOGI("MAIN", "Starting application");

    load_settings();
    ESP_LOGI("MAIN", "Loaded settings: WPM=%d, AP SSID=%s, STA SSID=%s", wpm, ap_ssid, sta_ssid);

    wifi_init();

    if(!start_webserver()) {
        ESP_LOGE("MAIN", "Failed to start web server");
        return;
    }

    register_index_page();
    register_message_endpoints();
    register_morse_endpoints();
    register_settings_endpoints();
    register_status_endpoints();

    morse_code_init(MORSE_GPIO_PIN);

    ESP_LOGI("MAIN", "Application started");
}
