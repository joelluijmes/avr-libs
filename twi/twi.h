// Generic driver wrapper for i2c. Currently it works for the ATmega328 and ATtiny85
// it could be compatible on many more ATmegas and ATtinys (not tested.)

// Author: Joël Luijmes

#pragma once
#include <inttypes.h>

#define KEEP_ALIVE 0
#define CLOSE 1
#define REP_START 2

// Status codes of what this module will return
typedef uint8_t TWRESULT;
#define TWST_OK 1
#define TWST_START_FAILED 2

#define TWST_MASTER_NACK 3
#define TWST_MASTER_RECEIVE_FAILED 7

#define TWST_SL_TRANSMITTING 4
#define TWST_SL_RECEIVING 5

#define TWST_STOP_FAILED 6
#define TWST_PARTIAL_TRANSMIT 7
#define TWST_PARTIAL_READ 8
#define TWST_SLAVE_RECEIVE_FAILED 9
#define TWST_SL_STOP 10

void twi_master_init();
TWRESULT twi_master_send(uint8_t slaveaddr, uint8_t* buffer, uint8_t len, uint8_t keepAlive); 
TWRESULT twi_slave_send(uint8_t slaveaddr, uint8_t* buffer, uint8_t* len);
TWRESULT twi_master_receive(uint8_t slaveaddr, uint8_t* buffer, uint8_t len, uint8_t keepAlive);
TWRESULT twi_slave_receive(uint8_t slaveaddr, uint8_t* buffer, uint8_t* len);
void twi_stop(uint8_t close);
uint8_t twi_slave_available();

static inline TWRESULT twi_master_send_byte(uint8_t slaveaddr, uint8_t data, uint8_t keepAlive)
{
    uint8_t buf[] = { data };
    return twi_master_send(slaveaddr, buf, 1, keepAlive);
}

static inline TWRESULT twi_slave_send_byte(uint8_t slaveaddr, uint8_t data)
{
    uint8_t buf[] = { data };
    uint8_t len = 1;
    return twi_slave_send(slaveaddr, buf, &len);
}

static inline TWRESULT twi_master_receive_byte(uint8_t slaveaddr, uint8_t* data, uint8_t keepAlive)
{
    uint8_t buf[1];
    if (twi_master_receive(slaveaddr, buf, 1, keepAlive) != TWST_OK)
        return TWST_MASTER_RECEIVE_FAILED;

    *data = buf[0];
    return TWST_OK;
}

static inline TWRESULT twi_slave_receive_byte(uint8_t slaveaddr, uint8_t* data)
{
    uint8_t buf[1];
    uint8_t len = 1;
    if (twi_slave_receive(slaveaddr, buf, &len) != TWST_OK)
        return TWST_SLAVE_RECEIVE_FAILED;

    *data = buf[0];
    return TWST_OK;
}