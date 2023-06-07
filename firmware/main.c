#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include "nokia5110.h"

/**
 * Values for one interruption per second.
 */
#define FREQ_CPU                       16000000UL
#define PRESCALER                      1024
#define PERIOD                         1

#define GAME_DURATION_SEC              5

bool game_over = false;
uint16_t interruption_count = 0;
uint16_t points_counter = 0;

/**
 * Initiates/resets Timer 1.
 */
void timer1_init();

/*
 * Interruption routine for Timer 1.
 */
ISR(TIMER1_COMPA_vect)
{
    interruption_count++;
    if (interruption_count == GAME_DURATION_SEC+1)
    {
        while (((PIND & (1 << PD0)) != 0))  // while button has not been presssed
        {
            nokia_lcd_clear();
            nokia_lcd_set_cursor(20, 0);
            nokia_lcd_write_string("GAME", 2);
            nokia_lcd_set_cursor(20, 20);
            nokia_lcd_write_string("OVER", 2);
            nokia_lcd_set_cursor(0, 40);
            nokia_lcd_write_string("Points: ", 1);
            char points[17];
            sprintf(points, "%d", points_counter);
            nokia_lcd_set_cursor(45, 40);
            nokia_lcd_write_string(points, 1);
            nokia_lcd_render();
        }
        game_over = true;
    }
}

int main()
{
    cli();                                                                        // disable interruptions
    DDRC |= (1 << PC5) | (1 << PC4) | (1 << PC3);                                 // PC5 to PC3 -> output
    DDRD &= ~(1 << PD4) & ~(1 << PD3) & ~(1 << PD2) & ~(1 << PD1) & ~(1 << PD0);  // PD4 to PD0 -> input
    PORTD |= (1 << PD4) | (1 << PD3) | (1 << PD2) | (1 << PD1) | (1 << PD0);      // enabling internal pull-up

    // setting up the LCD
    nokia_lcd_init();
    nokia_lcd_clear();

    timer1_init();

    unsigned int seed = 0;                                           // "random" seed generated by iterations
    while (((PIND & (1 << PD0)) != 0))                               // while button has not been presssed, display message and "generate" seed
    {
        nokia_lcd_set_cursor(7, 15);
        nokia_lcd_write_string("Whac-A-Mole!", 1);
        nokia_lcd_set_cursor(6, 25);
        nokia_lcd_write_string("(W to start)", 1);
        nokia_lcd_render();
        seed++;
    }

    sei();       // enable interruptions
    srand(seed); // set the seed

    while (!game_over)
    {
        nokia_lcd_clear();
        nokia_lcd_set_cursor(0, 40);
        nokia_lcd_write_string("Time: ", 1);
        char time[17];
        sprintf(time, "%d", GAME_DURATION_SEC-interruption_count+1);
        nokia_lcd_set_cursor(35, 40);
        nokia_lcd_write_string(time, 1);
        nokia_lcd_render();

        //TODO: implement game logic
    }

    nokia_lcd_power(0); // turn off display
}

void timer1_init()
{
    TCNT1 = 0;                                                         // reset timer counter
    TCCR1B |= (1 << WGM12);                                            // set CTC mode
    TCCR1B |= (1 << CS12) | (1 << CS10);                               // set prescaler to 1024
    long unsigned int compare_value = (FREQ_CPU / PRESCALER) * PERIOD; // calculate compare value for PERIOD seconds
    OCR1A = compare_value;                                             // set compare value
    TIMSK1 |= (1 << OCIE1A);                                           // enable compare interruption A
}