#ifndef BCD_H
#define BCD_H

#include <stdint.h>
#include <stddef.h>

uint32_t bcd_to_uint32(const uint8_t *bcd, size_t length);
void uint32_to_bcd(uint32_t value, uint8_t *bcd, size_t length);

#endif // BCD_H