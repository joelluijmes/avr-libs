#include "twi_usi.h"

#define SCL_HIGH() PORT_USI |= (1<<PIN_USI_SCL)
#define SCL_LOW() PORT_USI &= ~(1<<PIN_USI_SCL)
#define SDA_LOW() PORT_USI &= ~(1<<PIN_USI_SDA)
#define SDA_HIGH() PORT_USI |= (1<<PIN_USI_SDA)

#define SDA_OUTPUT() DDR_USI |= (1<<PIN_USI_SDA)
#define SDA_INPUT() DDR_USI &= ~(1<<PIN_USI_SDA)
#define SCL_OUTPUT() DDR_USI |= (1<<PIN_USI_SCL)
#define SCL_INPUT() DDR_USI &= ~(1<<PIN_USI_SCL)

#define IS_SCL_HIGH() (PIN_USI & (1<<PIN_USI_SCL))
#define IS_SDA_HIGH() (PIN_USI & (1<<PIN_USI_SDA))

#define SR_RESET (1 << USISIF | 1 << USIOIF | 1 << USIPF | 1 << USIDC)
#define SR_SHIFT8 (SR_RESET)
#define SR_SHIFT1 (SR_RESET | 0x0E << USICNT0)
#define SR_TICK (SR_RESET | 1 << USITC)

#define CR_RESET (1 << USIWM1 | 1 << USICS1 | 1 << USICLK)
#define CR_TICK (1 << USIWM1 | 1 << USICS1 | 1 << USICLK | 1 << USITC)

// Atmel stuff (AVR310)
#define TWI_FAST_MODE

#define SYS_CLK   5000.0  // [kHz]     4000 = 120khz, 6000 = 90khz

#ifdef TWI_FAST_MODE               // TWI FAST mode timing limits. SCL = 100-400kHz
#define T2_TWI    ((SYS_CLK *1300) /1000000) +1 // >1,3us
#define T4_TWI    ((SYS_CLK * 600) /1000000) +1 // >0,6us
#else                              // TWI STANDARD mode timing limits. SCL <= 100kHz
#define T2_TWI    ((SYS_CLK *4700) /1000000) +1 // >4,7us
#define T4_TWI    ((SYS_CLK *4000) /1000000) +1 // >4,0us
#endif

#define SET_USI_TO_SEND_ACK()                                                                          \
{                                                                                                      \
    USIDR = 0;                                                          /* Prepare ACK */              \
    SDA_OUTPUT();                                                       /* Set SDA as output */        \
    USISR = (1 << USIOIF | 1 << USIPF | 1 << USIDC | 0x0E << USICNT0);  /* Clear flags | count 1 bit*/ \
}

#define SET_USI_TO_READ_ACK()                                                                          \
{                                                                                                      \
    USIDR = 0;                                                          /* Prepare ACK */              \
    SDA_INPUT();                                                        /* Set SDA as intput */        \
    USISR = (1 << USIOIF | 1 << USIPF | 1 << USIDC | 0x0E << USICNT0);  /* Clear flags | count 1 bit*/ \
}

#define SET_USI_TO_TWI_START_CONDITION_MODE()                                                          \
{                                                                                                      \
    USICR = (1 << USISIE | 1 << USIWM1 | 1 << USICS1);      /* Start Interrupt | TWI Mode | Ext Clk */ \
    USISR = SR_RESET;														        /* Clear flags */  \
}

#define SET_USI_TO_SEND_DATA()                                                                         \
{                                                                                                      \
    SDA_OUTPUT();                                           /* Set SDA as output */                    \
    USISR = (1 << USIOIF | 1 << USIPF | 1 << USIDC);        /* Clear flags, except start condition */  \
}

#define SET_USI_TO_READ_DATA()                                                                         \
{                                                                                                      \
    SDA_INPUT();                                            /* Set SDA as input */                     \
    USISR = (1 << USIOIF | 1 << USIPF | 1 << USIDC);        /* Clear flags, except start condition */  \
}

