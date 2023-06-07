/* Nokia 5110 LCD AVR Library
 *
 * Copyright (C) 2015 Sergey Denisov.
 * Written by Sergey Denisov aka LittleBuster (DenisovS21@gmail.com)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public Licence
 * as published by the Free Software Foundation; either version 3
 * of the Licence, or (at your option) any later version.
 *
 * Original library written by SkewPL, http://skew.tk
 * 
 * Functions for drawing shapes (lines, rectangles, circles) added by Ricardo Süffert.
 */

#ifndef __NOKIA_5110_H__
#define __NOKIA_5110_H__

#include <avr/pgmspace.h>
#include <stdint.h>

/*
 * LCD's port
 */
#define PORT_LCD PORTB
#define DDR_LCD DDRB

/*
 * LCD's pins
 */
#define LCD_SCE PB1
#define LCD_RST PB2
#define LCD_DC PB3
#define LCD_DIN PB4
#define LCD_CLK PB5

#define LCD_CONTRAST 0x40

/*
 * Must be called once before any other function, initializes display
 */
void nokia_lcd_init(void);

/*
 * Clear screen
 */
void nokia_lcd_clear(void);

/**
 * Power of display
 * @lcd: lcd nokia struct
 * @on: 1 - on; 0 - off;
 */
void nokia_lcd_power(uint8_t on);

/**
 * Set single pixel
 * @x: horizontal pozition
 * @y: vertical position
 * @value: show/hide pixel
 */
void nokia_lcd_set_pixel(uint8_t x, uint8_t y, uint8_t value);

/**
 * Draw single char with 1-6 scale
 * @code: char code
 * @scale: size of char
 */
void nokia_lcd_write_char(char code, uint8_t scale);

/**
 * Draw string. Example: writeString("abc",3);
 * @str: sending string
 * @scale: size of text
 */
void nokia_lcd_write_string(const char *str, uint8_t scale);

/**
 * Set cursor position
 * @x: horizontal position
 * @y: vertical position
 */
void nokia_lcd_set_cursor(uint8_t x, uint8_t y);

/*
 * Render screen to display
 */
void nokia_lcd_render(void);

/*
 * Define custom char (ASCII 0-31)
 */
void nokia_lcd_custom(char code, uint8_t* glyph);

/**
 * Draws a line.
 * @x1: x-coordinate of the first point of the line
 * @y1: y-coordinate of the first point of the line
 * @x2: x-coordinate of the second point of the line
 * @y2: y-coordinate of the second point of the line
 */
void nokia_lcd_draw_line(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2);

/**
 * Draws a circle.
 * @centerX: x-coordinate of the center point of the circle
 * @centerY: y-coordinate of the center point of the circle
 * @radius: the radius of the circle
 */
void nokia_lcd_draw_circle(uint8_t centerX, uint8_t centerY, uint8_t radius);

/**
 * Draws a rectangle.
 * @topLefX: x-coordinate of the top-left point of the rectangle
 * @topLefY: y-coordinate of the top-left point of the rectangle
 * @bottomRightX: x-coordinate of the bottom-right point of the rectangle
 * @bottomRightY: y-coordinate of the bottom-right point of the rectangle
 */
void nokia_lcd_draw_rectangle(uint8_t topLeftX, uint8_t topLeftY, uint8_t bottomRightX, uint8_t bottomRightY);

#endif
