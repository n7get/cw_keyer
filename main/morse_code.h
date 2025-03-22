#ifndef MORSE_CODE_H
#define MORSE_CODE_H

void morse_code_init(int pin); // Initialize the Morse code system with a GPIO pin
void send_morse_code(const char *message, int wpm); // Send a message and WPM to the Morse code task

#endif // MORSE_CODE_H