#define WAIT_TRANSFER()                                                                                \
{                                                                                                      \
    do                                                                                                 \
    {                                                                                                  \
        _delay_us(T2_TWI/4);                                                                           \
        USICR = CR_TICK;                        /* Rising SCL Edge */                                  \
        while (!IS_SCL_HIGH());                 /* wait scl high */                                    \
        _delay_us(T4_TWI/4);                                                                           \
        USICR = CR_TICK;                        /* Falling SCL Edge */                                 \
    } while ((USISR & (1 << USIOIF)) == 0);     /* Operation completes */                              \
    _delay_us(T2_TWI/4);                                                                               \
}

#define WAIT_TRANSFER_1BIT()                                                                           \
{                                                                                                      \
    USISR = SR_SHIFT1;                          /* Count one bit (2 edges) */                          \
    WAIT_TRANSFER();                                                                                   \
}
    
#define WAIT_TRANSFER_8BIT()                                                                           \
{                                                                                                      \
    USISR = SR_SHIFT8;                          /* Count one bit (2 edges) */                          \
    WAIT_TRANSFER();                                                                                   \
}
																									   
static inline TWRESULT slave_wait()																				   
{																									   
	while ((USISR & (1 << USIOIF | 1 << USIPF)) == 0)
		;
	return (USISR & (1 << USIPF)) 
		? TWST_SL_STOP 
		: TWST_OK;												   
}

static uint8_t _address;
static void start_condition();
static void usi_init_master();
static uint8_t wait_slave_start();

TWRESULT usi_init_slave(uint8_t slave_addr)
{
    _address = slave_addr;
    // Okay these need to be used :D
    SCL_HIGH();
    SDA_HIGH();
	SDA_OUTPUT();
    SCL_OUTPUT();
    
    SDA_INPUT();

    USISR = SR_RESET;
    do
    {
	    if (!wait_slave_start())						// we received a start or stop -> dont check the address
			continue;									// (wouldn't be a valid address)
    }													// Repeat as long we are not addressed by the master
#ifdef I2C_ALL_ADDR 
	while (USIDR != 0 && (USIDR >> 1) != _address);   
#else
	while ((USIDR >> 1) != _address);   
#endif

    TWRESULT result = (USIDR & 0x01)					// Check if we are transmitting or receiving slave
		? TWST_SL_TRANSMITTING
		: TWST_SL_RECEIVING;

    SET_USI_TO_SEND_ACK();                              // Send the acknowledge
	return result;
}

static void usi_init_master()
{
    SCL_HIGH();
    SDA_HIGH();
        
    SCL_OUTPUT();
    SDA_OUTPUT();

    USIDR = 0xFF;
    USICR = CR_RESET;                                   // TWI Mode | Software Strobe Clock
    USISR = SR_RESET;                                   // Clear flags
}

TWRESULT usi_start_master(uint8_t slave_addr, uint8_t transmitting)
{
    usi_init_master(); 
    start_condition();
        
    if ((USISR & (1 << USISIF)) == 0)                   // Checks start condition :)
        return TWST_START_FAILED;

    // Data is the slave address and the bit if we are sending or receiving
    uint8_t data = (slave_addr << 1) | (transmitting ? 0x00 : 0x01);
    return usi_write_master(data)
		? TWST_OK										// Didn't receive ack :(
		: TWST_MASTER_NACK;
}

uint8_t usi_write_master(uint8_t data)
{
    // WRITE DATA
    SCL_LOW();
    USIDR = data;                                       // Set data to send
    SDA_HIGH();
    WAIT_TRANSFER_8BIT();                               // Wait transfer of 8 bits (data)
    SDA_OUTPUT();                                       // Claim SDA
    USIDR = 0xFF;
    SDA_OUTPUT();

    // READ ACK
    SDA_INPUT();                                        // Receive input from slave
    WAIT_TRANSFER_1BIT();                               // Wait transfer of a bit (ACK bit)
    SDA_OUTPUT();                                       // Claim SDA
    uint8_t tmp = USIDR;
    USIDR = 0xFF;
    SDA_OUTPUT();

    return !(tmp & 0x01);
}

