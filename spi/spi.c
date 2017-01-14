#include "spi.h"
#include <avr/io.h>

#if defined(SPCR)
    #define SPI_SINGLE
    #include "single/spi_single.h"
#elif defined(SPCR0) && defined(SPCR1)
    #define SPI_DOUBLE
    #include "double/spi_double.h"
#else
    #error 'Unsupported device'
#endif

int8_t _spi_tranceive(spi_state_t* const state, uint8_t* const buffer, uint8_t len)
{
    if (state->running)
        return SPI_ALREADY_STARTED;

    state->buf = buffer;
    state->len = len;
    state->index = 0;
    state->running = 1;
    
    if (state->mode == SPI_MASTER)
        *state->spdr = buffer[0];    // start  by transmitting the first byte

    return SPI_OK;
}

void _spi_isr(spi_state_t* const state)
{
    if (!state->running)
        return;

    uint8_t c = *state->spdr;               // read the received byte

    if (++state->index >= state->len)       // completed
        state->running = 0;
    else
        *state->spdr = state->transmitting ? state->buf[state->index] : 0xFF;    // transmit next byte

    if (state->receiving)
        state->buf[state->index - 1] = c;       // store the received byte
}