#pragma once

#include <inttypes.h>
#include <avr/io.h>

#if defined(SPCR)
    #define SPI_SINGLE
#elif defined(SPCR0) && defined(SPCR1)
    #define SPI_DOUBLE
#else
    #error 'Unsupported device'
#endif

#define SPI_ALREADY_STARTED 0
#define SPI_OK -1

#define SPI_MASTER 0
#define SPI_SLAVE 1

#define SPI_2X      4
#define SPI_4X      0
#define SPI_8X      5
#define SPI_16X     1
#define SPI_32X     6
#define SPI_64X     2
#define SPI_128X    3


typedef struct spi_state_t {
    volatile uint8_t* buf;
    volatile uint8_t index;
    volatile uint8_t len;
    
    volatile uint8_t mode : 1;
    volatile uint8_t transmitting : 1;
    volatile uint8_t receiving : 1;
    volatile uint8_t running : 1;

    volatile uint8_t* const spdr;
} spi_state_t;


int8_t _spi_tranceive(spi_state_t* const state, uint8_t* const buffer, uint8_t len);
void _spi_isr(spi_state_t* const state);