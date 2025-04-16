#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "morse.h"
#include "pins.h"
#include "tune.h"
#include <stdio.h>

#define LONG_PRESS_THRESHOLD_MS 500 // Threshold for long press in milliseconds

static const char *TAG = "BUTTON";

static TimerHandle_t long_press_timer; // Timer to detect long press
static bool is_long_press = false;     // Flag to indicate a long press
static tune_data_t tune_data;          // Data to store frequency and mode

// Debounce timers for buttons
static TimerHandle_t debounce_timer_prev;
static TimerHandle_t debounce_timer_next;

// ISR handler for the button press
static void IRAM_ATTR select_btn_isr_handler(void *arg) {
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
void select_btn_task(void *arg) {
    while (1) {
        // Wait for notification from the ISR
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        // Check if the button is pressed
        if (gpio_get_level(SELECT_BUTTON_PIN) == 0) {
            // Start the long press timer
            xTimerStart(long_press_timer, 0);
        } else {
            // Select button released
            xTimerStop(long_press_timer, 0);

            if (is_long_press) {
                // If it was a long press, stop tuning
                ESP_LOGI(TAG, "Select button released after long press, stopping tuning...");
                tune_stop(&tune_data);
                is_long_press = false;
            } else {
                // If it was a short press, execute the momentary action
                ESP_LOGI(TAG, "Select button pressed momentarily, executing send_morse_code...");
                send_morse_code();
            }
        }
    }
}

// Debounce callback for prev_button
static void debounce_timer_prev_callback(TimerHandle_t xTimer) {
    if (gpio_get_level(PREV_BUTTON_PIN) == 0) {
        ESP_LOGI(TAG, "Debounced: Previous button pressed");
        // Add logic for handling debounced prev_button press
    }
}

// Debounce callback for next_button
static void debounce_timer_next_callback(TimerHandle_t xTimer) {
    if (gpio_get_level(NEXT_BUTTON_PIN) == 0) {
        ESP_LOGI(TAG, "Debounced: Next button pressed");
        // Add logic for handling debounced next_button press
    }
}

// ISR handler for prev_button
static void IRAM_ATTR prev_button_isr_handler(void *arg) {
    xTimerStartFromISR(debounce_timer_prev, NULL);
}

// ISR handler for next_button
static void IRAM_ATTR next_button_isr_handler(void *arg) {
    xTimerStartFromISR(debounce_timer_next, NULL);
}

// Initialize the button GPIO and task
void button_init(void) {
    // Configure the GPIO pin as input with a pull-up resistor
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << SELECT_BUTTON_PIN),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_ANYEDGE // Trigger on both press and release
    };
    gpio_config(&io_conf);

    // Create a task to handle the button press
    TaskHandle_t select_btn_task_handle;
    xTaskCreate(select_btn_task, "select_btn_task", 2048, NULL, 10, &select_btn_task_handle);

    // Install the ISR handler
    gpio_install_isr_service(0);
    gpio_isr_handler_add(SELECT_BUTTON_PIN, select_btn_isr_handler, (void *)select_btn_task_handle);
    
    // Create the long press timer
    long_press_timer = xTimerCreate("long_press_timer", pdMS_TO_TICKS(LONG_PRESS_THRESHOLD_MS), pdFALSE, NULL, long_press_timer_callback);

    // Initialize GPIO for prev_button
    gpio_config_t io_conf_prev = {
        .pin_bit_mask = (1ULL << PREV_BUTTON_PIN),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_NEGEDGE
    };
    gpio_config(&io_conf_prev);
    gpio_isr_handler_add(PREV_BUTTON_PIN, prev_button_isr_handler, NULL);

    // Initialize GPIO for next_button
    gpio_config_t io_conf_next = {
        .pin_bit_mask = (1ULL << NEXT_BUTTON_PIN),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_NEGEDGE
    };
    gpio_config(&io_conf_next);
    gpio_isr_handler_add(NEXT_BUTTON_PIN, next_button_isr_handler, NULL);

    // Create debounce timers
    debounce_timer_prev = xTimerCreate("debounce_timer_prev", pdMS_TO_TICKS(50), pdFALSE, NULL, debounce_timer_prev_callback);
    debounce_timer_next = xTimerCreate("debounce_timer_next", pdMS_TO_TICKS(50), pdFALSE, NULL, debounce_timer_next_callback);

    // ESP_LOGI(TAG, "Buttons initialized: SELECT on GPIO %d, PREV on GPIO %d, NEXT on GPIO %d", SELECT_BUTTON_PIN, PREV_BUTTON_PIN, NEXT_BUTTON_PIN);
}
