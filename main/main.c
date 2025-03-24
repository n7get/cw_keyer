#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/FreeRTOSConfig.h"
#include "morse_code_characters.h"
#include "esp_log.h"
#include "network.h"
#include "morse_code.h"
#include "http.h"
#include "index.h"
#include "api.h"
#include "nvs_flash.h"
#include "config.h"
#include "settings.h"

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

    api_init();
    register_index_page();
    register_settings_endpoint();

    morse_code_init(MORSE_GPIO_PIN);

    ESP_LOGI("MAIN", "Application started");
}
