#pragma once

#include <avr/io.h>

#define MAX7221_DATA_PORT   PORTD
#define MAX7221_DATA_MASK   1 << 2
#define MAX7221_CS_PORT     PORTD
#define MAX7221_CS_MASK     1 << 3
#define MAX7221_SCK_PORT    PORTD
#define MAX7221_SCK_MASK    1 << 4