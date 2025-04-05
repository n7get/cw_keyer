#ifndef UART_H
#define UART_H

#include "esp_err.h"
#include <stddef.h>
#include <stdint.h>

// UART configuration
#define UART_NUM UART_NUM_1
#define UART_BAUD_RATE 4800
#define BUF_SIZE 1024

// Function prototypes
esp_err_t cat_init(void);
esp_err_t cat_send(const uint8_t *command, size_t command_size);
esp_err_t cat_recv(uint8_t *response, size_t response_size);
esp_err_t cat_recv_until(uint8_t *response, size_t response_size, char terminator);

#endif // UART_H
