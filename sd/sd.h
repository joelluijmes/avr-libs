/*
 * sd.h
 *
 * Created: 9-10-2015 19:22:35
 *  Author: joell
 */ 


#pragma once

#ifndef __AVR_ATmega328P__
#pragma message "warning: designed for ATmega328P. Could work when same SPI pin mapping."
#endif

#include <avr/io.h>
#include <inttypes.h>

// SD card types (sd_init returnsit)
#define TYPE_MMC 1
#define TYPE_SD1 2
#define TYPE_SD2 4
#define TYPE_SDC (TYPE_SD1 | TYPE_SD2)
#define TYPE_BLOCK 8

uint8_t sd_init();
uint8_t sd_read_address(uint32_t address, uint16_t count, uint8_t* buffer);
uint8_t sd_read_part(uint16_t block, uint16_t offset, uint16_t count, uint8_t* buffer);