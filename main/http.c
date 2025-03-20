#include "http.h"
#include "esp_http_server.h"
#include "esp_log.h"

static const char *TAG = "HTTP";

// HTTP GET handler
esp_err_t hello_get_handler(httpd_req_t *req)
{
    const char *response = "Hello, World!";
    httpd_resp_send(req, response, strlen(response));
    return ESP_OK;
}

// Start the web server
void start_webserver(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    httpd_handle_t server = NULL;

    if (httpd_start(&server, &config) == ESP_OK)
    {
        httpd_uri_t hello_uri = {
            .uri = "/",
            .method = HTTP_GET,
            .handler = hello_get_handler,
            .user_ctx = NULL};
        httpd_register_uri_handler(server, &hello_uri);
        ESP_LOGI(TAG, "Web server started");
    }
    else
    {
        ESP_LOGE(TAG, "Failed to start web server");
    }
}