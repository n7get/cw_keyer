#ifndef SETTINGS_H
#define SETTINGS_H

#include "esp_err.h"
#include <esp_http_server.h>

// Global variables for settings
extern int wpm;
extern char ap_ssid[32];
extern char ap_password[64];
extern char sta_ssid[32];
extern char sta_password[64];

// Function declarations
void load_settings(void);
esp_err_t settings_handler(httpd_req_t *req);
esp_err_t get_settings_handler(httpd_req_t *req);
void register_settings_endpoint(void);

#endif // SETTINGS_H
