CC = avr-gcc
OBJCOPY = avr-objcopy
DUDE = avrdude

CCFLAGS = -Wall -O2 -mmcu=atmega328p -DF_CPU=16000000UL -DBAUD=57600 -std=c99
OBJCOPYFLAGS = -O ihex
DUDEFLAGS = -p atmega328p -b 57600 -c arduino -PCOM3 -v

SOURCES = main.c ..\twi_mega.c ..\..\..\uart\uart.c
COMPILED = firmware.hex
LINKED = firmware.o
OBJECTS = $(SOURCES:.c=.o)
LIBRARIES = ../twi

all: $(COMPILED)
	avr-size firmware.o

flash: $(COMPILED)
	$(DUDE) $(DUDEFLAGS) -Uflash:w:$<:i

disassemble: $(LINKED)
	$(OBJDUMP) $(OBJDUMPFLAGS) $(LINKED)

clean:
	$(RM) $(COMPILED) $(LINKED) $(OBJECTS)

$(COMPILED): $(LINKED)
	$(OBJCOPY) $(OBJCOPYFLAGS) $< $@

$(LINKED): $(SOURCES)
	$(CC) $(CCFLAGS) $(SOURCES) -o $(LINKED)
