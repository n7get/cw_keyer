#ifndef MESSAGE_H
#define MESSAGE_H

#include <esp_err.h>
#include "esp_http_server.h"

#define MESSAGE_MAX_SIZE 64
#define MAX_MESSAGES 10

esp_err_t set_message(int index, const char *message);
esp_err_t get_current_message(char *message, size_t size);

void register_message_endpoints(void);

#endif // MESSAGE_H