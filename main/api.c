#include <stdio.h>
#include <string.h>
#include "esp_http_server.h"
#include "esp_log.h"
#include "morse_code.h"
#include "http.h"

static const char *TAG = "API";
static char* message = NULL;
static int wpm = 20;

extern bool busy; // Flag to indicate if the system is busy

static esp_err_t status_handler(httpd_req_t *req) {
    char response[256];

    // Construct the JSON response with message, wpm, and busy
    snprintf(response, sizeof(response),
             "{\"status\": \"ok\", \"message\": \"%s\", \"wpm\": %d, \"busy\": %s}",
             message ? message : "",
             wpm,
             busy ? "true" : "false");

    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, response, strlen(response));
    return ESP_OK;
}

static esp_err_t morse_handler(httpd_req_t *req) {
    char query[128];
    char message_param[64];
    char wpm_param[8];

    // Get the query string from the request
    if (httpd_req_get_url_query_str(req, query, sizeof(query)) == ESP_OK) {
        ESP_LOGI(TAG, "Query string: %s", query);

        // Extract the "message" parameter
        if (httpd_query_key_value(query, "message", message_param, sizeof(message_param)) == ESP_OK) {
            ESP_LOGI(TAG, "Message: %s", message_param);
            message = strdup(message_param); // Save the message
        } else {
            ESP_LOGE(TAG, "Message parameter not found");
            const char *response = "{\"error\": \"Missing 'message' parameter\"}";
            httpd_resp_set_type(req, "application/json");
            httpd_resp_send(req, response, strlen(response));
            return ESP_FAIL;
        }

        // Extract the "wpm" parameter
        if (httpd_query_key_value(query, "wpm", wpm_param, sizeof(wpm_param)) == ESP_OK) {
            ESP_LOGI(TAG, "WPM: %s", wpm_param);
            wpm = atoi(wpm_param); // Convert WPM to integer
        } else {
            ESP_LOGE(TAG, "WPM parameter not found");
            const char *response = "{\"error\": \"Missing 'wpm' parameter\"}";
            httpd_resp_set_type(req, "application/json");
            httpd_resp_send(req, response, strlen(response));
            return ESP_FAIL;
        }
    } else {
        ESP_LOGE(TAG, "Failed to get query string");
        const char *response = "{\"error\": \"Invalid query string\"}";
        httpd_resp_set_type(req, "application/json");
        httpd_resp_send(req, response, strlen(response));
        return ESP_FAIL;
    }

    // Trigger Morse code
    ESP_LOGI(TAG, "Sending Morse Code: message=%s, wpm=%d", message, wpm);
    send_morse_code(message, wpm);

    // Send success response
    const char *response = "{\"result\": \"Morse code sent\"}";
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, response, strlen(response));

    return ESP_OK;
}

// Register API endpoints
void api_register_endpoints(void) {
    register_html_page("/status", HTTP_GET, status_handler);
    
    register_html_page("/morse", HTTP_POST, morse_handler);

    ESP_LOGI(TAG, "API endpoints registered");
}
