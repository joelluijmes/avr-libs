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

	volatile TWRESULT status = usi_init_slave(0x08);
	LED_HIGH();
	
	while (1)
	{
		volatile uint8_t data = usi_read_slave();
		//TODO : stop check
	}
	
	LED_LOW();

	while(1);
	return 0;
}
