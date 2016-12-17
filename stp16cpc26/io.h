#pragma once

#include <avr/io.h>

#define STP16_CLK_PORT  PORTD
#define STP16_CLK_MASK  1 << 2
#define STP16_SDI_PORT  PORTD
#define STP16_SDI_MASK  1 << 3
#define STP16_LE_PORT   PORTD
#define STP16_LE_MASK   1 << 4
#define STP16_OE_PORT   PORTD
#define STP16_OE_MASK   1 << 5
#define STP16_OUTPUT_MASK STP16_CLK_MASK | STP16_SDI_MASK | STP16_LE_MASK | STP16_OE_MASK