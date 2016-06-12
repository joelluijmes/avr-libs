#include "gfx.h"

#include <stdlib.h>
#include <math.h>
#include "ssd1306.h"

static uint8_t* _buf;

static inline void swap(int8_t* lhs, int8_t* rhs)
{
    int8_t tmp = *lhs;
    *lhs = *rhs;
    *rhs = tmp;
}
    
GFX_RESULT gfx_init()
{
    volatile SSD_RESULT err;    
    err = ssd_init();
    if (err != SSD_OK)
        return GFX_DIVER_ERROR;
        
    err = ssd_clear();
    if (err != SSD_OK)
        return GFX_DIVER_ERROR;
    
    err = ssd_update();
    if (err != SSD_OK)
        return GFX_DIVER_ERROR;
        
    _buf = ssd_get_buffer();
    return GFX_OK;
}

void gfx_pixel(int8_t x, int8_t y, int8_t on)
{
    uint8_t* pix = &_buf[x + (y/8) * SSD1306_LCDWIDTH];
    
    if (on)
        *pix |= (1 << (y & 7));
    else 
        *pix &= ~(1 << (y & 7));
}

void gfx_line(int8_t x1, int8_t y1, int8_t x2, int8_t y2, int8_t on)
{
    int8_t dx = abs(x2 - x1);
    int8_t dy = abs(y2 - y1);
        
    int8_t sx = x1 < x2 ? 1 : -1;
    int8_t sy = y1 < y2 ? 1 : -1;
    int8_t err = (dx > dy ? dx : -dy)/2;
    int8_t e2;
    
    while (1)
    {
        gfx_pixel(x1, y1, on);
        
        if (x1 == x2 && y1 == y2)
            break;
        
        e2 = err;
        if (e2 > -dx)
        {
            err -= dy;
            x1 += sx;
        }
        if (e2 < dy)
        {
            err += dx;
            y1 += sy;
        }
    }
}

void gfx_vline(int8_t x, int8_t y, int8_t height, int8_t on)
{
    if (height < 0)
    {
        y -= height;
        height *= -1;
    }
    
    if (y < 0)
    {
        height += y;
        y = 0;
    }
    
    if ((y + height) > SSD1306_LCDHEIGHT)
    {
        height = (SSD1306_LCDHEIGHT - y);
        if (height <= 0)
            return;
    }
    
    uint8_t* p = &_buf[(x + (y/8) * SSD1306_LCDWIDTH)];
    
    // set the first bit of the height
    int8_t modu = y & 7;
    if (modu)
    {
        modu = 8 - modu;
        
        // Lookup table is faster :D
        static int8_t premask[8] = {0x00, 0x80, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC, 0xFE };
        int8_t mask = premask[modu];
        
        // strip if we don't reach end with this mask
        if(height < modu)
            mask &= (0xFF >> (modu-height));
            
        if (on)
            *p |= mask;
        else
            *p &= ~mask;
        
        if (height < modu)      // early exit :D
            return;     
        
        height -= modu;
        p += SSD1306_LCDWIDTH;
    }
    
    // write solid 8 bytes for fill :D
    while (height >= 8)
    {
        if (on)
            *p |= 0xFF;
        else
            *p &= 0;
            
        p += SSD1306_LCDWIDTH;
        height -= 8;
    }
    
    // Completed :)
    if (!height)
        return;
        
    // Rest of the byte
    modu = height & 7;
    static int8_t postmask[8] = {0x00, 0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x7F };
    int8_t mask = postmask[modu];
    
    if (on)
        *p |= mask;
    else
        *p &= ~mask;
}

void gfx_hline(int8_t x, int8_t y, int8_t width, int8_t on)
{
    if (width < 0)
    {
        x += width;
        width *= -1;
    }
    
    if (x < 0)
    {
        width += x;
        x = 0;
    }
    
    if ((x + width) > SSD1306_LCDWIDTH)
    {
        width = SSD1306_LCDWIDTH - x;
        if (width < 0)
            return;
    }
    
    uint8_t* p = &_buf[(x + (y/8) * SSD1306_LCDWIDTH)];
    int8_t mask = 1 << (y & 7);
    
    if (on)
    {
        while (width--)
            *p++ |= mask;
    }
    else 
    {
        while (width--)
            *p++ &= ~mask;
    }
}

void gfx_angled_line(int8_t x, int8_t y, int16_t degrees, int8_t length, int8_t on)
{
    double rad = degrees*M_PI/180.0;
    
    int8_t x2 = x + length*cos(rad);
    int8_t y2 = y + length*sin(rad);
    
    gfx_line(x, y, x2, y2, on);
}

