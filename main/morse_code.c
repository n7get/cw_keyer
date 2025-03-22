#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "morse_code.h"
#include "morse_code_characters.h"

typedef struct {
    char message[128];
    int wpm;
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
        // Wait for a message from the queue
        if (xQueueReceive(morse_queue, &task_data, portMAX_DELAY)) {

            // Set busy to true while processing
            busy = true;
        
            ESP_LOGI("MORSE_TASK", "Processing message: %s, WPM: %d", task_data.message, task_data.wpm);

            int unit = calculate_dit_duration(task_data.wpm); // duration of one DIT in milliseconds

            for (int i = 0; i < strlen(task_data.message); i++) {
                char c = task_data.message[i];
                if (c == ' ') {
                    space(WORD_SPACE * unit); // Space between words
                } else {
                    int *morse = char_to_morse(c);

                    for (int j = 0; morse[j] != END; j++) {
                        blink_led(morse[j] * unit);

                        if (morse[j + 1] != END) { // If not the last element
                            space(SPACE * unit); // Space between DITs and DAHs
                        }
                    }

                    if (task_data.message[i + 1] != '\0' && task_data.message[i + 1] != ' ') {
                        space(LETTER_SPACE * unit); // Space between letters
                    }
                }
            }

            // Set busy to false after processing
            busy = false;
        }
    }
}

void morse_code_init(int pin)
{
    gpio_pin = pin;

    // Initialize GPIO for output
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = (1ULL << gpio_pin);
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;

    // Configure the GPIO
    gpio_config(&io_conf);

    // Set the initial state to LOW
    gpio_reset_pin(gpio_pin);
    gpio_set_direction(gpio_pin, GPIO_MODE_OUTPUT);
    gpio_set_level(gpio_pin, 0);

    // Create the queue
    morse_queue = xQueueCreate(10, sizeof(morse_task_t));
    if (morse_queue == NULL) {
        ESP_LOGE("MORSE_INIT", "Failed to create queue");
        return;
    }

    // Create the task
    if (xTaskCreate(morse_code_task, "morse_code_task", 4096, NULL, 5, &morse_task_handle) != pdPASS) {
        ESP_LOGE("MORSE_INIT", "Failed to create task");
        return;
    }           

    ESP_LOGI("MORSE_INIT", "Morse code initialized");
}

void send_morse_code(const char *message, int wpm)
{
    if (morse_queue == NULL) {
        ESP_LOGE("SEND_MORSE", "Queue not initialized");
        return;
    }

    morse_task_t task_data;

    // Copy the message and WPM into the allocated structure
    strncpy(task_data.message, message, sizeof(task_data.message) - 1);
    task_data.message[sizeof(task_data.message) - 1] = '\0';
    task_data.wpm = wpm;

    ESP_LOGI("SEND_MORSE", "Sending message: %s, WPM: %d", task_data.message, task_data.wpm);
    // Send the pointer to the queue
    if (xQueueSend(morse_queue, &task_data, portMAX_DELAY) != pdPASS) {
        ESP_LOGE("SEND_MORSE", "Failed to send message to queue");
    }
    else {
        ESP_LOGI("SEND_MORSE", "Message sent to queue");
    }
}
