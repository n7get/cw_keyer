#ifndef MORSE_CODE_H
#define MORSE_CODE_H

#include "esp_err.h"
#include "stdbool.h"

void morse_code_init(void);
void register_morse_endpoints(void);
void queue_morse_code(char message[], bool enable_key);
void send_morse_code(void);

#endif // MORSE_CODE_H
