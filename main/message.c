#include "message.h"
#include "cJSON.h"
#include "config.h"
#include "esp_err.h"
#include "esp_http_server.h"
#include "esp_log.h"
#include "http.h"
#include "message.h"
#include "nvs.h"
#include "nvs_flash.h"
#include <stdio.h>
#include <string.h>

static const char *TAG = "MESSAGE";
#define MESSAGE_KEY_PREFIX "message_"

esp_err_t set_message(int index, const char *message) {
    if (index < 0 || index >= MAX_MESSAGES) {
        ESP_LOGE(TAG, "Invalid message index: %d", index);
        return ESP_ERR_INVALID_ARG;
    }

    char key[16];
    snprintf(key, sizeof(key), "%s%d", MESSAGE_KEY_PREFIX, index);

    esp_err_t err = set_string(key, message);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "Message %d saved to NVS: %s", index, message);
    } else {
        ESP_LOGE(TAG, "Failed to save message %d to NVS: %s", index, esp_err_to_name(err));
    }
    return err;
}

static esp_err_t get_message(int index, char *message, size_t size) {
    if (index < 0 || index >= MAX_MESSAGES) {
        ESP_LOGE(TAG, "Invalid message index: %d", index);
        return ESP_ERR_INVALID_ARG;
    }

    char key[16];
    snprintf(key, sizeof(key), "%s%d", MESSAGE_KEY_PREFIX, index);

    esp_err_t err = get_string(key, message, size);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "Message %d loaded from NVS: %s", index, message);
    } else if (err == ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGI(TAG, "No message %d found in NVS, initializing to empty", index);
        message[0] = '\0';
    } else {
        ESP_LOGE(TAG, "Failed to load message %d from NVS: %s", index, esp_err_to_name(err));
    }
    return err;
}

esp_err_t get_current_message(char *message, size_t size) {
    uint8_t current_message = 0;

    char key[16];
    snprintf(key, sizeof(key), "%s%d", MESSAGE_KEY_PREFIX, current_message);

    esp_err_t err = get_string(key, message, size);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get current message: %s", esp_err_to_name(err));
        return err;
    }
    return current_message;
}

esp_err_t get_message_handler(httpd_req_t *req) {
    cJSON *response_json = cJSON_CreateArray();
    if (!response_json) {
        ESP_LOGE(TAG, "Failed to create JSON array");
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to create JSON response");
        return ESP_FAIL;
    }

    for (int i = 0; i < MAX_MESSAGES; i++) {
        char message[MESSAGE_MAX_SIZE] = "";
        get_message(i, message, sizeof(message));
        cJSON_AddItemToArray(response_json, cJSON_CreateString(message));
    }

    const char *response = cJSON_PrintUnformatted(response_json);
    if (!response) {
        ESP_LOGE(TAG, "Failed to print JSON response");
        cJSON_Delete(response_json);
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to create JSON response");
        return ESP_FAIL;
    }

    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, response, strlen(response));

    cJSON_Delete(response_json);
    free((void *)response);
    return ESP_OK;
}

esp_err_t set_message_handler(httpd_req_t *req) {
    char buffer[256];
    int received = httpd_req_recv(req, buffer, sizeof(buffer) - 1);

    if (received <= 0) {
        ESP_LOGE(TAG, "Failed to receive request body");
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Failed to receive request body");
        return ESP_FAIL;
    }

    buffer[received] = '\0';
    ESP_LOGI(TAG, "Received body: %s", buffer);

    cJSON *json = cJSON_Parse(buffer);
    if (!json) {
        ESP_LOGE(TAG, "Failed to parse JSON");
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid JSON");
        return ESP_FAIL;
    }

    for (int i = 0; i < MAX_MESSAGES; i++) {
        char key[16];
        snprintf(key, sizeof(key), "message_%d", i);

        cJSON *message_json = cJSON_GetObjectItem(json, key);
        if (message_json && cJSON_IsString(message_json)) {
            set_message(i, message_json->valuestring);
        }
    }

    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, "{\"result\": \"Messages saved\"}", HTTPD_RESP_USE_STRLEN);

    cJSON_Delete(json);
    return ESP_OK;
}

void register_message_endpoints(void) {
    register_html_page("/api/message", HTTP_GET, get_message_handler);
    register_html_page("/api/message", HTTP_POST, set_message_handler);
    ESP_LOGI(TAG, "Message API endpoints registered");
}
