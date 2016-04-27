#include <util/delay.h>
#include <avr/io.h>

#include "twi.h"
#include "twi_usi.h"

#define PORT_LED PORTB
#define DDR_LED DDRB
#define PIN_LED PINB
#define MASK_LED (1 << 4)

#define LED_HIGH() (PORT_LED |= MASK_LED)
#define LED_LOW() (PORT_LED &= ~MASK_LED)
#define LED_TOGGLE() (PIN_LED = MASK_LED)
#define LED_OUTPUT() (DDR_LED |= MASK_LED)

int main()
{
	LED_OUTPUT();

	usi_init_master();
	volatile TWRESULT status = usi_start_master(0x08, 1);
	if (status != TWST_OK)
	{
		while(1)
		{
			LED_TOGGLE();
			_delay_ms(100);
		}
	}

	LED_HIGH();
	
	for (uint8_t i = 0; i < 25; ++i)
	{
		volatile uint8_t data = usi_read_master(0);
	}
	usi_read_master(1);

	usi_stop();
	LED_LOW();

	while(1);
	return 0;
}
