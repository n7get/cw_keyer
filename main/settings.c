#include "cJSON.h"
#include "config.h"
#include "esp_log.h"
#include "http.h"
#include "nvs_flash.h"
#include "status.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *TAG = "SETTINGS";

int wpm = 20;
char ap_ssid[32] = "cw_keyer";
char ap_password[64] = "";
char sta_ssid[32] = "";
char sta_password[64] = "";

#ifdef CONFIG_RADIO_FT857D
#define DEFAULT_BAUD_RATE 4800
#endif
#ifndef DEFAULT_BAUD_RATE
#define DEFAULT_BAUD_RATE 38400
#endif
int baud_rate = DEFAULT_BAUD_RATE;

void load_settings(void) {
    ESP_ERROR_CHECK(nvs_flash_init());

    uint8_t u8_v;
    if (get_u8("wpm", &u8_v) == ESP_OK) {
        ESP_LOGI(TAG, "Loaded WPM from NVS: %d", u8_v);
        wpm = (int)u8_v;
    } else {
        ESP_LOGW(TAG, "Failed to load WPM from NVS, using default: %d", wpm);
        set_u8("wpm", (uint8_t)wpm);
        ESP_LOGI(TAG, "Default WPM saved to NVS: %d", wpm);
    }

    if (get_string("ap_ssid", ap_ssid, sizeof(ap_ssid)) == ESP_OK) {
        ESP_LOGI(TAG, "Loaded AP SSID from NVS: %s", ap_ssid);
    } else {
        ESP_LOGW(TAG, "Failed to load AP SSID from NVS, using default: %s", ap_ssid);
        set_string("ap_ssid", ap_ssid);
        ESP_LOGI(TAG, "Default AP SSID saved to NVS: %s", ap_ssid);
    }

    if (get_string("ap_password", ap_password, sizeof(ap_password)) == ESP_OK) {
        ESP_LOGI(TAG, "Loaded AP Password from NVS: %s", ap_password);
    } else {
        ESP_LOGW(TAG, "Failed to load AP Password from NVS, using default: %s", ap_password);
        set_string("ap_password", ap_password);
        ESP_LOGI(TAG, "Default AP Password saved to NVS: %s", ap_password);
    }

    if (get_string("sta_ssid", sta_ssid, sizeof(sta_ssid)) == ESP_OK) {
        ESP_LOGI(TAG, "Loaded STA SSID from NVS: %s", sta_ssid);
    } else {
        ESP_LOGW(TAG, "Failed to load STA SSID from NVS, using default: %s", sta_ssid);
        set_string("sta_ssid", sta_ssid);
        ESP_LOGI(TAG, "Default STA SSID saved to NVS: %s", sta_ssid);
    }

    if (get_string("sta_password", sta_password, sizeof(sta_password)) == ESP_OK) {
        ESP_LOGI(TAG, "Loaded STA Password from NVS: %s", sta_password);
    } else {
        ESP_LOGW(TAG, "Failed to load STA Password from NVS, using default: %s", sta_password);
        set_string("sta_password", sta_password);
        ESP_LOGI(TAG, "Default STA Password saved to NVS: %s", sta_password);
    }

    uint32_t u32_v;
    if (get_u32("baud_rate", &u32_v) == ESP_OK) {
        ESP_LOGI(TAG, "Loaded baud rate from NVS: %ld", u32_v);
        baud_rate = (int)u32_v;
    } else {
        ESP_LOGW(TAG, "Failed to load baud rate from NVS, using default: %d", baud_rate);
        set_u32("baud_rate", (uint32_t)baud_rate);
    }
}

