/*
 * sd.c
 *
 * Created: 9-10-2015 19:22:28
 *  Author: joell
 */ 

#include "sd.h"

// SPI pins
#define SD_SS 1 << PB2
#define SD_MOSI 1 << PB3
#define SD_MISO 1 << PB4
#define SD_SCK 1 << PB5

#define CMD_RESET 0
#define CMD_OP_COND 1
#define CMD_IF_COND 8
#define CMD_READ_CSD 9
#define CMD_READ_CID 10
#define CMD_STOP_READ 12
#define CMD_BLOCK_LENGTH 16
#define CMD_READ_BLOCK 17
#define CMD_READ_BLOCKS 18
#define CMD_WRITE_BLOCK 24
#define CMD_WRITE_BLOCKS 25
#define CMD_APP 55
#define CMD_READ_OCR 58

#define ACMD_APP_OP_COND (0x80 + 41)

#define DATA_START_BLOCK 0xFE

// SD response
#define R1_IDLE (1 << 0)
#define R1_ERASE_RESET (1 << 1)
#define R1_ILLIGAL_CMD (1 << 2)
#define R1_CRC_ERROR (1 << 3)
#define R1_ERASE_SEQ_ERROR (1 << 4)
#define R1_ADDRESS_ERROR (1 << 5)
#define R1_PARAMETER_ERROR (1 << 6)

static uint8_t spi_tranceive(uint8_t data);
static uint8_t sd_command(uint8_t command, uint32_t argument);
static uint8_t sd_wait_transfer();
static uint8_t sd_read(uint16_t offset, uint16_t count, uint8_t* buffer);
static uint8_t sd_read_blocks(uint16_t block, uint8_t count, uint8_t* buffer);

uint8_t sd_init()
{   // Thank you http://elm-chan.org/docs/mmc/mmc_e.html
    PORTB |= SD_SS;
    for (uint8_t i = 0; i < 10; ++i)                                        // 80 cycles so the card can get ready
        spi_tranceive(0xFF);
    PORTB &= ~SD_SS;
    
    if (sd_command(CMD_RESET, 0) != R1_IDLE)                                // reset the card, enter SPI
        return 0;
    
    uint8_t ocr[4];
    uint8_t type = 0;
    if (sd_command(CMD_IF_COND, 0x1AA) == R1_IDLE)                          // Maybe it's SDv2..
    {
        for (uint8_t i = 0; i < 4; ++i)                                     // discards trailing R7 response
        ocr[i] = spi_tranceive(0xFF);
        
        if (ocr[2] == 0x01 && ocr[3] == 0xAA)                               // if the card operates between 2.7 - 3.6
        {
            while (sd_command(ACMD_APP_OP_COND, 1UL << 30)) ;               // wait for card leave Idle state
            if (sd_command(CMD_READ_OCR, 0) == 0)                           // check the CCS Bit
            {
                for (uint8_t i = 0; i < 4; ++i)
                ocr[i] = spi_tranceive(0xFF);
                
                type = (ocr[0] & 0x40) ? TYPE_BLOCK | TYPE_SD2 : TYPE_SD2;  // SDv2
            }
        }
    }
    else                                                                    // SDv1 or MMCv3
    {
        uint8_t command;
        if (sd_command(ACMD_APP_OP_COND, 0) <= 1)                           // SDv1
        {
            type = TYPE_SD1;
            command = ACMD_APP_OP_COND;
        }
        else                                                                // MMCv3
        {
            type = TYPE_MMC;
            command = CMD_OP_COND;
        }
        
        while (sd_command(command, 0)) ;                                    // wait card leave idle state
        if (sd_command(CMD_BLOCK_LENGTH, 512) != 0)                         // set block length at 512 (can be 1024 on some cards)
            type = 0;
    }
        
    return type;
}