void gfx_angled_pixel(int8_t x, int8_t y, int16_t degrees, int8_t length, int8_t on)
{
    double rad = degrees*M_PI/180.0;
    
    int8_t x2 = x + length*cos(rad);
    int8_t y2 = y + length*sin(rad);
    
    gfx_pixel(x2, y2, on);
}

void gfx_rect(int8_t x, int8_t y, int8_t width, int8_t height, int8_t on)
{
    gfx_hline(x, y, width, on);
    gfx_vline(x, y, height, on);    
    gfx_hline(x, y + height -1, width, on);
    gfx_vline(x + width -1, y, height, on);
}

void gfx_fill_rect(int8_t x, int8_t y, int8_t width, int8_t height, int8_t on)
{
    for (int8_t i = x; i < width + x; ++i)
        gfx_vline(i, y, height, on);
}

void gfx_circle(int8_t x, int8_t y, int8_t radius, int8_t on)
{
    int8_t f = 1 - radius;
    int8_t ddF_x = 1;
    int8_t ddF_y = -2*radius;
    int8_t xx = 0;
    int8_t yy = radius;
    
    gfx_pixel(x, y + radius, on);
    gfx_pixel(x, y - radius, on);
    gfx_pixel(x + radius, y, on);
    gfx_pixel(x - radius, y, on);
    
    while (xx < yy) 
    {
        if (f >= 0)
        {
            --yy;
            ddF_y += 2;
            f += ddF_y;
        }
        
        ++xx;
        ddF_x += 2;
        f += ddF_x;
        
        gfx_pixel(x + xx, y + yy, on);
        gfx_pixel(x - xx, y + yy, on);
        gfx_pixel(x + xx, y - yy, on);
        gfx_pixel(x - xx, y - yy, on);
        gfx_pixel(x + yy, y + xx, on);
        gfx_pixel(x - yy, y + xx, on);
        gfx_pixel(x + yy, y - xx, on);
        gfx_pixel(x - yy, y - xx, on);
    }
}

void gfx_print_char(int8_t x, int8_t y, char c, gfx_char gfx)
{
    if ((uint8_t)(6*gfx.size) + x >= SSD1306_LCDWIDTH)
    {
        x = 0;
        y += gfx.size*6;        
    }
    
    if ((uint8_t)(8*gfx.size) + y >= SSD1306_LCDHEIGHT)
        y = 0;
    
    if (c >= 176)
        ++c;
        
    for (int8_t xx = 0; xx < 6; ++xx)
    {
        int8_t line = (xx < 5)
            ? pgm_read_byte(font + (c * 5) + xx)
            : 0;
        
        for (int8_t yy = 0; yy < 8; ++yy, line >>= 1) 
        {
            int8_t x1 = x + xx*gfx.size;
            int8_t y1 = y + yy*gfx.size;
            
            int8_t width = gfx.size;            
            int8_t height = gfx.size;
            
            if (line & 0x01)
            {
                if (gfx.size == 1)
                    gfx_pixel(x1, y1, gfx.color);
                else
                    gfx_fill_rect(x1, y1, width, height, gfx.color);
            }
            else
            {
                if (gfx.size == 1)
                    gfx_pixel(x1, y1, !gfx.color);
                else
                    gfx_fill_rect(x1, y1, width, height, !gfx.color);
            }
        }
    }
}

void gfx_print_text(int8_t x, int8_t y, const char* text, gfx_char gfx)
{
    const char* p = text;
    
    while (*p)
    {
        if (*p == '\n')
        {
            y += gfx.size*8;
            x = 0;
        }
        else if (*p == '\r')
        {
            continue;
        }
        else  
        {
            gfx_print_char(x, y, *p, gfx);
            x += gfx.size*6;
        }
        
        ++p;
    }
}

void gfx_bitmap(const uint8_t* ptr)
{
    int8_t data = 0;
    int8_t count = 0;
    uint8_t* p_buf = _buf;
    
    for (uint16_t i = 0; i < 1024; i++)
    {
        if(count == 0)
        {
            data = *(++ptr);
            if(data == *(++ptr))
            {
                count = *(++ptr);
            }
            else
            {
                count = 1;
                --ptr;
            }
        }
        
        --count;
        *p_buf++ = data;
    }
}

void gfx_bitmap_progmem(const uint8_t* progmem)
{
    int8_t data = 0;
    int8_t count = 0;
    uint8_t* p = _buf;
    
    for (uint16_t i = 0; i < 1024; i++)
    {
        if(count == 0)
        {
            data = pgm_read_byte_near(progmem++);
            if(data == pgm_read_byte_near(progmem++))
            {
                count = pgm_read_byte_near(progmem++);
            }
            else
            {
                count = 1;
                progmem--;
            }
        }
        count--;
        
        *p++ |= data;
    }
}

void gfx_clear()
{
    ssd_clear();
}

GFX_RESULT gfx_update()
{
    return ssd_update();
}