#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>
#include "esp_err.h"

esp_err_t set_u8(const char *key, uint8_t value);
esp_err_t get_u8(const char *key, uint8_t *value);
esp_err_t set_u32(const char *key, uint32_t value); // Add set_u32 prototype
esp_err_t get_u32(const char *key, uint32_t *value); // Add get_u32 prototype
esp_err_t set_string(const char *key, const char *value);
esp_err_t get_string(const char *key, char *value, size_t max_len);

#endif // CONFIG_H