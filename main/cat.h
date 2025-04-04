#ifndef CAT_H
#define CAT_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"

// Radio operations interface
typedef struct {
    esp_err_t (*init_radio)(void);
    esp_err_t (*get_frequency)(uint32_t *frequency);
    esp_err_t (*set_frequency)(uint32_t frequency);
    esp_err_t (*get_mode)(uint8_t *mode);
    esp_err_t (*set_mode)(uint8_t mode);
    esp_err_t (*get_power)(uint8_t *power);
    esp_err_t (*set_power)(uint8_t power);
    esp_err_t (*set_ptt)(bool enable);
    uint8_t (*string_to_mode)(const char* mode);
    const char* (*mode_to_string)(uint8_t mode);
} radio_operations_t;

// Function to initialize the radio with a specific implementation
esp_err_t init_radio(const char* radio_model);

// Function prototypes for radio operations
esp_err_t get_frequency(uint32_t *frequency);
esp_err_t set_frequency(uint32_t frequency);
esp_err_t get_mode(uint8_t *mode);
esp_err_t set_mode(uint8_t mode);
esp_err_t set_ptt(bool enable);
esp_err_t get_power(uint8_t *power);
esp_err_t set_power(uint8_t power);

// Function prototypes for mode conversion
uint8_t string_to_mode(const char* mode_str);
const char* mode_to_string(uint8_t mode);

#endif // CAT_H