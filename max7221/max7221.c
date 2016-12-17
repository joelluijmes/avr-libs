#include "max7221.h"
#include <util/delay.h>

#define DATA_HIGH() (MAX7221_DATA_PORT  |= MAX7221_DATA_MASK)
#define DATA_LOW()  (MAX7221_DATA_PORT  &= ~MAX7221_DATA_MASK)
#define CS_HIGH()   (MAX7221_CS_PORT    |= MAX7221_CS_MASK)
#define CS_LOW()    (MAX7221_CS_PORT    &= ~MAX7221_CS_MASK)
#define SCK_HIGH()  (MAX7221_SCK_PORT   |= MAX7221_SCK_MASK)
#define SCK_LOW()   (MAX7221_SCK_PORT   &= ~MAX7221_SCK_MASK)

#define KEEP_ALIVE  0
#define CLOSE       1
#define CLOSED      1

static void spi_write(uint8_t* arr, uint8_t len, uint8_t close);
static void spi_stop(uint8_t close);
static uint8_t _spi_state = CLOSED;

void max7221_init(void)
{
    CS_HIGH();
    SCK_HIGH();
    DATA_HIGH();
    
    max7221_write(MAX7221_SCANLIMIT, MAX7221_DEVICES-1);
    max7221_write(MAX7221_DECODEMODE, 0xFF);
    max7221_write(MAX7221_SHUTDOWN, 1);
    max7221_write(MAX7221_INTENSITY, 8);
    spi_stop(CLOSE);
}

void max7221_write(uint8_t address, uint8_t data)
{
    uint8_t arr[MAX7221_DEVICES*2] = { 0 };
    
    for (uint8_t i = 0; i < MAX7221_DEVICES; i += 2)
    {
        arr[i] = address;
        arr[i + 1] = data;
    }
    
    spi_write(arr, MAX7221_DEVICES*2, KEEP_ALIVE);
}

void max7221_display(uint8_t* numbers, uint8_t len)
{
    for (uint8_t i = 0; i < len; ++i)
        max7221_write(i + 1, numbers[i]);
    
    spi_stop(CLOSE);
}

static void spi_write(uint8_t* arr, uint8_t len, uint8_t close)
{
    if (_spi_state & CLOSED)
    {
        CS_LOW();
        _spi_state &= ~CLOSED;
    }        
    
    for (uint8_t i = 0; i < len; ++i)
    {
        for (int8_t j = 7; j >= 0; --j)        // MAX7221 SPI is MSB (Big endian)
        {
            if ((arr[i] & (1 << j)) != 0)
                DATA_HIGH();
            else
                DATA_LOW();
            
            _delay_us(100); 
            SCK_HIGH();
            _delay_us(100);            
            SCK_LOW();
            _delay_us(100);    
        }
    }    
    
    spi_stop(close);   
}

static inline void spi_stop(uint8_t close)
{
    if (close == CLOSE)
    {
        CS_HIGH();
        _spi_state = CLOSED;
    }
}