uint8_t sd_read_address(uint32_t address, uint16_t count, uint8_t* buffer)
{
    uint16_t lowBoundaryOffset = address % 512;
    uint32_t lowAddress = (lowBoundaryOffset != 0)              // if address is not aligned on 512
            ? address - lowBoundaryOffset                       // align it on 512 block
            : address;
    uint16_t lowBlock = lowAddress >> 9;

    uint32_t endAddress = address + count;

    uint16_t highBoundaryOffset = endAddress % 512;
    uint32_t highAddress = (highBoundaryOffset != 0)            // align it (same as above)
            ? endAddress - highBoundaryOffset
            : endAddress;
    uint16_t highBlock = highAddress >> 9;

    if (lowBlock == highBlock)                                  // doesn't span multiple blocks
    {
        sd_read_part(lowBlock, lowBoundaryOffset, count, buffer);
        return 1;
    }

    if (lowBoundaryOffset > 0)                                  // if there is a offset between the lowBlock
    {
        uint16_t reading = 512 - lowBoundaryOffset;             // read the first part
        sd_read_part(lowBlock, lowBoundaryOffset, reading, buffer);

        buffer += reading;
        if (count - reading == 0)                               // no data left -> done (count < 512)
            return 1;
    }

    uint16_t blocks = highBlock - lowBlock;                     // amount of blocks to read
    if (highBoundaryOffset > 0)                                 // if there was a unaligned start
        --blocks;                                               // fix our calculation

    if (blocks > 0)                                             // if there are any blocks to be read
    {                                                           // (refix our calculation if needed)
        uint16_t startBlock = highBoundaryOffset > 0 ? lowBlock + 1 : lowBlock;
        sd_read_blocks(startBlock, blocks, buffer);             // again fix unaligned start
        
        buffer += blocks << 9;
    }

    if (highBoundaryOffset > 0)                                 // unaligned end -> (i.e. count == 513)
        sd_read_part(highBlock, 0, highBoundaryOffset, buffer);

    return 1;
}

uint8_t sd_read_blocks(uint16_t block, uint8_t count, uint8_t* buffer)
{
    if (count == 1)                                             // only one block
        return sd_read_part(block, 0, 512, buffer);
    
    if (sd_command(CMD_READ_BLOCKS, block) != 0)                // prepare read multiple blocks
        return 0;                                               
    
    do                                                          // for block count read blocks
    {
        if (!sd_read(0, 512, buffer))
            break;
        
        buffer += 512;
    } while (--count > 0);
    
    sd_command(CMD_STOP_READ, 0);                               // issue we are done reading blocks
    return count == 0;                                          // returns true if we readed everything
}

static uint8_t spi_tranceive(uint8_t data)
{
    SPDR = data;
    
    while (!(SPSR & (1 << SPIF)))                               // wait till received
        ;
    return SPDR;                                                // returns response
}

static uint8_t sd_command(uint8_t command, uint32_t argument)
{
    uint8_t response;
    if (command & 0x80)                                         // handle ACMD
    {
        command &= 0x7F;                                        // masks of the ACMD
        response = sd_command(CMD_APP, 0);
        if (response > 1)
            return response;
    }
    
    spi_tranceive(command | 0x40);                              // sends command
    spi_tranceive(argument >> 24);                              // sens command argument MSB first)
    spi_tranceive(argument >> 16);
    spi_tranceive(argument >> 8);
    spi_tranceive(argument);
    
    // crc is not needed in SPI mode except for initializing
    switch (command)
    {
        case CMD_RESET:
            spi_tranceive(0x95);
            break;
        case CMD_IF_COND:
            spi_tranceive(0x87);
            break;
        default:
            spi_tranceive(0xFF);
            break;
    }
    
    response = 0x80;            
    while (response & 0x80)
        response = spi_tranceive(0xFF);                         // waits for response
    
    return response;
}

static uint8_t sd_wait_transfer()
{
    uint8_t response = 0;
    while ((response = spi_tranceive(0xFF)) == 0xFF)            // waits for card gets ready..
        ;
    
    return response == DATA_START_BLOCK;
}

static uint8_t sd_read(uint16_t offset, uint16_t count, uint8_t* buffer)
{
    if (!sd_wait_transfer())
        return 0;
    
    for (uint16_t i = offset; i > 0; --i)                       // discards what's in front
        spi_tranceive(0xFF);
    
    for (uint16_t i = 0; i < count; ++i)
        buffer[i] = spi_tranceive(0xFF);
    
    uint16_t left = 512 - offset - count;                       // discards what's after the requested data
    for (uint16_t i = left; i > 0; --i)
        spi_tranceive(0xFF);
    
    //discards CRC
    spi_tranceive(0xFF);
    spi_tranceive(0xFF);
    
    return 1;
}

uint8_t sd_read_part(uint16_t block, uint16_t offset, uint16_t count, uint8_t* buffer)
{
    if (sd_command(CMD_READ_BLOCK, block) != 0)                 // requests block
        return 0;
    
    return sd_read(offset, count, buffer);
}


