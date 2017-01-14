#pragma once

#include "../spi.h"

#ifdef SPI_SINGLE

extern spi_state_t spi_state;

void spi_init(uint8_t mode, uint8_t divider);

uint8_t spi_completed(void);
int8_t spi_tranceive(uint8_t* const buffer, uint8_t len);
int8_t spi_receive(uint8_t* const buffer, uint8_t len);
int8_t spi_transmit(uint8_t* const buffer, uint8_t len);

#endif