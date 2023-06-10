#include <avr/interrupt.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include "nokia5110.h"

/**
 * Values for one interruption per second.
 */
#define FREQ_CPU                       16000000UL
#define PRESCALER                      1024
#define PERIOD                         1

/**
 * Durations
 */
#define GAME_DURATION_SEC              60   // game duration, in seconds
#define APPEAR_DURATION_SEC            1    // how long the mole stays out of its whole

uint16_t global_interruption_count = 0;     // used for controlling when the game should end
uint16_t appear_duration_count = 0;         // used for controlling when the mole should change its whole
uint16_t points_counter = 0;                // counts how many times the player has hit the mole

/**
 * Initiates/resets Timer 1.
 */
void timer1_init();

/**
 * Renders the table, drawing, for each of the five wholes, the character specified in the array passed as parameter.
 * @param WHOLE the whole where the mole should be drawn
 * @param NWHOLES the number of wholes in the game
 */
void render_table(const uint8_t WHOLE, const uint8_t NWHOLES);

/**
 * Renders the timer and the points.
 */
void render_timer_points();

/**
 * Randomly selects a new whole for the mole to appear, making sure the new whole is different from the previous.
 * @param CURRENT_WHOLE the current whole where the mole is
 * @param NWHOLES the number of wholes in the game
 * @return the new whole randomly picked, which necessarily is different from the 'CURRENT_WHOLE' parameter
 */
uint8_t new_rand_whole(const uint8_t CURRENT_WHOLE, const uint8_t NWHOLES);

/**
 * Interruption routine for Timer 1.
 */
ISR(TIMER1_COMPA_vect)
{
    global_interruption_count++;
    appear_duration_count++;
    if (global_interruption_count >= GAME_DURATION_SEC+1) // game over
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
        while (1);
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

    // INITIAL SCREEN:
    nokia_lcd_drawrect(0, 0, 83, 47);
    nokia_lcd_set_cursor(7, 10);
    nokia_lcd_write_string("WHAC-A-MOLE!", 1);
    nokia_lcd_set_cursor(6, 20);
    nokia_lcd_write_string("(W to start)", 1);
    nokia_lcd_set_cursor(5, 35);
    char time[14];
    sprintf(time, "You have %ds", GAME_DURATION_SEC);
    nokia_lcd_write_string(time, 1);
    nokia_lcd_render();
    unsigned int seed = 0;                                           // seed generated by iterations
    while (((PIND & (1 << PD0)) != 0))                               // while button has not been presssed, display the initial screen and "generate" seed
        seed++;
    while ((PIND & (1 << PD0)) == 0);                                // once the button has been pressed, wait for button release

    srand(seed); // set the seed
    sei();       // enable interruptions

    // GAME LOGIC
    const uint8_t WHOLES_BUTTONS = 5;             // how many wholes and buttons there are in the game
    uint8_t rand_whole = rand() % WHOLES_BUTTONS; // stores the current whole where the mole is
    while (1)
    {
        render_timer_points();

        if (appear_duration_count >= APPEAR_DURATION_SEC) // if APPEAR_DURATION_SEC have passed, change the whole where the mole should be
            rand_whole = new_rand_whole(rand_whole, WHOLES_BUTTONS);
        render_table(rand_whole, WHOLES_BUTTONS);

        // check which buttons are pressed
        bool pressed[WHOLES_BUTTONS];
        for (int i=0; i<WHOLES_BUTTONS; i++) // initialize all buttons as not pressed
            pressed[i] = false;
        for (int i=0; i<WHOLES_BUTTONS; i++) // iterate over all buttons and get the first pressed button
        {
            if ((PIND & (1 << i)) == 0)       // if button is pressed
            {
                while((PIND & (1 << i)) == 0) // wait button release, but keep rendering the points and table
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
            rand_whole = new_rand_whole(rand_whole, WHOLES_BUTTONS);
        }
        else                     // the user either did not try to hit the mole or missed it (guessed)
        {
            // iterate over all buttons to check if the user guessed a whole
            for (int i=0; i<WHOLES_BUTTONS; i++)
            {
                if (pressed[i]) // the user took a guess and missed, get new random whole
                {
                    rand_whole = new_rand_whole(rand_whole, WHOLES_BUTTONS);
                    break;
                }
            }
        }
    }
}

uint8_t new_rand_whole(const uint8_t CURRENT_WHOLE, const uint8_t NWHOLES)
{
    uint8_t randn;
    do 
    {
        randn = rand() % NWHOLES;
    } while (randn == CURRENT_WHOLE);
    appear_duration_count = 0;
    
    return randn;
}

void render_timer_points()
{
    nokia_lcd_clear();
    nokia_lcd_set_cursor(0, 41);
    nokia_lcd_write_string("T/Pts: ", 1);
    char t_pts[10];
    sprintf(t_pts, "%d/%d", GAME_DURATION_SEC-global_interruption_count+1, points_counter);
    nokia_lcd_set_cursor(39, 41);
    nokia_lcd_write_string(t_pts, 1);
}

void render_table(const uint8_t WHOLE, const uint8_t NWHOLES)
{
    char whole_symbols[NWHOLES];
    for(int i=0; i<NWHOLES; i++)
        whole_symbols[i] = 'O';
    
    whole_symbols[WHOLE] = '&';

    nokia_lcd_set_cursor(37, 0);
    nokia_lcd_write_char(whole_symbols[0], 1);
    nokia_lcd_set_cursor(0, 13);
    nokia_lcd_write_char(whole_symbols[1], 1);
    nokia_lcd_set_cursor(37, 13);
    nokia_lcd_write_char(whole_symbols[2], 1);
    nokia_lcd_set_cursor(72, 13);
    nokia_lcd_write_char(whole_symbols[3], 1);
    nokia_lcd_set_cursor(37, 28);
    nokia_lcd_write_char(whole_symbols[4], 1);
    nokia_lcd_drawline(0, 38, 84, 38); // divider
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