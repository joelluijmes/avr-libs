#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/twi.h>
#include <util/delay.h>

#include "../../twi.h"
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
	puts("Slave Transmit!");
	
	twi_slave_init(0x08);
	printf("Enter char to send: ");
	while (1)
	{
		char c = getchar();
		twi_write(c);
		printf("Status: %x\n", TW_STATUS);

		if (TW_STATUS != TW_ST_DATA_ACK)
			break;
		PINB = 0x20;
	}

	puts("Disconnected");
	return 0;
}