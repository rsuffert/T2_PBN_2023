#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "nokia5110.h"

uint8_t glyph[] = { 0x10, 0x38, 0x7C, 0x10, 0x20 }; // "fuinha"

int main() 
{
    nokia_lcd_init();
    nokia_lcd_custom(1, glyph);
    nokia_lcd_clear();
    nokia_lcd_write_string("\001", 2);
    nokia_lcd_render();
    while(1);
}