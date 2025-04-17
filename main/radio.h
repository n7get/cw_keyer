#ifndef RADIO_H
#define RADIO_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"

esp_err_t init_radio();
esp_err_t get_frequency(uint32_t *frequency);
esp_err_t set_frequency(uint32_t frequency);
esp_err_t get_mode(uint8_t *mode);
esp_err_t set_mode(uint8_t mode);
esp_err_t set_ptt(bool enable);
esp_err_t get_power(uint8_t *power);
esp_err_t set_power(uint8_t power);

uint8_t string_to_mode(const char* mode_str);
const char* mode_to_string(uint8_t mode);

#endif // RADIO_H
