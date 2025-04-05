#include "button.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "http.h"
#include "index.h"
#include "message.h"
#include "morse.h"
#include "network.h"
#include "radio.h"
#include "settings.h"
#include "status.h"

#define TAG "MAIN"

void app_main(void) {
    ESP_LOGI(TAG, "Starting application");
    ESP_LOGI(TAG, "FreeRTOS version: %s", tskKERNEL_VERSION_NUMBER);

    load_settings();
    ESP_LOGI(TAG, "Loaded settings: WPM=%d, AP SSID=%s, STA SSID=%s", wpm,
             ap_ssid, sta_ssid);

    wifi_init();

    if (!start_webserver()) {
        ESP_LOGE(TAG, "Failed to start web server");
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
    init_radio(radio_model);
}
