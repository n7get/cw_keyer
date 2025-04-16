#include "tune.h"
#include "esp_log.h"
#include "gpio.h"
#include "radio.h"
#include <stdio.h>
#include <string.h>

#define TAG "TUNE"
#define TUNE_MODE "CW"
#define TUNE_POWER 5 // Power level during tuning

// Ham band frequency limits (in Hz)
typedef struct {
    uint32_t lower;
    uint32_t upper;
} ham_band_t;

static const ham_band_t ham_bands[] = {
    {1800000, 2000000},     // 160m
    {3500000, 4000000},     // 80m
    {7000000, 7300000},     // 40m
    {10100000, 10150000},   // 30m
    {14000000, 14350000},   // 20m
    {18068000, 18168000},   // 17m
    {21000000, 21450000},   // 15m
    {24890000, 24990000},   // 12m
    {28000000, 29700000},   // 10m
    {50000000, 54000000},   // 6m
    {144000000, 148000000}, // 2m
    {420000000, 450000000}, // 70cm
};

// Function to check if a frequency is near the bottom of a ham band
static bool is_inband(uint32_t frequency) {
    ESP_LOGI(TAG, "Checking if frequency %lu Hz is in band...", frequency);
    for (size_t i = 0; i < sizeof(ham_bands) / sizeof(ham_band_t); i++) {
        if (frequency > ham_bands[i].lower && frequency < ham_bands[i].upper) {
            return true;
        }
    }
    return false;
}

// Start the tuning process
void tune_start(tune_data_t *tune_data) {
    ESP_LOGI(TAG, "Starting tuning process...");

    // Save the current frequency, mode, and power
    if (get_frequency(&tune_data->frequency) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get current frequency");
        return;
    }
    if (get_mode(&tune_data->mode) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get current mode");
        return;
    }
    if (get_power(&tune_data->power) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get current power");
        return;
    }
    ESP_LOGI(TAG, "Saved frequency: %lu Hz, mode: %s, power: %u", tune_data->frequency, mode_to_string(tune_data->mode), tune_data->power);

    // Set mode for tuning
    uint8_t tune_mode = string_to_mode(TUNE_MODE);
    if (set_mode(tune_mode) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set tune mode");
        return;
    }

    // Set power for tuning
    if (set_power(TUNE_POWER) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set tune power");
        return;
    }
    ESP_LOGI(TAG, "Power set to: %u", TUNE_POWER);

    // Adjust frequency for tuning
    const char *mode_str = mode_to_string(tune_data->mode);
    uint32_t tune_offset = 5000;

    if (strcmp(mode_str, "USB") == 0) {
        tune_offset = 3000;
    } else if (strcmp(mode_str, "LSB") == 0) {
        tune_offset = -3000;
    }

    uint32_t new_frequency = tune_data->frequency + tune_offset;
    if (!is_inband(new_frequency)) {
        new_frequency = tune_data->frequency - tune_offset;
        if (!is_inband(new_frequency)) {
            ESP_LOGE(TAG, "Frequency out of band limits");
            return;
        }
    }

    if (set_frequency(new_frequency) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set tuning frequency");
        return;
    }
    ESP_LOGI(TAG, "Tuning frequency set to: %lu Hz", new_frequency);

    key_down();
    ESP_LOGI(TAG, "Key down for tuning...");
}

// Stop the tuning process
void tune_stop(tune_data_t *tune_data) {
    ESP_LOGI(TAG, "Stopping tuning process...");

    key_up();
    ESP_LOGI(TAG, "Key up after tuning...");

    // Restore the original power
    if (set_power(tune_data->power) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to restore power");
        return;
    }
    ESP_LOGI(TAG, "Restored power: %u", tune_data->power);

    // Restore the original mode
    if (set_mode(tune_data->mode) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to restore mode");
        return;
    }
    ESP_LOGI(TAG, "Restored mode: %s", mode_to_string(tune_data->mode));

    // Restore the original frequency
    if (set_frequency(tune_data->frequency) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to restore frequency");
        return;
    }
    ESP_LOGI(TAG, "Restored frequency: %lu Hz", tune_data->frequency);
}
