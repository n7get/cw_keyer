#include <stdio.h>

// Helper function to convert BCD to unsigned integer
uint32_t bcd_to_uint32(const uint8_t *bcd, size_t length) {
    uint32_t result = 0;
    for (size_t i = 0; i < length; i++) {
        result = result * 100 + ((bcd[i] >> 4) * 10) + (bcd[i] & 0x0F);
    }
    return result;
}

// Helper function to convert an unsigned integer to BCD
void uint32_to_bcd(uint32_t value, uint8_t *bcd, size_t length) {
    for (int i = length - 1; i >= 0; i--) {
        bcd[i] = (value % 10) | ((value / 10 % 10) << 4);
        value /= 100;
    }
}