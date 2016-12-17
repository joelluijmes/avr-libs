#include "stp16cpc26.h"
#include <util/delay.h>

#define ASSERT_CLK() (STP16_CLK_PORT |= STP16_CLK_MASK)
#define DISABLE_CLK() (STP16_CLK_PORT &= ~STP16_CLK_MASK)
#define ASSERT_SDI() (STP16_SDI_PORT |= STP16_SDI_MASK)
#define DISABLE_SDI() (STP16_SDI_PORT &= ~STP16_SDI_MASK)
#define ASSERT_LE() (STP16_LE_PORT |= STP16_LE_MASK)
#define DISABLE_LE() (STP16_LE_PORT &= ~STP16_LE_MASK)
#define ASSERT_OE() (STP16_OE_PORT &= ~STP16_OE_MASK)
#define DISABLE_OE() (STP16_OE_PORT |= STP16_OE_MASK)

void stp16_output(uint8_t enable)
{
    if (enable)
        ASSERT_OE();
    else
        DISABLE_OE();
}

void stp16_latch(void)
{
    ASSERT_LE();
    _delay_us(100);
    DISABLE_LE();
}

void stp16_shift(uint16_t val)
{
    for (uint8_t i = 0; i < 16; ++i)
    {
        if ((val & (1 << i)) != 0)
            ASSERT_SDI();
        else
            DISABLE_SDI();
        
        _delay_us(100);
        ASSERT_CLK();
        _delay_us(100);
        DISABLE_CLK();
        _delay_us(100);
    }
}