#include "ssd1306.h"
#include "twi.h"

#include <string.h>

#define BUF_BYTES (SSD1306_LCDWIDTH*SSD1306_LCDHEIGHT/8)

typedef int8_t SSD_COMMAND;

static TWRESULT send_command(SSD_COMMAND command);
static SSD_RESULT send_commands(SSD_COMMAND commands[], int8_t len);

static int8_t _buffer[BUF_BYTES];

// Source: https://github.com/adafruit/Adafruit_SSD1306/blob/master/Adafruit_SSD1306.cpp
static SSD_COMMAND _init_sequence[] =
{
	SSD1306_DISPLAYOFF, SSD1306_SETDISPLAYCLOCKDIV, 0x80,							// 0x80 is suggested ratio
	SSD1306_SETMULTIPLEX, SSD1306_LCDHEIGHT - 1,
	SSD1306_SETDISPLAYOFFSET, 0x00, SSD1306_SETSTARTLINE, SSD1306_CHARGEPUMP, 0x14,	// external vcc (0x14 is internal)
	SSD1306_MEMORYMODE, 0x00, SSD1306_SEGREMAP | 0x01, SSD1306_COMSCANDEC,
	#if defined SSD1306_128_64
	SSD1306_SETCOMPINS, 0x12, SSD1306_SETCONTRAST, 0xCF,							// external vcc (0xCF is internal)
	#elif defined SSD1306_128_32
	SSD1306_SETCOMPINS, 0x02, SSD1306_SETCONTRAST, 0x8F,
	#elif defined SSD1306_96_16
	SSD1306_SETCOMPINS, 0x02, SSD1306_SETCONTRAST, 0x10,							// external vcc (0xAF is internal)
	#endif
	SSD1306_SETPRECHARGE, 0xF1,														// external vcc (0xF1 is internal)
	SSD1306_SETVCOMDETECT, 0x40, SSD1306_DISPLAYALLON_RESUME, SSD1306_NORMALDISPLAY,
	SSD1306_DISPLAYON
};

SSD_RESULT ssd_init() 
{
	return send_commands(_init_sequence, sizeof(_init_sequence));
}

SSD_RESULT ssd_clear()
{
	memset(_buffer, 0, BUF_BYTES);
	return SSD_OK;
}

SSD_RESULT ssd_display(int8_t on)
{
	return !!(send_command(on ? SSD1306_DISPLAYON : SSD1306_DISPLAYOFF) == TWST_OK);
}

SSD_RESULT ssd_contrast(int8_t contrast)
{
	SSD_COMMAND commands[] = { SSD1306_SETCONTRAST, contrast };
	return send_commands(commands, sizeof(commands));
}

SSD_RESULT ssd_inverse(int8_t inverse)
{
	SSD_COMMAND commands[] = { SSD1306_INVERTDISPLAY, inverse };
	return send_commands(commands, sizeof(commands));
}

SSD_RESULT ssd_update()
{
	int8_t i, j;
	SSD_RESULT ssd_result;
	TWRESULT tw_result;

	ssd_result = ssd_set_column(0x00, SSD1306_MAXCOLUMN);
	if (ssd_result != SSD_OK)
		return ssd_result;

	ssd_result = ssd_set_page(0x00, SSD1306_MAXPAGE);
	if (ssd_result != SSD_OK)
		return ssd_result;

	for (i = 0; i < BUF_BYTES/16; ++i)
	{
		tw_result = twi_master_send_byte(SSD1306_I2C_ADDRESS, 0x40, KEEP_ALIVE);
		if (tw_result != TWST_OK)
			return SSD_TWI_ERROR;

		for (j = 0; j < 16; ++j)
		{
			tw_result = twi_master_send_byte(SSD1306_I2C_ADDRESS, _buffer[i*16 + j], KEEP_ALIVE);
			if (tw_result != TWST_OK)
				return SSD_TWI_ERROR;
		}

		twi_stop(CLOSE);
	}

	return SSD_OK;
}

SSD_RESULT ssd_set_column(int8_t start, int8_t end)
{
	SSD_COMMAND commands[] = { SSD1306_COLUMNADDR, (start & SSD1306_MAXCOLUMN), (end & SSD1306_MAXCOLUMN) };
	return send_commands(commands, sizeof(commands));
}

SSD_RESULT ssd_set_page(int8_t start, int8_t end)
{
	SSD_COMMAND commands[] = { SSD1306_PAGEADDR, (start & SSD1306_MAXPAGE), (end & SSD1306_MAXPAGE) };
	return send_commands(commands, sizeof(commands));
}

uint8_t* ssd_get_buffer()
{
	return _buffer;
}

static TWRESULT send_command(SSD_COMMAND command)
{
	uint8_t buf[] = { 0x00, command };
	return twi_master_send(SSD1306_I2C_ADDRESS, buf, 2, CLOSE);
}

static SSD_RESULT send_commands(SSD_COMMAND commands[], int8_t len)
{
	int8_t i;
	TWRESULT result;

	for (i = 0; i < len; ++i)
	{
		result = send_command(commands[i]);
		if (result != TWST_OK)
			return SSD_TWI_ERROR;
	}

	return SSD_OK;
}

