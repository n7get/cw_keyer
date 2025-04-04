#include "cat.h"
#include "esp_log.h"
#include "esp_err.h"
#include <string.h>

// Forward declaration
static const char* mock_mode_to_string(uint8_t mode);
static uint8_t mock_string_to_mode(const char* mode_str);

#define TAG "MOCK_RADIO"

// Mock radio state
static uint32_t mock_frequency = 7280000;
static char* mock_mode = "LSB";
static bool mock_ptt = false;
static uint8_t mock_power = 50; // Default power level (50%)

static esp_err_t mock_init_radio(void) {
    ESP_LOGI(TAG, "Mock radio initialized");
    return ESP_OK;
}

static esp_err_t mock_get_frequency(uint32_t *frequency) {
    *frequency = mock_frequency;
    ESP_LOGI(TAG, "Mock frequency: %u Hz", *frequency);
    return ESP_OK;
}

static esp_err_t mock_set_frequency(uint32_t frequency) {
    mock_frequency = frequency;
    ESP_LOGI(TAG, "Mock frequency set to: %u Hz", mock_frequency);
    return ESP_OK;
}

static esp_err_t mock_get_mode(uint8_t *mode) {
    *mode = string_to_mode(mock_mode);
    ESP_LOGI(TAG, "Mock mode: %s", mock_mode_to_string(*mode));
    return ESP_OK;
}

static esp_err_t mock_set_mode(uint8_t mode) {
    mock_mode = mode_to_string(mode);
    ESP_LOGI(TAG, "Mock mode set to: %s", mock_mode);
    return ESP_OK;
}

static esp_err_t mock_get_power(uint8_t *power) {
    *power = mock_power;
    ESP_LOGI(TAG, "Mock power level: %u", *power);
    return ESP_OK;
}

static esp_err_t mock_set_power(uint8_t power) {
    mock_power = power;
    ESP_LOGI(TAG, "Mock power level set to: %u", mock_power);
    return ESP_OK;
}

static esp_err_t mock_set_ptt(bool enable) {
    mock_ptt = enable;
    ESP_LOGI(TAG, "Mock PTT %s", enable ? "enabled" : "disabled");
    return ESP_OK;
}

static uint8_t mock_string_to_mode(const char* mode_str) {
    if (strcmp(mode_str, "LSB") == 0) return 0x00;
    if (strcmp(mode_str, "USB") == 0) return 0x01;
    if (strcmp(mode_str, "CW") == 0) return 0x02;
    if (strcmp(mode_str, "FM") == 0) return 0x03;
    return 0xFF; // Unknown mode
}

static const char* mock_mode_to_string(uint8_t mode) {
    switch (mode) {
        case 0x00: return "LSB";
        case 0x01: return "USB";
        case 0x02: return "CW";
        case 0x03: return "FM";
        default: return "UNKNOWN";
    }
}

// Mock radio operations
const radio_operations_t mock_radio_ops = {
    .init_radio = mock_init_radio,
    .get_frequency = mock_get_frequency,
    .set_frequency = mock_set_frequency,
    .get_mode = mock_get_mode,
    .set_mode = mock_set_mode,
    .get_power = mock_get_power,
    .set_power = mock_set_power,
    .set_ptt = mock_set_ptt,
    .string_to_mode = mock_string_to_mode,
    .mode_to_string = mock_mode_to_string,
};
