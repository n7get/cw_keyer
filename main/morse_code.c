#include <string.h>
#include <portmacro.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "morse_code.h"
#include "morse_code_characters.h"

int gpio_pin = -1;
int wpm = 5;

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
}

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

void morse_code(const char *message, int wpm)
{
    int unit = calculate_dit_duration(wpm); // duration of one DIT in milliseconds

    for (int i = 0; i < strlen(message); i++)
    {
        char c = message[i];
        if (c == ' ')
        {
            space(WORD_SPACE * unit); // Space between words
        }
        else
        {
            int *morse = char_to_morse(c);

            for (int j = 0; morse[j] != END; j++)
            {
                blink_led(morse[j] * unit);

                if (morse[j + 1] != END) // If not the last element
                {
                    space(SPACE * unit); // Space between DITs and DAHs
                }
            }

            if (message[i + 1] != '\0' && message[i + 1] != ' ')
            {
                space(LETTER_SPACE * unit); // Space between letters
            }
        }
    }
}