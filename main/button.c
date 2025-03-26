#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "morse.h"
#include <stdio.h>

#define BUTTON_GPIO_PIN 4
static const char *TAG = "BUTTON";

// ISR handler for the button press
static void IRAM_ATTR button_isr_handler(void *arg) {
    // Notify the task to execute send_morse_code
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xTaskNotifyFromISR((TaskHandle_t)arg, 0, eNoAction,
                       &xHigherPriorityTaskWoken);

    if (xHigherPriorityTaskWoken) {
        portYIELD_FROM_ISR();
    }
}

// Task to handle the button press
void button_task(void *arg) {
    while (1) {
        // Wait for notification from the ISR
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        ESP_LOGI(TAG, "Button pressed, executing send_morse_code...");
        send_morse_code();
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
        .intr_type = GPIO_INTR_NEGEDGE // Trigger on falling edge (grounded)
    };
    gpio_config(&io_conf);

    // Create a task to handle the button press
    TaskHandle_t button_task_handle;
    xTaskCreate(button_task, "button_task", 2048, NULL, 10, &button_task_handle);

    // Install the ISR handler
    gpio_install_isr_service(0);
    gpio_isr_handler_add(BUTTON_GPIO_PIN, button_isr_handler,
                         (void *)button_task_handle);

    ESP_LOGI(TAG, "Button initialized on GPIO %d", BUTTON_GPIO_PIN);
}
