#include "spi_double.h"
#include "../../io.h"

#include <string.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#ifdef SPI_DOUBLE

spi_state_t spi_state_0 = { .spdr = &SPDR0 };
spi_state_t spi_state_1 = { .spdr = &SPDR1 };

void spi_init_0(uint8_t mode, uint8_t divider)
{
    spi_state_0.mode = mode;

    if (mode == SPI_MASTER)
    {
        DDR_MOSI_0 |= MASK_MOSI_0;
        DDR_SCK_0 |= MASK_SCK_0;
        DDR_SS_0 |= MASK_SS_0;
        SPCR0 |= 1 << MSTR;

        SPCR0 = 1 << SPIE | 1 << SPE | 1 << MSTR | (divider & 0x03);
    } 
    else if (mode == SPI_SLAVE)
    {
        DDR_MISO_0 |= MASK_MISO_0;
    }

    if (divider & 0x04)
        SPSR0 |= 1 << SPI2X;
}

void spi_init_1(uint8_t mode, uint8_t divider)
{
    spi_state_1.mode = mode;

    if (mode == SPI_MASTER)
    {
        DDR_MOSI_1 |= MASK_MOSI_1;
        DDR_SCK_1 |= MASK_SCK_1;
        DDR_SS_1 |= MASK_SS_1;

        SPCR1 = 1 << SPIE1 | 1 << SPE1 | 1 << MSTR1 | (divider & 0x03);
    }
    else if (mode == SPI_SLAVE)
    {
        DDR_MISO_1 |= MASK_MISO_1;
    }

    if (divider & 0x04)
        SPSR1 |= 1 << SPI2X;
}

uint8_t spi_completed_0(void) 
{
    return !spi_state_0.running;    
}

uint8_t spi_completed_1(void)
{
    return !spi_state_1.running;
}

int8_t spi_tranceive_0(uint8_t* const buffer, uint8_t len)
{
    spi_state_0.transmitting = 1;
    spi_state_0.receiving = 1;

    return _spi_tranceive(&spi_state_0, buffer, len);
}

int8_t spi_receive_0(uint8_t* const buffer, uint8_t len)
{
    spi_state_0.transmitting = 0;
    spi_state_0.receiving = 1;

    return _spi_tranceive(&spi_state_0, buffer, len);
}

int8_t spi_transmit_0(uint8_t* const buffer, uint8_t len)
{
    spi_state_0.transmitting = 1;
    spi_state_0.receiving = 0;

    return _spi_tranceive(&spi_state_0, buffer, len);
}

int8_t spi_tranceive_1(uint8_t* const buffer, uint8_t len)
{
    spi_state_1.transmitting = 1;
    spi_state_1.receiving = 1;

    return _spi_tranceive(&spi_state_1, buffer, len);
}

int8_t spi_receive_1(uint8_t* const buffer, uint8_t len)
{
    spi_state_1.transmitting = 0;
    spi_state_1.receiving = 1;

    return _spi_tranceive(&spi_state_1, buffer, len);
}

int8_t spi_transmit_1(uint8_t* const buffer, uint8_t len)
{
    spi_state_1.transmitting = 1;
    spi_state_1.receiving = 0;

    return _spi_tranceive(&spi_state_1, buffer, len);
}

ISR(SPI0_STC_vect)
{
    _spi_isr(&spi_state_0);
}

ISR(SPI1_STC_vect)
{
    _spi_isr(&spi_state_1);
}

#endif
