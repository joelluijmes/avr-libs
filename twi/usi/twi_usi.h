#pragma once

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#include "../twi.h"

#if defined (__AVR_ATtiny85__)
	#define DDR_USI             DDRB
	#define PORT_USI            PORTB
	#define PIN_USI             PINB
	#define PORT_USI_SDA        PORTB0
	#define PORT_USI_SCL        PORTB2
	#define PIN_USI_SDA         PINB0
	#define PIN_USI_SCL         PINB2
#else
	#error Device is not supported
#endif

TWRESULT usi_init_slave(uint8_t slave_addr);
TWRESULT usi_start_master(uint8_t slave_addr, uint8_t transmitting);
uint8_t usi_write_master(uint8_t data);
uint8_t usi_read_master(uint8_t nack);
uint8_t usi_write_slave(uint8_t data);
uint8_t usi_read_slave();
TWRESULT usi_stop();

static inline uint8_t usi_available() 
{
	return (USISR & (1 << USIOIF));
}

static inline uint8_t usi_is_stop()
{
	return (USISR & (1 << USIPF));
}