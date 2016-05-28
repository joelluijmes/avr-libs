#pragma once
#include <inttypes.h>

#pragma region commands
#define BLACK 0
#define WHITE 1
#define INVERSE 2

#define SSD1306_I2C_ADDRESS  0x3C

// Pick the correct dimension screen
#define SSD1306_128_64
//#define SSD1306_128_32
//#define SSD1306_96_16

#if defined SSD1306_128_64 && defined SSD1306_128_32
  #error "Only one SSD1306 display can be specified at once in SSD1306.h"
#endif
#if !defined SSD1306_128_64 && !defined SSD1306_128_32 && !defined SSD1306_96_16
  #error "At least one SSD1306 display must be specified in SSD1306.h"
#endif

#if defined SSD1306_128_64
    #define SSD1306_LCDWIDTH        128
    #define SSD1306_LCDHEIGHT       64

    #define SSD1306_MAXCOLUMN       0x7F
    #define SSD1306_MAXPAGE         0x07
#endif
#if defined SSD1306_128_32
    #define SSD1306_LCDWIDTH        128
    #define SSD1306_LCDHEIGHT       32

    #define SSD1306_MAXCOLUMN       0x7F
    #define SSD1306_MAXPAGE         0x03
#endif
#if defined SSD1306_96_16
    #define SSD1306_LCDWIDTH        96
    #define SSD1306_LCDHEIGHT       16

    #define SSD1306_MAXCOLUMN       0x3F
    #define SSD1306_MAXPAGE         0x03
#endif

#define SSD1306_SETCONTRAST         0x81
#define SSD1306_DISPLAYALLON_RESUME 0xA4
#define SSD1306_DISPLAYALLON        0xA5
#define SSD1306_NORMALDISPLAY       0xA6
#define SSD1306_INVERTDISPLAY       0xA7
#define SSD1306_DISPLAYOFF          0xAE
#define SSD1306_DISPLAYON           0xAF

#define SSD1306_SETDISPLAYOFFSET    0xD3
#define SSD1306_SETCOMPINS          0xDA

#define SSD1306_SETVCOMDETECT       0xDB

#define SSD1306_SETDISPLAYCLOCKDIV  0xD5
#define SSD1306_SETPRECHARGE        0xD9

#define SSD1306_SETMULTIPLEX        0xA8

#define SSD1306_SETLOWCOLUMN        0x00
#define SSD1306_SETHIGHCOLUMN       0x10

#define SSD1306_SETSTARTLINE        0x40

#define SSD1306_MEMORYMODE          0x20
#define SSD1306_COLUMNADDR          0x21
#define SSD1306_PAGEADDR            0x22

#define SSD1306_COMSCANINC          0xC0
#define SSD1306_COMSCANDEC          0xC8

#define SSD1306_SEGREMAP            0xA0

#define SSD1306_CHARGEPUMP          0x8D

#define SSD1306_EXTERNALVCC         0x01
#define SSD1306_SWITCHCAPVCC        0x02

#define SSD1306_ACTIVATE_SCROLL                         0x2F
#define SSD1306_DEACTIVATE_SCROLL                       0x2E
#define SSD1306_SET_VERTICAL_SCROLL_AREA                0xA3
#define SSD1306_RIGHT_HORIZONTAL_SCROLL                 0x26
#define SSD1306_LEFT_HORIZONTAL_SCROLL                  0x27
#define SSD1306_VERTICAL_AND_RIGHT_HORIZONTAL_SCROLL    0x29
#define SSD1306_VERTICAL_AND_LEFT_HORIZONTAL_SCROLL     0x2A

#pragma endregion commands

typedef int8_t SSD_RESULT;
#define SSD_ERROR 0
#define SSD_OK 1
#define SSD_TWI_ERROR 2

#define ON 1
#define OFF 0 

SSD_RESULT ssd_init();
SSD_RESULT ssd_clear();
SSD_RESULT ssd_display(int8_t on);
SSD_RESULT ssd_contrast(int8_t contrast);
SSD_RESULT ssd_inverse(int8_t inverse);
SSD_RESULT ssd_update();
SSD_RESULT ssd_set_column(int8_t start, int8_t end);
SSD_RESULT ssd_set_page(int8_t start, int8_t end);
uint8_t* ssd_get_buffer();