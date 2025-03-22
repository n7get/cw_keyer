#include <stdio.h>
#include <string.h>
#include "http.h"
#include "esp_http_server.h"
#include "esp_log.h"

static const char *TAG = "HTTP";
static httpd_handle_t server = NULL;

// Generic HTTP GET handler for serving HTML content
void register_html_page(const char *uri, httpd_method_t method, esp_err_t handler(httpd_req_t *))
{
    if (server == NULL)
    {
        ESP_LOGE(TAG, "Web server is not running. Cannot register URI.");
        return;
    }

    httpd_uri_t page_uri = {
        .uri = uri,
        .method = method,
        .handler = handler
    };

    esp_err_t err = httpd_register_uri_handler(server, &page_uri);
    if (err == ESP_OK)
    {
        ESP_LOGI(TAG, "Registered URI: %s", uri);
    }
    else
    {
        ESP_LOGE(TAG, "Failed to register URI: %s", uri);
    }
}

// Start the web server and return the server handle
bool start_webserver(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    if (httpd_start(&server, &config) == ESP_OK)
    {
        ESP_LOGI(TAG, "Web server started");
        return true;
    }
    else
    {
        ESP_LOGE(TAG, "Failed to start web server");
        return false;
    }
}

// Stop the web server
void stop_webserver(void)
{
    if (server != NULL)
    {
        httpd_stop(server);
        server = NULL;
        ESP_LOGI(TAG, "Web server stopped");
    }
}
