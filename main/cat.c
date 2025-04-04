#include "cat.h"
#include "esp_log.h"
#include "ft857d.h"
#include "ft991a.h"
#include <string.h>
#include "mock_radio.h"

#define TAG "CAT"

static const radio_operations_t *radio_ops = NULL;

esp_err_t init_radio(const char* radio_model) {
    if (radio_model == NULL) {
        ESP_LOGE(TAG, "Radio model cannot be NULL");
        return ESP_FAIL;
    }

    if (strcmp(radio_model, "MOCK") == 0) {
        radio_ops = &mock_radio_ops;
    } else if(strcmp(radio_model, "FT-857D") == 0) {
        radio_ops = &ft857d_ops;
    } else if (strcmp(radio_model, "FT-991A") == 0) {
        radio_ops = &ft991a_ops;
    } else {
        ESP_LOGE(TAG, "Unsupported radio model: %s", radio_model);
        return ESP_FAIL;
    }

    if (radio_ops->init_radio() != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize radio operations");
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Radio initialized with custom operations");
    return ESP_OK;
}

esp_err_t get_frequency(uint32_t *frequency) {
    if (radio_ops && radio_ops->get_frequency) {
        return radio_ops->get_frequency(frequency);
    }
    return ESP_FAIL;
}

esp_err_t set_frequency(uint32_t frequency) {
    if (radio_ops && radio_ops->set_frequency) {
        return radio_ops->set_frequency(frequency);
    }
    return ESP_FAIL;
}

esp_err_t get_mode(uint8_t *mode) {
    if (radio_ops && radio_ops->get_mode) {
        return radio_ops->get_mode(mode);
    }
    return ESP_FAIL;
}

esp_err_t set_mode(uint8_t mode) {
    if (radio_ops && radio_ops->set_mode) {
        return radio_ops->set_mode(mode);
    }
    return ESP_FAIL;
}

esp_err_t get_power(uint8_t *power) {
    if (radio_ops && radio_ops->get_power) {
        return radio_ops->get_power(power);
    }
    return ESP_FAIL;
}

esp_err_t set_power(uint8_t power) {
    if (radio_ops && radio_ops->set_power) {
        return radio_ops->set_power(power);
    }
    return ESP_FAIL;
}

esp_err_t set_ptt(bool enable) {
    if (radio_ops && radio_ops->set_ptt) {
        return radio_ops->set_ptt(enable);
    }
    return ESP_FAIL;
}

uint8_t string_to_mode(const char* mode_str) {
    if (radio_ops && radio_ops->string_to_mode) {
        return radio_ops->string_to_mode(mode_str);
    }
    return 0xFF; // Unknown mode
}

const char* mode_to_string(uint8_t mode) {
    if (radio_ops && radio_ops->mode_to_string) {
        return radio_ops->mode_to_string(mode);
    }
    return "UNKNOWN";
}
