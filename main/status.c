#include "esp_http_server.h"
#include "esp_log.h"
#include "http.h"
#include "message.h"
#include <stdio.h>
#include <string.h>

static const char *TAG = "API";
extern bool busy;

static esp_err_t status_handler(httpd_req_t *req) {
    char response[256];
    snprintf(response, sizeof(response),
             "{\"status\": \"ok\", \"busy\": %s}",
             busy ? "true" : "false");

    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, response, strlen(response));
    return ESP_OK;
}

void register_status_endpoints(void) {
    register_html_page("/api/status", HTTP_GET, status_handler);
    ESP_LOGI(TAG, "API endpoints registered");
}
