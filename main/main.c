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

#define MORSE_GPIO_PIN 2 // Define the GPIO pin for Morse code output

void app_main(void)
{
    ESP_LOGI("MAIN", "Starting application");

    // Initialize the network
    wifi_init();

    if(!start_webserver()) {
        ESP_LOGE("MAIN", "Failed to start web server");
        return;
    }
    
    register_index_page(); // Register the index page
    api_register_endpoints(); // Register API endpoints

    morse_code_init(MORSE_GPIO_PIN);
    
    ESP_LOGI("MAIN", "Application started");
}
