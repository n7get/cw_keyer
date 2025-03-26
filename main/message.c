#include "message.h"
#include "cJSON.h"
#include "config.h"
#include "esp_err.h"
#include "esp_http_server.h"
#include "esp_log.h"
#include "http.h"
#include "nvs.h"
#include "nvs_flash.h"
#include <stdio.h>
#include <string.h>

static const char *TAG = "MESSAGE";
#define MESSAGE_KEY "message"

esp_err_t set_message(const char *message) {
    char current_message[MESSAGE_MAX_SIZE] = "";
    esp_err_t err = get_message(current_message, sizeof(current_message));

    if (err == ESP_OK || err == ESP_ERR_NVS_NOT_FOUND) {
        if (strcmp(current_message, message) != 0) {
            err = set_string(MESSAGE_KEY, message);
            if (err == ESP_OK) {
                ESP_LOGI(TAG, "Message saved to NVS: %s", message);
            } else {
                ESP_LOGE(TAG, "Failed to save message to NVS: %s", esp_err_to_name(err));
            }
        } else {
            ESP_LOGI(TAG, "Message unchanged, not saving to NVS.");
            err = ESP_OK;
        }
    } else {
        ESP_LOGE(TAG, "Failed to load current message from NVS: %s", esp_err_to_name(err));
    }

    return err;
}

esp_err_t get_message(char *message, size_t size) {
    esp_err_t err = get_string(MESSAGE_KEY, message, size);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "Message loaded from NVS: %s", message);
    } else if (err == ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGI(TAG, "No message found in NVS, initializing to empty");
        message[0] = '\0';
    } else {
        ESP_LOGE(TAG, "Failed to load message from NVS: %s", esp_err_to_name(err));
    }
    return err;
}

esp_err_t get_message_handler(httpd_req_t *req) {
    char current_message[MESSAGE_MAX_SIZE] = "";
    esp_err_t err = get_message(current_message, sizeof(current_message));
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get message: %s", esp_err_to_name(err));
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to get message");
        return ESP_FAIL;
    }

    cJSON *response_json = cJSON_CreateObject();
    if (!response_json) {
        ESP_LOGE(TAG, "Failed to create JSON object");
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to create JSON response");
        return ESP_FAIL;
    }

    cJSON_AddStringToObject(response_json, "message", current_message);

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

    cJSON *message_json = cJSON_GetObjectItem(json, "message");
    if (!cJSON_IsString(message_json)) {
        ESP_LOGE(TAG, "Missing or invalid 'message' field");
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Missing or invalid 'message' field");
        cJSON_Delete(json);
        return ESP_FAIL;
    }

    const char *message_param = message_json->valuestring;
    ESP_LOGI(TAG, "Message: %s", message_param);

    esp_err_t err = set_message(message_param);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to save message: %s", esp_err_to_name(err));
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to save message");
        cJSON_Delete(json);
        return ESP_FAIL;
    }

    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, "{\"result\": \"Message saved\"}", HTTPD_RESP_USE_STRLEN);

    cJSON_Delete(json);
    return ESP_OK;
}

void register_message_endpoints(void) {
    register_html_page("/api/message", HTTP_GET, get_message_handler);
    register_html_page("/api/message", HTTP_POST, set_message_handler);
    ESP_LOGI(TAG, "Message API endpoints registered");
}
