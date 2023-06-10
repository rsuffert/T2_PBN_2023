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

#define GAME_DURATION_SEC              50

uint16_t global_interruption_count = 0; // used for telling when the game should end
uint16_t editable_interr_count = 1;     // used for controlling when the mole should change its whole
uint16_t points_counter = 0;            // counts how many times the player has hit the mole

/**
 * Initiates/resets Timer 1.
 */
void timer1_init();

/**
 * Renders the table, drawing, for each of the five wholes, the character specified in the array passed as parameter.
 * @WHOLE: the whole where the mole should be drawn
 * @NWHOLES: the number of wholes in the game
 */
void render_table(const uint8_t WHOLE, const uint8_t NWHOLES);

/**
 * Renders the timer and the points.
 */
void render_timer_points();

/*
 * Interruption routine for Timer 1.
 */
ISR(TIMER1_COMPA_vect)
{
    global_interruption_count++;
    editable_interr_count++;
    if (global_interruption_count == GAME_DURATION_SEC+1) // game over
    {
        while (1)
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
    while ((PIND & (1 << PD0)) == 0); // wait for button release

    srand(seed); // set the seed
    sei();       // enable interruptions

    // GAME LOGIC
    const uint16_t APPEAR_DURATION_SEC = 5;
    const uint8_t WHOLES_BUTTONS = 5;
    uint8_t rand_whole = rand() % WHOLES_BUTTONS;
    while (1)
    {
        render_timer_points();

        if (editable_interr_count == APPEAR_DURATION_SEC) // if APPEAR_DURATION_SEC have passed, change the whole where the mole should be
        {
            rand_whole = rand() % WHOLES_BUTTONS;
            editable_interr_count = 1;
        }
        render_table(rand_whole, WHOLES_BUTTONS);

        // check which buttons are pressed
        bool pressed[WHOLES_BUTTONS];
        for (int i=0; i<WHOLES_BUTTONS; i++) // initialize all buttons as not pressed
            pressed[i] = false;
        for (int i=0; i<WHOLES_BUTTONS; i++) // get first pressed button
        {
            if ((PIND & (1 << i)) == 0)
            {
                while((PIND & (1 << i)) == 0) // wait button release, but keep rendering the table
                {
                    render_timer_points();
                    render_table(rand_whole, WHOLES_BUTTONS);
                }
                pressed[i] = true;
                break;
            }
        }

        if (pressed[rand_whole]) // hit (increment points_counter and get new random whole)
        {
            points_counter++;
            rand_whole = rand() % WHOLES_BUTTONS;
        }
        else
        {
            // check if the user guessed a whole (if so, that position in the array will be set to "true")
            for (int i=0; i<WHOLES_BUTTONS; i++)
            {
                if (pressed[i]) // if so, get new random whole
                {
                    rand_whole = rand() % WHOLES_BUTTONS;
                    break;
                }
            }
        }
    }
}

void render_timer_points()
{
    nokia_lcd_clear();
    nokia_lcd_set_cursor(0, 40);
    nokia_lcd_write_string("T/Pts: ", 1);
    char t_pts[10];
    sprintf(t_pts, "%d/%d", GAME_DURATION_SEC-global_interruption_count+1, points_counter);
    nokia_lcd_set_cursor(39, 40);
    nokia_lcd_write_string(t_pts, 1);
}

void render_table(const uint8_t WHOLE, const uint8_t NWHOLES)
{
    char whole_symbols[NWHOLES];
    for(int i=0; i<NWHOLES; i++)
        whole_symbols[i] = 'O';
    
    whole_symbols[WHOLE] = 'A';

    nokia_lcd_set_cursor(10, 5);
    nokia_lcd_write_char(whole_symbols[0], 1);
    nokia_lcd_set_cursor(65, 5);
    nokia_lcd_write_char(whole_symbols[1], 1);
    nokia_lcd_set_cursor(37, 15);
    nokia_lcd_write_char(whole_symbols[2], 1);
    nokia_lcd_set_cursor(10, 25);
    nokia_lcd_write_char(whole_symbols[3], 1);
    nokia_lcd_set_cursor(65, 25);
    nokia_lcd_write_char(whole_symbols[4], 1);
    nokia_lcd_render();
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