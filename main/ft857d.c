#include "bcd.h"
#include "cat.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "radio.h"
#include <stdio.h>
#include <string.h>

#define CAT_COMMAND_SIZE 5           // CAT commands and responses are always 5 bytes

// Mode definitions for FT-857D
#define MODE_LSB       0x00  // Lower Sideband
#define MODE_USB       0x01  // Upper Sideband
#define MODE_CW        0x02  // CW
#define MODE_CWR       0x03  // CW Reverse
#define MODE_AM        0x04  // AM
#define MODE_FM        0x08  // FM
#define MODE_DIG       0x0A  // Digital
#define MODE_PKT       0x0C  // Packet
#define MODE_FMN       0x88  // Narrow FM
#define MODE_DIGN      0x8A  // Narrow Digital
#define MODE_PKTN      0x8C  // Narrow Packet

// CAT command definitions for FT-857D
#define CMD_READ_FREQ       0x03  // Read frequency
#define CMD_READ_MODE       0x07  // Read mode
#define CMD_SET_FREQ        0x01  // Set frequency
#define CMD_SET_MODE        0x07  // Set mode
#define CMD_PTT_ON          0x08  // Enable PTT (Push-to-Talk)
#define CMD_PTT_OFF         0x88  // Disable PTT
#define CMD_READ_SMETER     0x02  // Read S-meter
#define CMD_SPLIT_ON        0x02  // Enable split operation
#define CMD_SPLIT_OFF       0x82  // Disable split operation
#define CMD_READ_VFO        0xE7  // Read VFO state
#define CMD_SET_VFO_A       0x81  // Set VFO A
#define CMD_SET_VFO_B       0x82  // Set VFO B
#define CMD_SWAP_VFO        0x87  // Swap VFO A and B
#define CMD_SET_CLAR_ON     0x05  // Enable clarifier
#define CMD_SET_CLAR_OFF    0x85  // Disable clarifier
#define CMD_SET_CLAR_FREQ   0xF5  // Set clarifier frequency
#define CMD_READ_CLAR_FREQ  0xF5  // Read clarifier frequency
#define CMD_READ_MEMORY     0xA0  // Read memory channel
#define CMD_WRITE_MEMORY    0xB0  // Write memory channel
#define CMD_READ_STATUS     0xE7  // Read transceiver status
#define CMD_LOCK_ON         0x00  // Enable lock
#define CMD_LOCK_OFF        0x80  // Disable lock

#define TAG "FT857D"

// Initialize the FT-857D radio
static esp_err_t ft857d_init_radio() {
    if (cat_init() != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize UART for FT-857D");
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "FT-857D initialized");
    return ESP_OK;
}

static esp_err_t get_frequency_and_mode(uint8_t *response) {
    uint8_t command[CAT_COMMAND_SIZE] = {0, 0, 0, 0, CMD_READ_FREQ};

    ESP_LOGI(TAG, "Sending get frequency and mode command: %02X %02X %02X %02X %02X", command[0], command[1], command[2], command[3], command[4]);
    if (cat_send(command, CAT_COMMAND_SIZE) != ESP_OK) {
        return ESP_FAIL;
    }

    if (cat_recv(response, CAT_COMMAND_SIZE) != ESP_OK) {
        ESP_LOGE(TAG, "Invalid get frequency and mode response");
        return ESP_FAIL;
    }
    ESP_LOGI(TAG, "Get frequency and mode response: %02X %02X %02X %02X %02X", response[0], response[1], response[2], response[3], response[4]);

    return ESP_OK;
}

// Get frequency from the FT-857D
static esp_err_t ft857d_get_frequency(uint32_t *frequency) {
    uint8_t response[CAT_COMMAND_SIZE] = {0};

    if (get_frequency_and_mode(response) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get frequency and mode");
        return ESP_FAIL;
    }

    *frequency = bcd_to_uint32(&response[0], CAT_COMMAND_SIZE - 1) * 10;

    ESP_LOGI(TAG, "Frequency: %lu Hz", *frequency);
    return ESP_OK;
}

// Get mode from the FT-857D
static esp_err_t ft857d_get_mode(uint8_t *mode) {
    uint8_t response[CAT_COMMAND_SIZE] = {0, 0, 0, 0, 0};

    if (get_frequency_and_mode(response) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get frequency and mode");
        return ESP_FAIL;
    }

    *mode = response[4];
    return ESP_OK;
}

