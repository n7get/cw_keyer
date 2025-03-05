#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "morse_code_characters.h"

#define LED_GPIO_PIN 2

void blink_led(int duration)
{
    gpio_set_level(LED_GPIO_PIN, 1);
    vTaskDelay(duration / portTICK_PERIOD_MS);
    gpio_set_level(LED_GPIO_PIN, 0);
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

            if ( message[i + 1] != '\0' && message[i + 1] != ' ')
            {
                space(LETTER_SPACE * unit); // Space between letters
            }
        }
    }
}

void app_main(void)
{
    gpio_reset_pin(LED_GPIO_PIN);
    gpio_set_direction(LED_GPIO_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(LED_GPIO_PIN, 0);

    const char *message = "HELLO WORLD";
    int wpm = 5;

    morse_code(message, wpm);
}
