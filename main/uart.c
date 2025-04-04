#include "uart.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "pins.h"
#include <string.h>

#define TAG "UART"

static QueueHandle_t uart_queue;
char* response_buffer = NULL;
int response_buffer_index = 0;
#define RESPONSE_BUF_SIZE 1024

// UART interrupt handler task
static void uart_event_task(void *pvParameters) {
    uart_event_t event;
    uint8_t *data = malloc(BUF_SIZE);
    if (data == NULL) {
        ESP_LOGE(TAG, "Failed to allocate memory for data buffer");
        vTaskDelete(NULL);
        return;
    }

    while (1) {
        // Wait for UART events
        if (xQueueReceive(uart_queue, (void *)&event, portMAX_DELAY)) {
            switch (event.type) {
                case UART_DATA:
                    ESP_LOGI(TAG, "Data received: %d bytes", event.size);
                    if (event.size > BUF_SIZE) {
                        ESP_LOGE(TAG, "Received data length exceeds buffer size");
                        break;
                    }
                    int len = uart_read_bytes(UART_NUM, data, event.size, portMAX_DELAY);
                    if (len > 0) {
                        data[len] = '\0'; // Null-terminate the received data
                        ESP_LOGI(TAG, "Received data:");
                        for (int i = 0; i < len; i++) {
                            ESP_LOGI(TAG, "Byte %d: %02X", i, data[i]);
                        }
                        if (response_buffer_index + len < RESPONSE_BUF_SIZE) {
                            memcpy(response_buffer + response_buffer_index, data, len);
                            response_buffer_index += len;
                        } else {
                            ESP_LOGE(TAG, "Response buffer overflow");
                        }
                    }
                    break;

                case UART_FIFO_OVF:
                    ESP_LOGW(TAG, "UART FIFO overflow");
                    uart_flush_input(UART_NUM);
                    xQueueReset(uart_queue);
                    break;

                case UART_BUFFER_FULL:
                    ESP_LOGW(TAG, "UART buffer full");
                    uart_flush_input(UART_NUM);
                    xQueueReset(uart_queue);
                    break;

                case UART_PARITY_ERR:
                    ESP_LOGE(TAG, "UART parity error");
                    break;

                case UART_FRAME_ERR:
                    ESP_LOGE(TAG, "UART frame error");
                    break;

                default:
                    ESP_LOGW(TAG, "Unhandled UART event type: %d", event.type);
                    break;
            }
        }
    }

    free(data);
    vTaskDelete(NULL);
}

// Initialize the UART driver with interrupt-based reading
esp_err_t uart_init(void) {
    // Allocate memory for the response buffer
    response_buffer = malloc(RESPONSE_BUF_SIZE);
    if (response_buffer == NULL) {
        ESP_LOGE(TAG, "Failed to allocate memory for response buffer");
        return ESP_FAIL;
    }

    uart_config_t uart_config = {
        .baud_rate = UART_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    };

    if (uart_param_config(UART_NUM, &uart_config) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure UART parameters");
        return ESP_FAIL;
    }

    if (uart_set_pin(UART_NUM, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set UART pins");
        return ESP_FAIL;
    }

    // Install UART driver with RX buffer and event queue
    if (uart_driver_install(UART_NUM, BUF_SIZE * 2, BUF_SIZE * 2, 20, &uart_queue, 0) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to install UART driver");
        return ESP_FAIL;
    }

    // Create a task to handle UART events
    if (xTaskCreate(uart_event_task, "uart_event_task", 2048, NULL, 12, NULL) != pdPASS) {
        ESP_LOGE(TAG, "Failed to create UART event task");
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "UART initialized with interrupt-based reading");
    return ESP_OK;
}

// Send a CAT command
esp_err_t uart_send_command(const uint8_t *command, size_t command_size) {
    response_buffer_index = 0; // Reset the response buffer index

    int bytes_written = uart_write_bytes(UART_NUM, (const char *)command, command_size);
    if (bytes_written < 0) {
        ESP_LOGE(TAG, "Failed to write CAT command");
        return ESP_FAIL;
    }
    if (bytes_written != command_size) {
        ESP_LOGE(TAG, "Number of bytes written does not match command size: expected %d, got %d", command_size, bytes_written);
        return ESP_FAIL;
    }

    // Wait for a short period to allow the command to be processed
    vTaskDelay(pdMS_TO_TICKS(100)); // 100 ms delay

    ESP_LOGI(TAG, "CAT command sent");
    return ESP_OK;
}

// Read a CAT response
esp_err_t uart_read_response(uint8_t *response, size_t response_size) {
    if (response_buffer_index < response_size) {
        ESP_LOGE(TAG, "Not enough data in response buffer: expected %d, got %d", response_size, response_buffer_index);
        return ESP_FAIL;
    }
    memcpy(response, response_buffer, response_size);
    response_buffer_index -= response_size;
    return ESP_OK;
}
