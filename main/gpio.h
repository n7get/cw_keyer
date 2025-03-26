#ifndef GPIO_H
#define GPIO_H

void gpio_init(int gpio_pin);

void key_init(void);
void key_down(void);
void key_up(void);

void led_init(void);
void led_on(void);
void led_off(void);

#endif // GPIO_H