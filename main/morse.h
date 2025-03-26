#ifndef MORSE_CODE_H
#define MORSE_CODE_H

#include "esp_err.h"
#include "esp_http_server.h"

void morse_code_init(int pin);
void register_morse_endpoints(void);
void send_morse_code(void);

#endif // MORSE_CODE_H
