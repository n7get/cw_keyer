#include "http.h"
#include "index.h"
#include <esp_err.h>
#include <esp_http_server.h>

esp_err_t index_handler(httpd_req_t *req)
{
    const char *text = "Hello, World!"; // Sample text to send
    const char *pattern = 
        "<html>"
        "<body>"
        "<h1>Index Page</h1>"
        "<p>%s</p>"
        "</body>"
        "</html>";
    char html[256]; // Buffer to hold the formatted HTML content
    snprintf(html, sizeof(html), pattern, text); // Format the HTML content with the text

    httpd_resp_set_type(req, "text/html");         // Set response type to HTML
    httpd_resp_send(req, html, strlen(html));      // Send the HTML content
    return ESP_OK;
}


void register_index_page(void)
{
    register_html_page("/", index_handler);
}