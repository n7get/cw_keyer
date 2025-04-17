#include "cat.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "pins.h"
#include "settings.h"
#include <string.h>

#define TAG "UART"

static QueueHandle_t uart_queue;
static QueueHandle_t data_queue;
#define DATA_QUEUE_SIZE 1024
#define RESPONSE_TIMEOUT_MS 1000

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
                    for (int i = 0; i < len; i++) {
                        if (xQueueSend(data_queue, &data[i], portMAX_DELAY) != pdPASS) {
                            ESP_LOGE(TAG, "Data queue overflow");
                            break;
                        }
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
esp_err_t cat_init(void) {
    // Create the data queue
    data_queue = xQueueCreate(DATA_QUEUE_SIZE, sizeof(uint8_t));
    if (data_queue == NULL) {
        ESP_LOGE(TAG, "Failed to create data queue");
        return ESP_FAIL;
    }

    uart_config_t uart_config = {
        .baud_rate = baud_rate,
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
esp_err_t cat_send(const uint8_t *command, size_t command_size) {
    int bytes_written = uart_write_bytes(UART_NUM, (const char *)command, command_size);
    if (bytes_written < 0) {
        ESP_LOGE(TAG, "Failed to write CAT command");
        return ESP_FAIL;
    }
    if (bytes_written != command_size) {
        ESP_LOGE(TAG, "Number of bytes written does not match command size: expected %d, got %d", command_size, bytes_written);
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "CAT command sent");
    return ESP_OK;
}

// Read a CAT response
esp_err_t cat_recv(uint8_t *response, size_t response_size) {
    for (size_t i = 0; i < response_size; i++) {
        if (xQueueReceive(data_queue, &response[i], pdMS_TO_TICKS(RESPONSE_TIMEOUT_MS)) != pdPASS) {
            ESP_LOGE(TAG, "Timeout while reading response");
            return ESP_FAIL;
        }
    }
    return ESP_OK;
}

// Read response until a terminator is found
esp_err_t cat_recv_until(uint8_t *response, size_t response_size, char terminator) {
    size_t i = 0;
    while (i < response_size) {
        if (xQueueReceive(data_queue, &response[i], pdMS_TO_TICKS(RESPONSE_TIMEOUT_MS)) != pdPASS) {
            ESP_LOGE(TAG, "Timeout while reading response");
            return ESP_FAIL;
        }
        if (response[i] == terminator) {
            if (i > response_size) {
                ESP_LOGE(TAG, "Response size exceeds buffer size");
                return ESP_FAIL;
            }

            response[i + 1] = '\0';
            return ESP_OK;
        }
        i++;
    }
    ESP_LOGE(TAG, "Terminator not found in response");
    return ESP_FAIL;
}