uint8_t usi_write_slave(uint8_t data)
{
    slave_wait();                                       // Wait for bus to be ready
    USIDR = data;                                       // Sets data

    SET_USI_TO_SEND_DATA();                             // Issues USI to send data
    
    slave_wait();                                       // Wait again..
    SET_USI_TO_READ_ACK();                              // Issues USI to read the ack

    slave_wait();
    return !(USIDR & 0x01);								// Returns ACK received
}

uint8_t usi_read_master(uint8_t nack)
{
    SDA_INPUT();
    WAIT_TRANSFER_8BIT();
    uint8_t data = USIDR;
        
    SDA_OUTPUT();
    USIDR = nack ? 0xFF : 0x00;
    WAIT_TRANSFER_1BIT();

    return data;
}

TWRESULT usi_read_slave(uint8_t* data)
{	
    volatile TWRESULT res = slave_wait();               // Wait to be come ready
	if (res != TWST_OK)
		return res;

    SET_USI_TO_READ_DATA();                             // Sets usi to start reading
    res = slave_wait();                                 // Wait for read to complete
	if (res != TWST_OK)
		return res;

    *data = USIDR;										// Temporary hold data
    SET_USI_TO_SEND_ACK();                              // Send ACK

    return TWST_OK;                                     // Returns the data
}

TWRESULT usi_stop()
{
    SDA_LOW();                                          // Pulls data low
    _delay_us(T2_TWI/4);
    SCL_INPUT();
    while (!IS_SCL_HIGH()) ;                            // Wait for clock to be released
    _delay_us(T4_TWI/4);
    SDA_INPUT();                                        // Releases data
    while (!IS_SDA_HIGH()) ;

    return (USISR & (1 << USIPF))
		? TWST_OK
		: TWST_STOP_FAILED;                             // Returns true if stop succeeded
}

static void start_condition()
{
    // We set the clock manually high and wait for it. This due the fact if there was already some I2C traffic
    // we wouldn't be able to claim it. And so not corrup the bus

    SCL_HIGH();                                         // Set clock high
    while (!IS_SCL_HIGH()) ;                            // Wait for it
    #ifdef TWI_FAST_MODE
    _delay_us( T4_TWI/4 );                              // Delay for T4TWI if TWI_FAST_MODE
    #else
    _delay_us( T2_TWI/4 );                              // Delay for T2TWI if TWI_STANDARD_MODE
    #endif                                              // Wait rising

    SDA_LOW();                                          // Data low
    _delay_us(T4_TWI/4);                                // Wait falling
    SCL_LOW();                                          // Clock low
}

static uint8_t wait_slave_start()
{
	SET_USI_TO_TWI_START_CONDITION_MODE();              // Reset to receive start condition
	while ((USISR & (1 << USISIF)) == 0) ;              // Wait for USI to detect the start condition
	SDA_INPUT();                                        // Data as input -> to receive slave address

	while (IS_SCL_HIGH() && !(IS_SDA_HIGH())) ;         // Be sure start condition was completed

	USICR = (IS_SDA_HIGH())                             // Check if we received start of stop condition
		? 1 << USISIE | 1 << USIWM1 | 1 << USIWM0 | 1 << USICS1                 // stop condition
		: 1 << USISIE | 1 << USIOIE | 1 << USIWM1 | 1 << USIWM0 | 1 << USICS1;  // start condition

	USISR = SR_RESET;                                   // Reset flags
	while ((USISR & (1 << USIOIF)) == 0)				// wait for transfer 8 bits (address)
	{
		if (USISR & (1 << USISIF | 1 << USIPF))			// we don't expect a start or stop here!?
			return 0;
	}

	return 1;
}