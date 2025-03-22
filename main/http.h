#ifndef HTTP_H
#define HTTP_H
#include "esp_err.h"
#include <esp_http_server.h>

bool start_webserver(void);
void stop_webserver(void);
void register_html_page(const char *uri, httpd_method_t method, esp_err_t handler(httpd_req_t *));

#endif // HTTP_H
