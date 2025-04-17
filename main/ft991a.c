#include "cat.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "pins.h"
#include "radio.h"
#include <stdio.h>
#include <string.h>
#include "sdkconfig.h"

#ifdef CONFIG_RADIO_FT991A
#define TAG "FT991A"

// CAT command definitions for FT-991A
#define CMD_GET_FREQ       "FA;"     // Get frequency
#define CMD_SET_FREQ       "FA%09lu;" // Set frequency (9 digits)
#define CMD_GET_MODE       "MD;"     // Get mode
#define CMD_SET_MODE       "MD%01u;" // Set mode (1 digit)
#define CMD_PTT_ON         "TX0;"    // Enable PTT
#define CMD_PTT_OFF        "TX1;"    // Disable PTT

// CAT command definitions for power control
#define CMD_GET_POWER      "PC;"     // Get power level
#define CMD_SET_POWER      "PC%03u;" // Set power level (3 digits)

// Mode definitions for FT-991A
#define MODE_LSB       1  // Lower Sideband
#define MODE_USB       2  // Upper Sideband
#define MODE_CW        3  // CW
#define MODE_CWR       7  // CW Reverse
#define MODE_AM        5  // AM
#define MODE_FM        4  // FM
#define MODE_DIG       9  // Digital
#define MODE_PKT       10 // Packet

#define RESP_BUF_SIZE 64
static char command[RESP_BUF_SIZE];
static char response[RESP_BUF_SIZE];

// Initialize the UART driver
esp_err_t init_radio() {
    if (cat_init() != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize UART for FT-857D");
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "FT-991A initialized");
    return ESP_OK;
}

// Get frequency from the FT-991A
esp_err_t get_frequency(uint32_t *frequency) {
    if (cat_send((uint8_t *)CMD_GET_FREQ, strlen(CMD_GET_FREQ)) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to send get frequency command");
        return ESP_FAIL;
    }

    if (cat_recv_until((uint8_t *)response, sizeof(response), ';') != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read get frequency response");
        return ESP_FAIL;
    }

    // Parse frequency from response (9 digits)
    if (sscanf(response, "FA%9lu;", frequency) != 1) {
        ESP_LOGE(TAG, "Failed to parse frequency from response");
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Frequency: %lu Hz", *frequency);
    return ESP_OK;
}

// Set frequency on the FT-991A
esp_err_t set_frequency(uint32_t frequency) {
    char command[RESP_BUF_SIZE] = {0};

    size_t len = snprintf(command, sizeof(command), CMD_SET_FREQ, frequency);

    if (cat_send((uint8_t *)command, len) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set frequency");
        return ESP_FAIL;
    }
    ESP_LOGI(TAG, "Set frequency command sent: %s", command);

    return ESP_OK;
}

// Get mode from the FT-991A
esp_err_t get_mode(uint8_t *mode) {
    if (cat_send((uint8_t *)CMD_GET_MODE, strlen(CMD_GET_MODE)) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to send get mode command");
        return ESP_FAIL;
    }

    if (cat_recv_until((uint8_t *)response, sizeof(response), ';') != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read get mode response");
        return ESP_FAIL;
    }

    // Parse mode from response (1 digit)
    if (sscanf(response, "MD%1hhu;", mode) != 1) {
        ESP_LOGE(TAG, "Failed to parse mode from response");
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Mode: %u", *mode);
    return ESP_OK;
}

// Set mode on the FT-991A
esp_err_t set_mode(uint8_t mode) {
    char command[RESP_BUF_SIZE] = {0};

    size_t len = snprintf(command, sizeof(command), CMD_SET_MODE, mode);

    if (cat_send((uint8_t *)command, len) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set mode command");
        return ESP_FAIL;
    }
    ESP_LOGI(TAG, "Set mode command sent: %s", command);

    return ESP_OK;
}

// Get power level from the FT-991A
esp_err_t get_power(uint8_t *power) {
    if (cat_send((uint8_t *)CMD_GET_POWER, strlen(CMD_GET_POWER)) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to send get power command");
        return ESP_FAIL;
    }

    if (cat_recv_until((uint8_t *)response, sizeof(response), ';') != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read get power response");
        return ESP_FAIL;
    }

    // Parse power level from response (3 digits)
    unsigned int power_level;
    if (sscanf(response, "PC%03u;", &power_level) != 1) {
        ESP_LOGE(TAG, "Failed to parse power level from response");
        return ESP_FAIL;
    }

    *power = (uint8_t)power_level;
    ESP_LOGI(TAG, "Power level: %u%%", *power);
    return ESP_OK;
}

// Set power level on the FT-991A
esp_err_t set_power(uint8_t power) {
    if (power > 100) {
        ESP_LOGE(TAG, "Invalid power level: %u (must be between 0 and 100)", power);
        return ESP_ERR_INVALID_ARG;
    }

    size_t len = snprintf(command, sizeof(command), CMD_SET_POWER, power);

    if (cat_send((uint8_t *)command, len) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set power command");
        return ESP_FAIL;
    }
    ESP_LOGI(TAG, "Set power command sent: %s", command);

    ESP_LOGI(TAG, "Power level set to: %u%%", power);
    return ESP_OK;
}

// Set PTT (Push-to-Talk) on the FT-991A
esp_err_t set_ptt(bool enable) {
    strcpy(command, enable ? CMD_PTT_ON : CMD_PTT_OFF);

    if (cat_send((uint8_t *)command, strlen(command)) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to send PTT command");
        return ESP_FAIL;
    }
    ESP_LOGI(TAG, "PTT command sent: %s", command);

    return ESP_OK;
}

// Convert mode string to numeric mode value
uint8_t string_to_mode(const char *mode_str) {
    if (strcmp(mode_str, "LSB") == 0) return MODE_LSB;
    if (strcmp(mode_str, "USB") == 0) return MODE_USB;
    if (strcmp(mode_str, "CW") == 0) return MODE_CW;
    if (strcmp(mode_str, "CWR") == 0) return MODE_CWR;
    if (strcmp(mode_str, "AM") == 0) return MODE_AM;
    if (strcmp(mode_str, "FM") == 0) return MODE_FM;
    if (strcmp(mode_str, "DIG") == 0) return MODE_DIG;
    if (strcmp(mode_str, "PKT") == 0) return MODE_PKT;
    return 0xFF; // Unknown mode
}

// Convert numeric mode value to mode string
const char *mode_to_string(uint8_t mode) {
    switch (mode) {
        case MODE_LSB: return "LSB";
        case MODE_USB: return "USB";
        case MODE_CW: return "CW";
        case MODE_CWR: return "CWR";
        case MODE_AM: return "AM";
        case MODE_FM: return "FM";
        case MODE_DIG: return "DIG";
        case MODE_PKT: return "PKT";
        default: return "UNKNOWN";
    }
}
#endif // CONFIG_RADIO_FT991A

