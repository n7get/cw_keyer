#include "button.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "http.h"
#include "index.h"
#include "message.h"
#include "morse.h"
#include "network.h"
#include "settings.h"
#include "status.h"

void app_main(void) {
    ESP_LOGI("MAIN", "Starting application");
    ESP_LOGI("MAIN", "FreeRTOS version: %s", tskKERNEL_VERSION_NUMBER);

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
