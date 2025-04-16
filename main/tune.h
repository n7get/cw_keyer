#ifndef TUNE_H
#define TUNE_H

#include <stdint.h>

typedef struct {
    uint32_t frequency; // Frequency in Hz
    uint8_t mode;       // Mode
    uint8_t power;      // Power level
} tune_data_t;

void tune_start(tune_data_t *data);
void tune_stop(tune_data_t *data);

#endif // TUNE_H
