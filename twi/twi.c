#include "twi.h"

#define TRANSMIT 0x01
#define RECEIVE 0x00
#define NACK 0x01
#define ACK 0x00

#if defined (__AVR_ATmega328P__) || defined (__AVR_ATmega328__)
	#include "avr/twi_mega.h"

	#define MASTER_INIT() (twi_master_init())
	#define MT_START(slaveaddr) (twi_mt_start(slaveaddr))
	#define MR_START(slaveaddr) (twi_mr_start(slaveaddr))
	#define MASTER_WRITE(data) (twi_write(data))
	#define MASTER_READ(nack) (twi_read(nack))
	#define MASTER_STOP() (twi_stop(CLOSE))

	#define SLAVE_INIT(slaveaddr) (twi_slave_init(slaveaddr))
	#define SLAVE_WRITE(data) 0
	#define SLAVE_READ(p_data) 0
	#define SLAVE_IS_STOP() 0
	#define SLAVE_AVAILABLE() 0

#elif defined (__AVR_ATtiny85__)
	#include "usi/twi_usi.h"

	#define MASTER_INIT() ()	
	#define MT_START(slaveaddr) (usi_start_master(slaveaddr, TRANSMIT))
	#define MR_START(slaveaddr) (usi_start_master(slaveaddr, RECEIVE))
	#define MASTER_WRITE(data) (usi_write_master(data))
	#define MASTER_READ(nack) (usi_read_master(nack))
	#define MASTER_STOP() (usi_stop())

	#define SLAVE_INIT(slaveaddr) (usi_init_slave(slaveaddr))
	#define SLAVE_WRITE(data) (usi_write_slave(data))
	#define SLAVE_READ(p_data) (usi_read_slave(p_data))
	#define SLAVE_IS_STOP() (usi_is_stop())
	#define SLAVE_AVAILABLE() (usi_available())
#else
	#error "Device not supported"
#endif

#define CLOSED (1 << 0)
static uint8_t _state = CLOSED;

TWRESULT twi_master_send(uint8_t slaveaddr, uint8_t* buffer, uint8_t len, uint8_t keepAlive)
{
	if (_state & CLOSED)
	{
		if(MT_START(slaveaddr) != TWST_OK)
		{
			MASTER_STOP();
			return TWST_START_FAILED; 
		}

		_state &= ~CLOSED;
	}

	uint8_t nacked = 0;
	for (uint8_t i = 0; i < len; ++i)
	{
		uint8_t ack = MASTER_WRITE(buffer[i]);
		if (!ack)
		{
			nacked = 1;
			break;
		}
	}

	if (!keepAlive)
		twi_close();

	return (nacked)
		? TWST_PARTIAL_TRANSMIT
		: TWST_OK;
}

TWRESULT twi_slave_send(uint8_t slaveaddr, uint8_t* buffer, uint8_t* len)
{
	TWRESULT result = SLAVE_INIT(slaveaddr);
	if (result != TWST_SL_TRANSMITTING)
		return result;

	uint8_t i;
	for (i = 0; i < *len; ++i)
	{
		uint8_t ack = SLAVE_WRITE(buffer[i]);
		if (!ack)												// NACK received
			break;
	}

	if (*len < i - 1)											// were cut off by master (NACK)
	{
		*len = i - 1;											// set bytes we actually were able to send
		return TWST_PARTIAL_TRANSMIT;
	}

	return TWST_OK;
}

TWRESULT twi_master_receive(uint8_t slaveaddr, uint8_t* buffer, uint8_t len, uint8_t keepAlive)
{
	if (_state & CLOSED)
	{
		if(MR_START(slaveaddr) != TWST_OK)
		{
			twi_close();
			return TWST_START_FAILED;
		}

		_state &= ~CLOSED;
	}

	for (uint8_t i = 0; i < len - 1; ++i)
		buffer[i] = MASTER_READ(ACK);

	buffer[len - 1] = MASTER_READ(NACK); 

	if (!keepAlive)
		twi_close();

	return TWST_OK;
}

TWRESULT twi_slave_receive(uint8_t slaveaddr, uint8_t* buffer, uint8_t* len)
{
	TWRESULT result = SLAVE_INIT(slaveaddr);
	if (result != TWST_SL_RECEIVING)
		return result;

	uint8_t i;
	for (i = 0; i < *len; ++i)
	{
		if (SLAVE_READ(&buffer[i]) != TWST_OK)
			break;	
	}

	if (*len != i)												// were cut off by master (NACK)
	{
		*len = i;												// set bytes we actually were able to send
		return TWST_PARTIAL_READ;
	}

	return TWST_OK;
}

void twi_stop(uint8_t close)
{
	if (close)
		MASTER_STOP();
	_state |= CLOSED;
}

uint8_t twi_slave_available()
{
	return SLAVE_AVAILABLE();
}