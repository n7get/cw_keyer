#include "http.h"
#include "esp_http_server.h"
#include "esp_log.h"
#include <stdio.h>
#include <string.h>
#include <esp_vfs.h>
#include "esp_littlefs.h"

static const char *TAG = "HTTP";
static httpd_handle_t server = NULL;

void register_html_page(const char *uri, httpd_method_t method, esp_err_t handler(httpd_req_t *)) {
    if (server == NULL) {
        ESP_LOGE(TAG, "Web server is not running. Cannot register URI.");
        return;
    }

    httpd_uri_t page_uri = {
        .uri = uri,
        .method = method,
        .handler = handler};

    esp_err_t err = httpd_register_uri_handler(server, &page_uri);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "Registered URI: %s", uri);
    } else {
        ESP_LOGE(TAG, "Failed to register URI: %s", uri);
    }
}

static esp_err_t redirect_handler(httpd_req_t *req) {
    ESP_LOGI(TAG, "Received redirect request: %s", req->uri);
    httpd_resp_set_status(req, "307 Temporary Redirect");
    httpd_resp_set_hdr(req, "Location", "/index.html");
    httpd_resp_send(req, NULL, 0);
    return ESP_OK;
}

static esp_err_t file_handler(httpd_req_t *req) {
    ESP_LOGI(TAG, "Received file request: %s", req->uri);

    char filepath[ESP_VFS_PATH_MAX + 128] = HTML_MOUNT_POINT;
    strncat(filepath, req->uri, sizeof(filepath) - strlen(filepath) - 1);
    ESP_LOGI(TAG, "File path: %s", filepath);

    FILE *file = fopen(filepath, "r");
    if (!file) {
        ESP_LOGE(TAG, "Failed to open file: %s", filepath);
        httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "File not found");
        return ESP_FAIL;
    }

    char buffer[1024];
    size_t read_bytes;
    while ((read_bytes = fread(buffer, 1, sizeof(buffer), file)) > 0) {
        if (httpd_resp_send_chunk(req, buffer, read_bytes) != ESP_OK) {
            fclose(file);
            ESP_LOGE(TAG, "File sending failed");
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to send file");
            return ESP_FAIL;
        }
    }

    fclose(file);
    httpd_resp_send_chunk(req, NULL, 0); // End response
    return ESP_OK;
}

bool start_webserver(void) {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.max_uri_handlers = 10;

    if (httpd_start(&server, &config) == ESP_OK) {
        ESP_LOGI(TAG, "Web server started");

        register_html_page("/", HTTP_GET, redirect_handler);

        ESP_LOGI(TAG, "Reading HTML contents...");

        DIR *dir = opendir(HTML_MOUNT_POINT);
        if (!dir) {
            ESP_LOGE(TAG, "Failed to open HTML directory");
            return false;
        }

        struct dirent *entry;
        while ((entry = readdir(dir)) != NULL) {
            char filepath[ESP_VFS_PATH_MAX + 128] = "/";
            strncat(filepath, entry->d_name, sizeof(filepath) - strlen(filepath) - 1);

            register_html_page(filepath, HTTP_GET, file_handler);
        }

        closedir(dir);
        ESP_LOGI(TAG, "Finished listing HTML contents");

        return true;
    } else {
        ESP_LOGE(TAG, "Failed to start web server");
        return false;
    }
}

void stop_webserver(void) {
    if (server != NULL) {
        httpd_stop(server);
        server = NULL;
        ESP_LOGI(TAG, "Web server stopped");
    }
}
