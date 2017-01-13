#pragma once
#include "../spi.h"

#ifdef SPI_DOUBLE

extern spi_state_t spi_state_0;
extern spi_state_t spi_state_1;

void spi_init_0(uint8_t mode, uint8_t divider);
void spi_init_1(uint8_t mode, uint8_t divider);

uint8_t spi_completed_0(void);
uint8_t spi_completed_1(void);

int8_t spi_tranceive_0(uint8_t* const buffer, uint8_t len);
int8_t spi_receive_0(uint8_t* const buffer, uint8_t len);
int8_t spi_transmit_0(uint8_t* const buffer, uint8_t len);

int8_t spi_tranceive_1(uint8_t* const buffer, uint8_t len);
int8_t spi_receive_1(uint8_t* const buffer, uint8_t len);
int8_t spi_transmit_1(uint8_t* const buffer, uint8_t len);

#endif