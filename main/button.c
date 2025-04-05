#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "morse.h"
#include "pins.h"
#include "tune.h"
#include <stdio.h>

#define LONG_PRESS_THRESHOLD_MS 1000 // Threshold for long press in milliseconds

static const char *TAG = "BUTTON";

static TimerHandle_t long_press_timer; // Timer to detect long press
static bool is_long_press = false;     // Flag to indicate a long press
static tune_data_t tune_data;          // Data to store frequency and mode

// ISR handler for the button press
static void IRAM_ATTR button_isr_handler(void *arg) {
    // Notify the task to handle the button press
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xTaskNotifyFromISR((TaskHandle_t)arg, 0, eNoAction, &xHigherPriorityTaskWoken);

    if (xHigherPriorityTaskWoken) {
        portYIELD_FROM_ISR();
    }
}

// Callback for the long press timer
static void long_press_timer_callback(TimerHandle_t xTimer) {
    is_long_press = true;
    ESP_LOGI(TAG, "Long press detected, starting tuning...");
    tune_start(&tune_data); // Start tuning
}

// Task to handle the button press
void button_task(void *arg) {
    while (1) {
        // Wait for notification from the ISR
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        // Check if the button is pressed
        if (gpio_get_level(BUTTON_GPIO_PIN) == 0) {
            // Start the long press timer
            xTimerStart(long_press_timer, 0);
        } else {
            // Button released
            xTimerStop(long_press_timer, 0);

            if (is_long_press) {
                // If it was a long press, stop tuning
                ESP_LOGI(TAG, "Button released after long press, stopping tuning...");
                tune_stop(&tune_data);
                is_long_press = false;
            } else {
                // If it was a short press, execute the momentary action
                ESP_LOGI(TAG, "Button pressed momentarily, executing send_morse_code...");
                send_morse_code();
            }
        }
    }
}

// Initialize the button GPIO and task
void button_init(void) {
    // Configure the GPIO pin as input with a pull-up resistor
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << BUTTON_GPIO_PIN),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_ANYEDGE // Trigger on both press and release
    };
    gpio_config(&io_conf);

    // Create a task to handle the button press
    TaskHandle_t button_task_handle;
    xTaskCreate(button_task, "button_task", 2048, NULL, 10, &button_task_handle);

    // Install the ISR handler
    gpio_install_isr_service(0);
    gpio_isr_handler_add(BUTTON_GPIO_PIN, button_isr_handler, (void *)button_task_handle);

    // Create the long press timer
    long_press_timer = xTimerCreate("long_press_timer", pdMS_TO_TICKS(LONG_PRESS_THRESHOLD_MS), pdFALSE, NULL, long_press_timer_callback);

    ESP_LOGI(TAG, "Button initialized on GPIO %d", BUTTON_GPIO_PIN);
}
