#include <stdio.h> 
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/FreeRTOSConfig.h"
#include "morse_code_characters.h"
#include "network.h"
#include "morse_code.h"
#include "http.h"
#include "index.h"
#define LED_GPIO_PIN 2

void app_main(void)
{
    morse_code_init(LED_GPIO_PIN); // Initialize GPIO for LED

    wifi_init_sta(); // Initialize Wi-Fi

    start_webserver(); // Start the web server
    
    register_index_page(); // Register the index page

    const char *message = "HELLO WORLD";
    int wpm = 5;

    morse_code(message, wpm);

    vTaskDelete(NULL);
}
