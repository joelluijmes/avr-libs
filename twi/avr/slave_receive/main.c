#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/twi.h>
#include <util/delay.h>

#include "../twi_mega.h"
#include "../../../uart/uart.h"

FILE mystdout = FDEV_SETUP_STREAM(uart_putchar, NULL, _FDEV_SETUP_WRITE);
FILE mystdin = FDEV_SETUP_STREAM(NULL, uart_getchar, _FDEV_SETUP_READ);

int main()
{
	DDRB = 0x20;
	PORTC = 1 << 4 | 1 << 5;
	stdout = &mystdout;
	stdin = &mystdin;

	uart_init();
	puts("Slave Receive!");
	
	PORTB = 0x20;
	twi_slave_init(0x08);
	printf("%x", TW_STATUS);
	PINB = 0x20;
	puts("Connected");

	while (TW_STATUS != TW_SR_STOP)
	{
		uint8_t c = twi_read();
		printf("Char: %x Status: %x\n", c, TW_STATUS);

		if (TW_STATUS != TW_SR_SLA_ACK)
			break;
		
		PINB = 0x20;
	}

	return 0;
}