# avr-ssd1306
Library for the ssd1306 for AVR however any twi enabled device will work. 
This library uses [avr-twi](https://github.com/joelluijmes/avr-twi).
Note this library is based on the one from Adafruit but this is plain old
C instead of C++.

#### ssd1306
The low level driving of the display. The memory buffer is directly in the
sram so for the 128x64 version it takes 1024 bytes of sram.

#### gfx
Some graphical functions which makes this library actually useful. For now
it programmed specifically for the ssd1306 (128x64) however it should be
fairly straight forward to make this work on other displays which uses
a memory buffer.

Note: for displaying a bitmap it uses a simple compression algorithm which
checks if the next byte is the same as the current, if so give the length
of that same occuring byte. Source and converter program: [gabotronics].
(http://www.gabotronics.com/tutorials/run-length-encoding-for-lcd.htm)

### Examples
The following are some examples using the gfx. Excuse me for the magic 
numbers these examples are directly taken from a project of my and shows
how easy it is to have some fancy images.
You must run `gfx_init()` before and `gfx_update()` after a gfx function.

#### Fancy Dotted Clock
```C
void display_time(int8_t x, int8_t y, int8_t len, int8_t hour, int8_t minute)
{
	for (int8_t i = 0; i < 12; ++i)
		gfx_angled_pixel(x, y, (360/12)*i, len, 1);
	
	double hour_angle = 0.5*(60 * hour + minute);
	double min_angle = 6.0*minute;
	
	
	gfx_angled_line(x, y, (hour_angle - 90.0), len *0.7, 1);
	gfx_angled_line(x, y, (min_angle - 90.0), len * 0.9, 1);
}
```

If you would like something less fancy replace the dots with a circle:
```C
void display_time(int8_t x, int8_t y, int8_t len, int8_t hour, int8_t minute)
{
  gfx_circle(x, y, len, 1);
	
	double hour_angle = 0.5*(60 * hour + minute);
	double min_angle = 6.0*minute;
	
	gfx_angled_line(x, y, (hour_angle - 90.0), len *0.7, 1);
	gfx_angled_line(x, y, (min_angle - 90.0), len * 0.9, 1);
}
```

#### Battery Indicator 
Designed for 3 battery indicators (0 -> 4 as input)
```C
void display_battery(int8_t life)
{
	gfx_rect(106, 3, 18, 9, 1);
	gfx_vline(124, 5, 5, 1);

	for (int8_t i = 0; i < life; ++i)
		gfx_fill_rect(108+i*5, 5, 4, 5, 1);
}
```

#### Text
```C
	gfx_char_options options = { .background = 0, .color = 1, .size = 1 };
	gfx_print(0, 0, "Fancieh", options);
```
