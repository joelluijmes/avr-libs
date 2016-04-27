TARGET = libtwi_mega.a
CC = avr-gcc
OBJCOPY = avr-objcopy

CFLAGS = -Wall -O2 -mmcu=atmega328p -std=c99
OBJCOPYFLAGS = -O ihex

AR = avr-ar
ARFLAGS = rcs

PRE = -DF_CPU=16000000UL

.PHONY: default all clean

default: $(TARGET)
all: default

SOURCES = twi.c avr\twi_mega.c
OBJECTS = $(SOURCES:.c=.o)
HEADERS = $(SOURCES:.c=.h)

%.o: %.c $(HEADERS)
	$(CC) $(PRE) $(CFLAGS) -c $< -o $@

.PRECIOUS: $(TARGET) $(OBJECTS)

$(TARGET): $(OBJECTS)
	$(AR) $(ARFLAGS) $@ $(OBJECTS)

clean:
	-rm -f $(OBJECTS)
	-rm -f $(TARGET)

run:
	$(TARGET)