// Set frequency on the FT-857D
static esp_err_t ft857d_set_frequency(uint32_t frequency) {
    uint8_t command[CAT_COMMAND_SIZE] = {0, 0, 0, 0, CMD_SET_FREQ};

    uint32_to_bcd(frequency / 10, command, CAT_COMMAND_SIZE - 1);

    ESP_LOGI(TAG, "Sending set frequency command: %02X %02X %02X %02X %02X", command[0], command[1], command[2], command[3], command[4]);
    if (cat_send(command, CAT_COMMAND_SIZE) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set frequency");
        return ESP_FAIL;
    }

    uint8_t response = 0;
    if (cat_recv(&response, 1) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read response");
        return ESP_FAIL;
    }
    if (response != 0) {
        ESP_LOGE(TAG, "Command failed: %02X", response);
        return ESP_FAIL;
    }

    return ESP_OK;
}

// Set mode on the FT-857D
static esp_err_t ft857d_set_mode(uint8_t mode) {
    uint8_t command[CAT_COMMAND_SIZE] = {mode, 0, 0, 0, CMD_SET_MODE};

    ESP_LOGI(TAG, "Sending set mode command: %02X %02X %02X %02X %02X", command[0], command[1], command[2], command[3], command[4]);
    if (cat_send(command, CAT_COMMAND_SIZE) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set mode");
        return ESP_FAIL;
    }

    uint8_t response = 0;
    if (cat_recv(&response, 1) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read response");
        return ESP_FAIL;
    }
    if (response != 0) {
        ESP_LOGE(TAG, "Command failed: %02X", response);
        return ESP_FAIL;
    }

    return ESP_OK;
}

// Set PTT (Push-to-Talk) on the FT-857D
static esp_err_t ft857d_set_ptt(bool enable) {
    uint8_t command[CAT_COMMAND_SIZE] = {0, 0, 0, 0, enable ? CMD_PTT_ON : CMD_PTT_OFF};

    ESP_LOGI(TAG, "Sending set ptt command: %02X %02X %02X %02X %02X", command[0], command[1], command[2], command[3], command[4]);
    if (cat_send(command, CAT_COMMAND_SIZE) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set PTT");
        return ESP_FAIL;
    }

    uint8_t response = 0;
    if (cat_recv(&response, 1) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read response");
        return ESP_FAIL;
    }
    if (response != 0) {
        ESP_LOGE(TAG, "Command failed: %02X", response);
        return ESP_FAIL;
    }

    return ESP_OK;
}

// Get power level from the FT-857D
static esp_err_t ft857d_get_power(uint8_t *power) {
    *power = 0;
    ESP_LOGI(TAG, "Get power level is not supported for FT-857D");
    return ESP_OK;
}

// Set power level on the FT-857D
static esp_err_t ft857d_set_power(uint8_t power) {
    ESP_LOGI(TAG, "Set power level is not supported for FT-857D");
    return ESP_OK;
}

static uint8_t ft857d_string_to_mode(const char* mode) {
    if (strcmp(mode, "LSB") == 0) return MODE_LSB;
    if (strcmp(mode, "USB") == 0) return MODE_USB;
    if (strcmp(mode, "CW") == 0) return MODE_CW;
    if (strcmp(mode, "CWR") == 0) return MODE_CWR;
    if (strcmp(mode, "AM") == 0) return MODE_AM;
    if (strcmp(mode, "FM") == 0) return MODE_FM;
    if (strcmp(mode, "DIG") == 0) return MODE_DIG;
    if (strcmp(mode, "PKT") == 0) return MODE_PKT;
    if (strcmp(mode, "FMN") == 0) return MODE_FMN;
    if (strcmp(mode, "DIGN") == 0) return MODE_DIGN;
    if (strcmp(mode, "PKTN") == 0) return MODE_PKTN;

    return 0xFF; // Unknown mode
}

const char* ft857d_mode_to_string(uint8_t mode) {
    switch (mode) {
        case MODE_LSB:   return "LSB";   // Lower Sideband
        case MODE_USB:   return "USB";   // Upper Sideband
        case MODE_CW:    return "CW";    // CW
        case MODE_CWR:   return "CWR";   // CW Reverse
        case MODE_AM:    return "AM";    // AM
        case MODE_FM:    return "FM";    // FM
        case MODE_DIG:   return "DIG";   // Digital
        case MODE_PKT:   return "PKT";   // Packet
        case MODE_FMN:   return "FMN";   // Narrow FM
        case MODE_DIGN:  return "DIGN";  // Narrow Digital
        case MODE_PKTN:  return "PKTN";  // Narrow Packet
        default:         return "UNKNOWN"; // Unknown mode
    }
}

// FT-857D radio operations
const radio_operations_t ft857d_ops = {
    .init_radio = ft857d_init_radio,
    .get_frequency = ft857d_get_frequency,
    .set_frequency = ft857d_set_frequency,
    .get_mode = ft857d_get_mode,
    .set_mode = ft857d_set_mode,
    .get_power = ft857d_get_power,
    .set_power = ft857d_set_power,
    .set_ptt = ft857d_set_ptt,
    .string_to_mode = ft857d_string_to_mode,
    .mode_to_string = ft857d_mode_to_string,
};
