#include "spi_single.h"
#include "../../io.h"

#include <string.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#ifdef SPI_SINGLE

spi_state_t spi_state = { .spdr = &SPDR };

void spi_init(uint8_t mode, uint8_t divider)
{
    spi_state.mode = mode;

    if (mode == SPI_MASTER)
    {
        DDR_MOSI |= MASK_MOSI;
        DDR_SCK |= MASK_SCK;
    }
    else if (mode == SPI_SLAVE)
    {
        DDR_MISO |= MASK_MISO;
        SPCR |= 1 << SPIE | 1 << SPE | (divider & 0x03);
    }
        
    if (divider & 0x04)
        SPSR |= 1 << SPI2X;
}

uint8_t spi_completed(void)
{
    return !spi_state.running;
}

int8_t spi_tranceive(uint8_t* const buffer, uint8_t len)
{
    spi_state.transmitting = 1;
    spi_state.receiving = 1;

    return _spi_tranceive(&spi_state, buffer, len);
}

int8_t spi_receive(uint8_t* const buffer, uint8_t len)
{
    spi_state.transmitting = 0;
    spi_state.receiving = 1;

    return _spi_tranceive(&spi_state, buffer, len);
}

int8_t spi_transmit(uint8_t* const buffer, uint8_t len)
{
    spi_state.transmitting = 1;
    spi_state.receiving = 0;

    return _spi_tranceive(&spi_state, buffer, len);
}

ISR(SPI_STC_vect)
{
    _spi_isr(&spi_state);
}

#endif
