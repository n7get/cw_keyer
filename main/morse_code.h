#ifndef MORSE_CODE_H
#define MORSE_CODE_H

void morse_code_init(int pin); // Initialize the Morse code system with a GPIO pin
void send_morse_code(void);    // Send the current message stored in NVS as Morse code

#endif // MORSE_CODE_H