static esp_err_t set_settings_handler(httpd_req_t *req) {
    char content[256];
    int content_len = httpd_req_recv(req, content, sizeof(content) - 1);

    if (content_len <= 0) {
        ESP_LOGE(TAG, "Failed to receive request body");
        const char *response = "{\"error\": \"Invalid request body\"}";
        httpd_resp_set_type(req, "application/json");
        httpd_resp_send(req, response, strlen(response));
        return ESP_FAIL;
    }

    content[content_len] = '\0';
    ESP_LOGI(TAG, "Received settings: %s", content);

    cJSON *json = cJSON_Parse(content);
    if (!json) {
        ESP_LOGE(TAG, "Failed to parse JSON");
        const char *response = "{\"error\": \"Invalid JSON format\"}";
        httpd_resp_set_type(req, "application/json");
        httpd_resp_send(req, response, strlen(response));
        return ESP_FAIL;
    }

    cJSON *wpm_json = cJSON_GetObjectItem(json, "wpm");
    if (wpm_json && cJSON_IsNumber(wpm_json)) {
        int new_wpm = wpm_json->valueint;
        if (new_wpm < 5 || new_wpm > 50) {
            ESP_LOGE(TAG, "WPM out of range (5-50), using default: %d", 20);
            new_wpm = 20;
        }
        if (new_wpm != wpm) {
            wpm = new_wpm;
            set_u8("wpm", (uint8_t)wpm);
            ESP_LOGI(TAG, "WPM updated and saved to NVS: %d", wpm);
        }
    } else {
        ESP_LOGE(TAG, "WPM parameter missing or invalid");
    }

    cJSON *ap_ssid_json = cJSON_GetObjectItem(json, "ap_ssid");
    if (ap_ssid_json && cJSON_IsString(ap_ssid_json)) {
        if (strcmp(ap_ssid, ap_ssid_json->valuestring) != 0) {
            strncpy(ap_ssid, ap_ssid_json->valuestring, sizeof(ap_ssid) - 1);
            ap_ssid[sizeof(ap_ssid) - 1] = '\0';
            set_string("ap_ssid", ap_ssid);
            ESP_LOGI(TAG, "AP SSID updated and saved to NVS: %s", ap_ssid);
        }
    } else {
        ESP_LOGE(TAG, "AP SSID parameter missing or invalid");
    }

    cJSON *ap_password_json = cJSON_GetObjectItem(json, "ap_password");
    if (ap_password_json && cJSON_IsString(ap_password_json)) {
        if (strcmp(ap_password, ap_password_json->valuestring) != 0) {
            strncpy(ap_password, ap_password_json->valuestring, sizeof(ap_password) - 1);
            ap_password[sizeof(ap_password) - 1] = '\0';
            set_string("ap_password", ap_password);
            ESP_LOGI(TAG, "AP Password updated and saved to NVS: %s", ap_password);
        }
    } else {
        ESP_LOGE(TAG, "AP Password parameter missing or invalid");
    }

    cJSON *sta_ssid_json = cJSON_GetObjectItem(json, "sta_ssid");
    if (sta_ssid_json && cJSON_IsString(sta_ssid_json)) {
        if (strcmp(sta_ssid, sta_ssid_json->valuestring) != 0) {
            strncpy(sta_ssid, sta_ssid_json->valuestring, sizeof(sta_ssid) - 1);
            sta_ssid[sizeof(sta_ssid) - 1] = '\0';
            set_string("sta_ssid", sta_ssid);
            ESP_LOGI(TAG, "STA SSID updated and saved to NVS: %s", sta_ssid);
        }
    } else {
        ESP_LOGE(TAG, "STA SSID parameter missing or invalid");
    }

    cJSON *sta_password_json = cJSON_GetObjectItem(json, "sta_password");
    if (sta_password_json && cJSON_IsString(sta_password_json)) {
        if (strcmp(sta_password, sta_password_json->valuestring) != 0) {
            strncpy(sta_password, sta_password_json->valuestring, sizeof(sta_password) - 1);
            sta_password[sizeof(sta_password) - 1] = '\0';
            set_string("sta_password", sta_password);
            ESP_LOGI(TAG, "STA Password updated and saved to NVS: %s", sta_password);
        }
    } else {
        ESP_LOGE(TAG, "STA Password parameter missing or invalid");
    }

    cJSON *baud_rate_json = cJSON_GetObjectItem(json, "baud_rate");
    if (baud_rate_json && cJSON_IsNumber(baud_rate_json)) {
        int new_baud_rate = baud_rate_json->valueint;
        if (new_baud_rate != baud_rate) {
            baud_rate = new_baud_rate;
            set_u32("baud_rate", (uint32_t)baud_rate);
            ESP_LOGI(TAG, "Baud rate updated and saved to NVS: %d", baud_rate);
        }
    } else {
        ESP_LOGE(TAG, "Baud rate parameter missing or invalid");
    }

    cJSON_Delete(json);

    const char *response = "{\"result\": \"Settings updated successfully\"}";
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, response, strlen(response));

    return ESP_OK;
}

static esp_err_t get_settings_handler(httpd_req_t *req) {
    cJSON *json = cJSON_CreateObject();
    if (!json) {
        ESP_LOGE(TAG, "Failed to create JSON object");
        const char *response = "{\"error\": \"Failed to create JSON object\"}";
        httpd_resp_set_type(req, "application/json");
        httpd_resp_send(req, response, strlen(response));
        return ESP_FAIL;
    }

    cJSON_AddNumberToObject(json, "wpm", wpm);
    cJSON_AddStringToObject(json, "ap_ssid", ap_ssid);
    cJSON_AddStringToObject(json, "ap_password", ap_password);
    cJSON_AddStringToObject(json, "sta_ssid", sta_ssid);
    cJSON_AddStringToObject(json, "sta_password", sta_password);
    cJSON_AddNumberToObject(json, "baud_rate", baud_rate);

    const char *response = cJSON_Print(json);
    if (!response) {
        ESP_LOGE(TAG, "Failed to print JSON object");
        cJSON_Delete(json);
        const char *error_response = "{\"error\": \"Failed to generate JSON response\"}";
        httpd_resp_set_type(req, "application/json");
        httpd_resp_send(req, error_response, strlen(error_response));
        return ESP_FAIL;
    }

    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, response, strlen(response));

    cJSON_Delete(json);
    free((void *)response);

    return ESP_OK;
}

void register_settings_endpoints(void) {
    register_html_page("/api/settings", HTTP_GET, get_settings_handler);
    register_html_page("/api/settings", HTTP_POST, set_settings_handler);

    ESP_LOGI(TAG, "Settings endpoints registered");
}
