#include <stdio.h>
#include <string.h>
#include "esp_http_server.h"
#include "esp_log.h"
#include "config.h"
#include "morse_code.h"
#include "http.h"
#include <nvs.h>
#include "cJSON.h" // Include cJSON for JSON parsing

static const char *TAG = "API";

static char message[64] = ""; // Changed to a fixed-size array
extern bool busy;

static esp_err_t status_handler(httpd_req_t *req) {
    char response[256];

    // Construct the JSON response with message and busy
    snprintf(response, sizeof(response),
             "{\"status\": \"ok\", \"message\": \"%s\", \"busy\": %s}",
             message,
             busy ? "true" : "false");

    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, response, strlen(response));
    return ESP_OK;
}

static esp_err_t morse_handler(httpd_req_t *req) {
    char buffer[256];
    int received = httpd_req_recv(req, buffer, sizeof(buffer) - 1);

    if (received <= 0) {
        ESP_LOGE(TAG, "Failed to receive request body");
        const char *response = "{\"error\": \"Failed to receive request body\"}";
        httpd_resp_set_type(req, "application/json");
        httpd_resp_send(req, response, strlen(response));
        return ESP_FAIL;
    }

    buffer[received] = '\0'; // Null-terminate the received data
    ESP_LOGI(TAG, "Received body: %s", buffer);

    // Parse the JSON body
    cJSON *json = cJSON_Parse(buffer);
    if (!json) {
        ESP_LOGE(TAG, "Failed to parse JSON");
        const char *response = "{\"error\": \"Invalid JSON\"}";
        httpd_resp_set_type(req, "application/json");
        httpd_resp_send(req, response, strlen(response));
        return ESP_FAIL;
    }

    // Extract the "message" field
    cJSON *message_json = cJSON_GetObjectItem(json, "message");
    if (!cJSON_IsString(message_json)) {
        ESP_LOGE(TAG, "Missing or invalid 'message' field");
        const char *response = "{\"error\": \"Missing or invalid 'message' field\"}";
        httpd_resp_set_type(req, "application/json");
        httpd_resp_send(req, response, strlen(response));
        cJSON_Delete(json);
        return ESP_FAIL;
    }

    const char *message_param = message_json->valuestring;
    ESP_LOGI(TAG, "Message: %s", message_param);

    // Save the message to NVS if it has changed
    if (strcmp(message, message_param) != 0) {
        strncpy(message, message_param, sizeof(message) - 1); // Copy the new message
        message[sizeof(message) - 1] = '\0'; // Ensure null termination

        esp_err_t err = set_string("message", message);
        if (err == ESP_OK) {
            ESP_LOGI(TAG, "Message saved to NVS: %s", message);
        } else {
            ESP_LOGE(TAG, "Failed to save message to NVS: %s", esp_err_to_name(err));
        }
    }

    // Trigger Morse code
    ESP_LOGI(TAG, "Sending Morse Code: message=%s", message);
    send_morse_code(message);

    // Send success response
    const char *response = "{\"result\": \"Morse code sent\"}";
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, response, strlen(response));

    cJSON_Delete(json); // Free the JSON object
    return ESP_OK;
}

static esp_err_t message_handler(httpd_req_t *req) {
    char response[256];

    // Construct the JSON response with the current message
    snprintf(response, sizeof(response),
             "{\"message\": \"%s\"}",
             message);

    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, response, strlen(response));
    return ESP_OK;
}

// Initialize the API module
void api_init(void) {
    esp_err_t err = get_string("message", message, sizeof(message));
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "Loaded message from NVS: %s", message);
    } else if (err == ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGI(TAG, "No message found in NVS, initializing to empty");
        message[0] = '\0'; // Initialize to an empty string
    } else {
        ESP_LOGE(TAG, "Failed to load message from NVS: %s", esp_err_to_name(err));
    }

    register_html_page("/api/status", HTTP_GET, status_handler);
    register_html_page("/api/morse", HTTP_POST, morse_handler);
    register_html_page("/api/message", HTTP_GET, message_handler); // Register the new endpoint

    ESP_LOGI(TAG, "API endpoints registered");
}
