#include "esp_err.h"
#include "esp_log.h"
#include "radio.h"
#include <string.h>
#include "sdkconfig.h"

#ifdef CONFIG_RADIO_MOCK
#define TAG "MOCK_RADIO"

// Mock radio state
static uint32_t mock_frequency = 7280000;
static char *mock_mode = "LSB";
static bool mock_ptt = false;
static uint8_t mock_power = 50; // Default power level (50%)

esp_err_t init_radio(void) {
    ESP_LOGI(TAG, "Mock radio initialized");
    return ESP_OK;
}

esp_err_t get_frequency(uint32_t *frequency) {
    *frequency = mock_frequency;
    ESP_LOGI(TAG, "Mock frequency: %lu Hz", *frequency);
    return ESP_OK;
}

esp_err_t set_frequency(uint32_t frequency) {
    mock_frequency = frequency;
    ESP_LOGI(TAG, "Mock frequency set to: %lu Hz", mock_frequency);
    return ESP_OK;
}

esp_err_t get_mode(uint8_t *mode) {
    *mode = string_to_mode(mock_mode);
    ESP_LOGI(TAG, "Mock mode: %s", mode_to_string(*mode));
    return ESP_OK;
}

esp_err_t set_mode(uint8_t mode) {
    mock_mode = (char *)mode_to_string(mode);
    ESP_LOGI(TAG, "Mock mode set to: %s", mock_mode);
    return ESP_OK;
}

esp_err_t get_power(uint8_t *power) {
    *power = mock_power;
    ESP_LOGI(TAG, "Mock power level: %u", *power);
    return ESP_OK;
}

esp_err_t set_power(uint8_t power) {
    mock_power = power;
    ESP_LOGI(TAG, "Mock power level set to: %u", mock_power);
    return ESP_OK;
}

esp_err_t set_ptt(bool enable) {
    mock_ptt = enable;
    ESP_LOGI(TAG, "Mock PTT %s", enable ? "enabled" : "disabled");
    return ESP_OK;
}

uint8_t string_to_mode(const char* mode_str) {
    if (strcmp(mode_str, "LSB") == 0) return 0x00;
    if (strcmp(mode_str, "USB") == 0) return 0x01;
    if (strcmp(mode_str, "CW") == 0) return 0x02;
    if (strcmp(mode_str, "FM") == 0) return 0x03;
    return 0xFF; // Unknown mode
}

const char* mode_to_string(uint8_t mode) {
    switch (mode) {
        case 0x00: return "LSB";
        case 0x01: return "USB";
        case 0x02: return "CW";
        case 0x03: return "FM";
        default: return "UNKNOWN";
    }
}
#endif // CONFIG_RADIO_MOCK

