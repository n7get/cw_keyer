#ifndef UART_H
#define UART_H

#include "esp_err.h"
#include <stdint.h>

// UART configuration
#define UART_NUM UART_NUM_1
#define UART_BAUD_RATE 4800
#define BUF_SIZE 1024

// Function prototypes
esp_err_t uart_init(void);
esp_err_t uart_send_command(const uint8_t *command, size_t command_size);
esp_err_t uart_read_response(uint8_t *response, size_t response_size);

#endif // UART_H
