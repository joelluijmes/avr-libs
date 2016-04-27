## avr-twi

A twi library for avr devices. Currently supported are the ATmega328P and 
ATtiny85 however the same library should run on both of their families
(ATmegax28 and ATtinyx5.)

### ATmega328P
The library uses the hardware twi in the chip it self this makes it very
easy. The library is actually just made so I have the same interface in
a different project.

### ATtiny85
The chip doesn't support twi in hardware but it has USI, which provides 
very basic hardware for common serial protocol features.

### Library
The library is fully blocking, which means that it waits on every operation
for completion so it doesn't use any interrupts.

In the subfolders there are the actual libraries, twi.h is just an interface
which calls these functions depending on what chip the compiler is targetting.

There is a keep alive feature which allows to keep the bus to the master 
between transfers.

The example projects are out of date and may or not work.
