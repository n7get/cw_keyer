#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"

static const char *TAG = "KEY";

#define KEY_GPIO_PIN 2
#define LED_GPIO_PIN 3

// Initialize the GPIO pin for Morse code output
void gpio_init(int gpio_pin) {
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

    ESP_LOGI(TAG, "Key initialized on GPIO %d", gpio_pin);
}

void key_init() { gpio_init(KEY_GPIO_PIN); }

void key_down() { gpio_set_level(KEY_GPIO_PIN, 1); }
void key_up() { gpio_set_level(KEY_GPIO_PIN, 0); }

void led_init() { gpio_init(LED_GPIO_PIN); }

void led_on() { gpio_set_level(LED_GPIO_PIN, 1); }
void led_off() { gpio_set_level(LED_GPIO_PIN, 0); }
