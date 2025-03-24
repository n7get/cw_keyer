#include "nvs_flash.h"
#include "nvs.h"
#include "esp_log.h"

#define NVS_NAMESPACE "cw_keyer"

// Function to save a uint8_t configuration parameter to NVS
esp_err_t set_u8(const char *key, uint8_t value)
{
    nvs_handle_t nvs_handle;
    esp_err_t err;

    // Open NVS storage with a namespace
    err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGE("NVS", "Error opening NVS handle: %s", esp_err_to_name(err));
        return err;
    }

    // Write the value to NVS
    err = nvs_set_u8(nvs_handle, key, value);
    if (err != ESP_OK) {
        ESP_LOGE("NVS", "Error saving uint8_t value to NVS: %s", esp_err_to_name(err));
        nvs_close(nvs_handle);
        return err;
    }

    // Commit the changes
    err = nvs_commit(nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGE("NVS", "Error committing changes to NVS: %s", esp_err_to_name(err));
    }

    // Close the NVS handle
    nvs_close(nvs_handle);
    return err;
}

// Function to load a uint8_t configuration parameter from NVS
esp_err_t get_u8(const char *key, uint8_t *value)
{
    nvs_handle_t nvs_handle;
    esp_err_t err;

    // Open NVS storage with a namespace
    err = nvs_open(NVS_NAMESPACE, NVS_READONLY, &nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGE("NVS", "Error opening NVS handle: %s", esp_err_to_name(err));
        return err;
    }

    // Read the value from NVS
    err = nvs_get_u8(nvs_handle, key, value);
    if (err == ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGW("NVS", "Key '%s' not found in NVS", key);
    } else if (err != ESP_OK) {
        ESP_LOGE("NVS", "Error reading uint8_t value from NVS: %s", esp_err_to_name(err));
    }

    // Close the NVS handle
    nvs_close(nvs_handle);
    return err;
}

// Function to save a string configuration parameter to NVS
esp_err_t set_string(const char *key, const char *value)
{
    nvs_handle_t nvs_handle;
    esp_err_t err;

    // Open NVS storage with a namespace
    err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGE("NVS", "Error opening NVS handle: %s", esp_err_to_name(err));
        return err;
    }

    // Write the string value to NVS
    err = nvs_set_str(nvs_handle, key, value);
    if (err != ESP_OK) {
        ESP_LOGE("NVS", "Error saving string to NVS: %s", esp_err_to_name(err));
        nvs_close(nvs_handle);
        return err;
    }

    // Commit the changes
    err = nvs_commit(nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGE("NVS", "Error committing changes to NVS: %s", esp_err_to_name(err));
    }

    // Close the NVS handle
    nvs_close(nvs_handle);
    return err;
}

// Function to load a string configuration parameter from NVS
esp_err_t get_string(const char *key, char *value, size_t max_len)
{
    nvs_handle_t nvs_handle;
    esp_err_t err;

    // Open NVS storage with a namespace
    err = nvs_open(NVS_NAMESPACE, NVS_READONLY, &nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGE("NVS", "Error opening NVS handle: %s", esp_err_to_name(err));
        return err;
    }

    // Read the string value from NVS
    err = nvs_get_str(nvs_handle, key, value, &max_len);
    if (err == ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGW("NVS", "Key '%s' not found in NVS", key);
    } else if (err != ESP_OK) {
        ESP_LOGE("NVS", "Error reading string from NVS: %s", esp_err_to_name(err));
    }

    // Close the NVS handle
    nvs_close(nvs_handle);
    return err;
}