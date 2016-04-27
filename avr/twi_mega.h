/*
*	Module for twi/i2c interface
*
*	Author: Joël Luijmes
*	Data: 04-06-2015 (updated: 14-12-2015)
*
*	Uses the avr twi interface, designed for mlx90614 -> only supports
*	MasterTransmit and MasterReceiver mode./
*/


#pragma once

#include <util/twi.h>
#include <stdio.h>

#include "../twi.h"

// 100 kHz as clock
#define SCL_CLOCK 800000L

// Initializes as master
void twi_master_init();

// Initializes as slave with address 
TWRESULT twi_slave_init(uint8_t slave_addr);

// Writes data to bus (note: should be in transmit mode)
uint8_t twi_write(uint8_t data);

// Starts transmission mode (MASTER)
TWRESULT twi_mt_start(uint8_t slave_addr);

// Starts receiver mode (MASTER)
TWRESULT twi_mr_start(uint8_t slave_addr);

// Releases the bus (must be master)
void twi_stop();

// Reads data from the bus (sends ACK)
uint8_t twi_read(uint8_t nack);
