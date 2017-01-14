#pragma once

#include <inttypes.h>
#include "io.h"

#define MAX7221_NOOP   0
#define MAX7221_DIGIT0 1
#define MAX7221_DIGIT1 2
#define MAX7221_DIGIT2 3
#define MAX7221_DIGIT3 4
#define MAX7221_DIGIT4 5
#define MAX7221_DIGIT5 6
#define MAX7221_DIGIT6 7
#define MAX7221_DIGIT7 8
#define MAX7221_DECODEMODE  9
#define MAX7221_INTENSITY   10
#define MAX7221_SCANLIMIT   11
#define MAX7221_SHUTDOWN    12
#define MAX7221_DISPLAYTEST 15

#ifndef MAX7221_DEVICES
    #define MAX7221_DEVICES 1
#endif

void max7221_init(void);
void max7221_write(uint8_t device, uint8_t address, uint8_t data);
void max7221_display(uint8_t device, uint8_t digit, uint8_t c);
