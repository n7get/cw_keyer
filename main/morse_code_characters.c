#include "morse_code_characters.h"
#include <stdio.h>
#include <string.h>

#define MAX_MORSE_LENGTH 10

typedef struct {
    char character;
    int morse[MAX_MORSE_LENGTH];
} MorseCode;

static const MorseCode morse_table[] = {
    {'A', {DIT, DAH, END}},
    {'B', {DAH, DIT, DIT, DIT, END}},
    {'C', {DAH, DIT, DAH, DIT, END}},
    {'D', {DAH, DIT, DIT, END}},
    {'E', {DIT, END}},
    {'F', {DIT, DIT, DAH, DIT, END}},
    {'G', {DAH, DAH, DIT, END}},
    {'H', {DIT, DIT, DIT, DIT, END}},
    {'I', {DIT, DIT, END}},
    {'J', {DIT, DAH, DAH, DAH, END}},
    {'K', {DAH, DIT, DAH, END}},
    {'L', {DIT, DAH, DIT, DIT, END}},
    {'M', {DAH, DAH, END}},
    {'N', {DAH, DIT, END}},
    {'O', {DAH, DAH, DAH, END}},
    {'P', {DIT, DAH, DAH, DIT, END}},
    {'Q', {DAH, DAH, DIT, DAH, END}},
    {'R', {DIT, DAH, DIT, END}},
    {'S', {DIT, DIT, DIT, END}},
    {'T', {DAH, END}},
    {'U', {DIT, DIT, DAH, END}},
    {'V', {DIT, DIT, DIT, DAH, END}},
    {'W', {DIT, DAH, DAH, END}},
    {'X', {DAH, DIT, DIT, DAH, END}},
    {'Y', {DAH, DIT, DAH, DAH, END}},
    {'Z', {DAH, DAH, DIT, DIT, END}},
    {'1', {DIT, DAH, DAH, DAH, DAH, END}},
    {'2', {DIT, DIT, DAH, DAH, DAH, END}},
    {'3', {DIT, DIT, DIT, DAH, DAH, END}},
    {'4', {DIT, DIT, DIT, DIT, DAH, END}},
    {'5', {DIT, DIT, DIT, DIT, DIT, END}},
    {'6', {DAH, DIT, DIT, DIT, DIT, END}},
    {'7', {DAH, DAH, DIT, DIT, DIT, END}},
    {'8', {DAH, DAH, DAH, DIT, DIT, END}},
    {'9', {DAH, DAH, DAH, DAH, DIT, END}},
    {'0', {DAH, DAH, DAH, DAH, DAH, END}},
    {'.', {DIT, DAH, DIT, DAH, DIT, DAH, END}},
    {',', {DAH, DAH, DIT, DIT, DAH, DAH, END}},
    {'?', {DIT, DIT, DAH, DAH, DIT, DIT, END}},
    {'\'', {DIT, DAH, DAH, DAH, DAH, DIT, END}},
    {'!', {DAH, DIT, DAH, DIT, DAH, DAH, END}},
    {'/', {DAH, DIT, DIT, DAH, DIT, END}},
    {'(', {DAH, DIT, DAH, DAH, DIT, END}},
    {')', {DAH, DIT, DAH, DAH, DIT, DAH, END}},
    {'&', {DIT, DAH, DIT, DIT, DIT, END}},
    {':', {DAH, DAH, DAH, DIT, DIT, DIT, END}},
    {';', {DAH, DIT, DAH, DIT, DAH, DIT, END}},
    {'=', {DAH, DIT, DIT, DIT, DAH, END}},
    {'+', {DIT, DAH, DIT, DAH, DIT, END}},
    {'-', {DAH, DIT, DIT, DIT, DIT, DAH, END}},
    {'_', {DIT, DIT, DAH, DAH, DIT, DAH, END}},
    {'"', {DIT, DAH, DIT, DIT, DAH, DIT, END}},
    {'$', {DIT, DIT, DIT, DAH, DIT, DIT, DAH, END}},
    {'@', {DIT, DAH, DAH, DIT, DAH, DIT, END}}};

/**
 * Calculate the duration of a DIT in milliseconds based on the given speed in WPM.
 * @param wpm Speed in words per minute.
 * @return Duration of a DIT in milliseconds.
 */
int calculate_dit_duration(int wpm) {
    // The standard word "PARIS" is used to calculate WPM, which has 50 units (DITs).
    // 1200 ms is the time for one WPM.
    return 1200 / wpm;
}

int *char_to_morse(char c) {
    static int morse_code[MAX_MORSE_LENGTH];
    memset(morse_code, 0, sizeof(morse_code));

    for (int i = 0; i < sizeof(morse_table) / sizeof(MorseCode); i++) {
        if (morse_table[i].character == c || morse_table[i].character == c - 32) {
            memcpy(morse_code, morse_table[i].morse, sizeof(morse_table[i].morse));
            break;
        }
    }

    return morse_code;
}
