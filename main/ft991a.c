#include <stdio.h>
#include <string.h>
#include "driver/uart.h"
#include "esp_log.h"
#include "cat.h"
#include "pins.h"

#define TAG "FT991A"
#define UART_NUM UART_NUM_1          // Use UART2 for FT-991A
#define UART_BAUD_RATE 38400         // FT-991A default baud rate
#define BUF_SIZE 1024                // Buffer size for UART
#define CAT_COMMAND_SIZE 5           // CAT commands and responses are always 5 bytes

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

// Initialize the UART driver
static esp_err_t ft991a_init_radio() {
    uart_config_t uart_config = {
        .baud_rate = UART_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    };

    if (uart_param_config(UART_NUM, &uart_config) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure UART parameters");
        return ESP_FAIL;
    }

    if (uart_set_pin(UART_NUM, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set UART pins");
        return ESP_FAIL;
    }

    if (uart_driver_install(UART_NUM, BUF_SIZE * 2, 0, 0, NULL, 0) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to install UART driver");
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "UART initialized for FT-991A");
    return ESP_OK;
}

// Send a CAT command
static esp_err_t send_cat_command(const char *command) {
    int bytes_written = uart_write_bytes(UART_NUM, command, strlen(command));
    if (bytes_written < 0) {
        ESP_LOGE(TAG, "Failed to write CAT command");
        return ESP_FAIL;
    }

    vTaskDelay(pdMS_TO_TICKS(100)); // 100 ms delay
    ESP_LOGI(TAG, "CAT command sent: %s", command);
    return ESP_OK;
}

// Read a CAT response
static esp_err_t read_cat_response(char *response, size_t response_size) {
    int read_len = uart_read_bytes(UART_NUM, (uint8_t *)response, response_size - 1, pdMS_TO_TICKS(1000)); // 1-second timeout
    if (read_len <= 0) {
        ESP_LOGE(TAG, "Failed to read CAT response");
        return ESP_FAIL;
    }

    response[read_len] = '\0'; // Null-terminate the response
    ESP_LOGI(TAG, "CAT response received: %s", response);
    return ESP_OK;
}

// Get frequency from the FT-991A
static esp_err_t ft991a_get_frequency(uint32_t *frequency) {
    char response[BUF_SIZE] = {0};

    if (send_cat_command(CMD_GET_FREQ) != ESP_OK) {
        return ESP_FAIL;
    }

    if (read_cat_response(response, sizeof(response)) != ESP_OK) {
        return ESP_FAIL;
    }

    // Parse frequency from response (9 digits)
    if (sscanf(response, "FA%9lu;", frequency) != 1) {
        ESP_LOGE(TAG, "Failed to parse frequency from response");
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Frequency: %u Hz", *frequency);
    return ESP_OK;
}

// Set frequency on the FT-991A
static esp_err_t ft991a_set_frequency(uint32_t frequency) {
    char command[BUF_SIZE] = {0};

    snprintf(command, sizeof(command), CMD_SET_FREQ, frequency);

    if (send_cat_command(command) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set frequency");
        return ESP_FAIL;
    }

    return ESP_OK;
}

// Get mode from the FT-991A
static esp_err_t ft991a_get_mode(uint8_t *mode) {
    char response[BUF_SIZE] = {0};

    if (send_cat_command(CMD_GET_MODE) != ESP_OK) {
        return ESP_FAIL;
    }

    if (read_cat_response(response, sizeof(response)) != ESP_OK) {
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
static esp_err_t ft991a_set_mode(uint8_t mode) {
    char command[BUF_SIZE] = {0};

    snprintf(command, sizeof(command), CMD_SET_MODE, mode);

    if (send_cat_command(command) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set mode");
        return ESP_FAIL;
    }

    return ESP_OK;
}

// Get power level from the FT-991A
static esp_err_t ft991a_get_power(uint8_t *power) {
    char response[BUF_SIZE] = {0};

    if (send_cat_command(CMD_GET_POWER) != ESP_OK) {
        return ESP_FAIL;
    }

    if (read_cat_response(response, sizeof(response)) != ESP_OK) {
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
static esp_err_t ft991a_set_power(uint8_t power) {
    if (power > 100) {
        ESP_LOGE(TAG, "Invalid power level: %u (must be between 0 and 100)", power);
        return ESP_ERR_INVALID_ARG;
    }

    char command[BUF_SIZE] = {0};
    snprintf(command, sizeof(command), CMD_SET_POWER, power);

    if (send_cat_command(command) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set power level");
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Power level set to: %u%%", power);
    return ESP_OK;
}

// Set PTT (Push-to-Talk) on the FT-991A
static esp_err_t ft991a_set_ptt(bool enable) {
    const char *command = enable ? CMD_PTT_ON : CMD_PTT_OFF;

    if (send_cat_command(command) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set PTT");
        return ESP_FAIL;
    }

    return ESP_OK;
}

// Convert mode string to numeric mode value
static uint8_t ft991a_string_to_mode(const char *mode_str) {
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
static const char *ft991a_mode_to_string(uint8_t mode) {
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

// FT-991A radio operations
const radio_operations_t ft991a_ops = {
    .init_radio = ft991a_init_radio,
    .get_frequency = ft991a_get_frequency,
    .set_frequency = ft991a_set_frequency,
    .get_mode = ft991a_get_mode,
    .set_mode = ft991a_set_mode,
    .get_power = ft991a_get_power,
    .set_power = ft991a_set_power,
    .set_ptt = ft991a_set_ptt,
    .string_to_mode = ft991a_string_to_mode,
    .mode_to_string = ft991a_mode_to_string,
};
