// Generic driver wrapper for i2c. Currently it works for the ATmega328 and ATtiny85
// it could be compatible on many more ATmegas and ATtinys (not tested.)

// Author: Joël Luijmes

#include "twi.h"

#define TRANSMIT 0x01
#define RECEIVE 0x00
#define NACK 0x01
#define ACK 0x00

#if defined (__AVR_ATmega328P__) || defined (__AVR_ATmega328__)
    #include "avr/twi_mega.h"

    #define MASTER_INIT() (mega_master_init())
    #define MT_START(slaveaddr) (mega_mt_start(slaveaddr))
    #define MR_START(slaveaddr) (mega_mr_start(slaveaddr))
    #define MASTER_WRITE(data) (mega_write(data))
    #define MASTER_READ(nack) (mega_read(nack))
    #define MASTER_STOP() (mega_close())

    #define SLAVE_INIT(slaveaddr) (mega_slave_init(slaveaddr))
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

void twi_master_init()
{
    MASTER_INIT();
}

TWRESULT twi_master_send(uint8_t slaveaddr, uint8_t* buffer, uint8_t len, uint8_t keepAlive)
{
    if (_state & CLOSED)                            // check if we have the bus (i.e. KEEP_ALIVE)
    {
        if(MT_START(slaveaddr) != TWST_OK)          // try to start a transfer with target slaveaddr
        {                                           // however we failed :(
            twi_stop(CLOSE);                        // so give the bus fre
            return TWST_START_FAILED;               // return our fail
        }

        _state &= ~CLOSED;                          // we got hold of the bus
    }

    uint8_t nacked = 0;
    for (uint8_t i = 0; i < len; ++i)               // while we have a buffer to send
    {
        uint8_t ack = MASTER_WRITE(buffer[i]);      // send it to the slave
        if (!ack)                                   // if we didn't get an ACK from the slave
        {                                           // store that result
            nacked = 1;
            break;
        }
    }

    twi_stop(keepAlive);                            // close the bus if needed
    return (nacked)                                 // if we prematurely failed
        ? TWST_PARTIAL_TRANSMIT                     // tell so
        : TWST_OK;                                  // if not great :)
}

TWRESULT twi_slave_send(uint8_t slaveaddr, uint8_t* buffer, uint8_t* len)
{
    TWRESULT result = SLAVE_INIT(slaveaddr);        // initialize as slave
    if (result != TWST_SL_TRANSMITTING)             // if we didn't get in the send mode
        return result;                              // something went wrong :(

    uint8_t i;
    for (i = 0; i < *len; ++i)                      // while we have a buffer to send
    {                                               
        uint8_t ack = SLAVE_WRITE(buffer[i]);       // send the data
        if (!ack)                                   // if we got a NACK
            break;                                  // master wants to stop the transmission
    }                                               

    if (*len < i - 1)                               // if we didn't send the full buffer
    {                                               // we received a nack so
        *len = i - 1;                               // set len we actually were able to send
        return TWST_PARTIAL_TRANSMIT;               // return our patial read result
    }                                               

    return TWST_OK;                                 // send everything
}

TWRESULT twi_master_receive(uint8_t slaveaddr, uint8_t* buffer, uint8_t len, uint8_t keepAlive)
{
    if (_state & CLOSED)                            // check if we have the bus (i.e. KEEP_ALIVE)
    {
        if(MR_START(slaveaddr) != TWST_OK)          // try to start a transfer with target slaveaddr
        {                                           // however we failed :(
            twi_stop(CLOSE);                        // so give the bus fre
            return TWST_START_FAILED;               // return our fail
        }

        _state &= ~CLOSED;                          // we got hold of the bus
    }

    for (uint8_t i = 0; i < len - 1; ++i)           // for all but one bytes receive data 
        buffer[i] = MASTER_READ(ACK);               // fill the buffer (ACK means more to expect)

    buffer[len - 1] = MASTER_READ(NACK);            // for the last byte send NACK to close the 
                                                    // transmission
    twi_stop(keepAlive);                            // close the bus as specified by the caller
    return TWST_OK;                                 // return it went ok 
}

TWRESULT twi_slave_receive(uint8_t slaveaddr, uint8_t* buffer, uint8_t* len)
{
    TWRESULT result = SLAVE_INIT(slaveaddr);        // initialize as slave
    if (result != TWST_SL_RECEIVING)                // if we didn't get in the receiving mode
        return result;                              // something went wrong :(

    uint8_t i;
    for (i = 0; i < *len; ++i)                      // while we have a buffer to fill
    {
        if (SLAVE_READ(&buffer[i]) != TWST_OK)      // read a byte, but if we received NACK
            break;                                  // stop reading
    }

    if (*len != i)                                  // if we didn't fill the full buffer
    {                                               // we received a nack so
        *len = i;                                   // set bytes we actually were able to receive
        return TWST_PARTIAL_READ;                   // return our patial read result
    }

    return TWST_OK;                                 // filled the fullbuffer
}

void twi_stop(uint8_t close)
{
    if (close == CLOSE)                             // if we really want to give the bus free
        MASTER_STOP();                              // do so :)

    if (close)                                      // CLOSE || REP_START
        _state |= CLOSED;                           // closes our internal state -> allows for repeated start
}

uint8_t twi_slave_available()
{
    return SLAVE_AVAILABLE();
}
