#include "mux4067.h"
#include <util/delay.h>

#define ASSERT_S1() (MUX4067_S1_PORT |= MUX4067_S1_MASK)
#define DISABLE_S1() (MUX4067_S1_PORT &= ~MUX4067_S1_MASK)
#define ASSERT_S2() (MUX4067_S2_PORT |= MUX4067_S2_MASK)
#define DISABLE_S2() (MUX4067_S2_PORT &= ~MUX4067_S2_MASK)
#define ASSERT_S3() (MUX4067_S3_PORT |= MUX4067_S3_MASK)
#define DISABLE_S3() (MUX4067_S3_PORT &= ~MUX4067_S3_MASK)
#define ASSERT_S4() (MUX4067_S4_PORT |= MUX4067_S4_MASK)
#define DISABLE_S4() (MUX4067_S4_PORT &= ~MUX4067_S4_MASK)
#define ASSERT_EN() (MUX4067_EN_PORT &= ~MUX4067_EN_MASK)
#define DISABLE_EN() (MUX4067_EN_PORT |= MUX4067_EN_MASK)

void mux4067_init(void)
{
    //Starts AD in Free Running mode with prescaler of 128
    ADMUX = 1 << REFS0 | MUX4067_ADC_MASK;			// AVCC as reference | ADC0
    ADCSRA = 1 << ADEN | 1 << ADPS2 | 1 << ADPS1 | 1 << ADPS0;
    
    ASSERT_EN();
}

uint16_t mux4067_read(uint8_t selection)
{
    ADCSRA |= 1 << ADSC;    // start adc
    
    if ((selection & (1 << 1)) != 0)
        ASSERT_S1();
    else
        DISABLE_S1();
    
    if ((selection & (1 << 2)) != 0)
        ASSERT_S2();
    else
        DISABLE_S2();
    
    if ((selection & (1 << 3)) != 0)
        ASSERT_S3();
    else
        DISABLE_S3();
    
    if ((selection & (1 << 4)) != 0)
        ASSERT_S4();
    else
        DISABLE_S4();
    
    while ((ADCSRA & ADSC) == 1) ; // = 1 as long conversion is in progress
    return ADC;
}
