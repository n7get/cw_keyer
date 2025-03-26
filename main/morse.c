#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_http_server.h"
#include "http.h"
#include "message.h"
#include "morse.h"
#include "morse_code_characters.h"
#include "settings.h"

typedef struct {
    char message[MESSAGE_MAX_SIZE];
} morse_task_t;

static int gpio_pin = -1;
static QueueHandle_t morse_queue = NULL;
static TaskHandle_t morse_task_handle = NULL;

bool busy = false;

void blink_led(int duration)
{
    gpio_set_level(gpio_pin, 1);
    vTaskDelay(duration / portTICK_PERIOD_MS);
    gpio_set_level(gpio_pin, 0);
}

void space(int duration)
{
    vTaskDelay(duration / portTICK_PERIOD_MS);
}

void morse_code_task(void *arg)
{
    morse_task_t task_data;

    while (1) {
        ESP_LOGI("MORSE_TASK", "Waiting for message...");
        if (xQueueReceive(morse_queue, &task_data, portMAX_DELAY)) {
            busy = true;

            ESP_LOGI("MORSE_TASK", "Processing message: %s", task_data.message);

            int unit = calculate_dit_duration(wpm); // duration of one DIT in milliseconds

            for (int i = 0; i < strlen(task_data.message); i++) {
                char c = task_data.message[i];
                if (c == ' ') {
                    space(WORD_SPACE * unit); // Space between words
                } else {
                    int *morse = char_to_morse(c);

                    for (int j = 0; morse[j] != END; j++) {
                        blink_led(morse[j] * unit);

                        if (morse[j + 1] != END) {                        // If not the last element
                            space(SPACE * unit); // Space between DITs and DAHs
                        }
                    }

                    if (task_data.message[i + 1] != '\0' && task_data.message[i + 1] != ' ') {
                        space(LETTER_SPACE * unit); // Space between letters
                    }
                }
            }

            busy = false;
        }
    }
}

void morse_code_init(int pin)
{
    gpio_pin = pin;

    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = (1ULL << gpio_pin);
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;

    gpio_config(&io_conf);

    gpio_reset_pin(gpio_pin);
    gpio_set_direction(gpio_pin, GPIO_MODE_OUTPUT);
    gpio_set_level(gpio_pin, 0);

    morse_queue = xQueueCreate(10, sizeof(morse_task_t));
    if (morse_queue == NULL) {
        ESP_LOGE("MORSE_INIT", "Failed to create queue");
        return;
    }

    if (xTaskCreate(morse_code_task, "morse_code_task", 4096, NULL, 5, &morse_task_handle) != pdPASS) {
        ESP_LOGE("MORSE_INIT", "Failed to create task");
        return;
    }

    ESP_LOGI("MORSE_INIT", "Morse code initialized");
}

void send_morse_code(void) {
    if (morse_queue == NULL) {
        ESP_LOGE("SEND_MORSE", "Queue not initialized");
        return;
    }

    morse_task_t task_data;

    // Retrieve the current message from NVS
    esp_err_t err = get_message(task_data.message, sizeof(task_data.message));
    if (err != ESP_OK) {
        ESP_LOGE("SEND_MORSE", "Failed to get message: %s", esp_err_to_name(err));
        return;
    }

    ESP_LOGI("SEND_MORSE", "Sending message: %s", task_data.message);

    if (xQueueSend(morse_queue, &task_data, portMAX_DELAY) != pdPASS) {
        ESP_LOGE("SEND_MORSE", "Failed to send message to queue");
    } else {
        ESP_LOGI("SEND_MORSE", "Message sent to queue");
    }
}

esp_err_t morse_handler(httpd_req_t *req) {
    ESP_LOGI("MORSE_CODE", "Handling /api/morse request...");
    send_morse_code();

    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, "{\"result\": \"Morse code sent\"}", HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

void register_morse_endpoints(void) {
    register_html_page("/api/morse", HTTP_POST, morse_handler);
    ESP_LOGI("MORSE_CODE", "Morse code API endpoints registered");
}
