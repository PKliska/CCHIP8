/**
 * \file
 * \brief Contains an implementation of CHIP8 timers. CHIP8 timers are unsigned
 *        bytes automatically decremented at the rate of 60Hz until they reach
 *        zero.
 */
#ifndef CHIP8_TIMER_H
#define CHIP8_TIMER_H
#include <time.h>


/**
 * \brief Unsigned byte automatically decremented at a rate of 60Hz.
 *
 * CHIP8 timers do not decrement if their value is zero. Value of a timer can be
 * queried using chip8_timer_get() and set using chip8_timer_set().
 */
typedef struct {
    /** Value of the timer at the moment it was last updated */
    unsigned char value;
    /** Last time the timer was updated*/
    clock_t last;
} chip8_timer;
/**
 * \brief Returns current value of the timer.
 * \param t pointer to the queried timer
 */
unsigned char chip8_timer_get(chip8_timer* t);
/**
 * \brief Sets timer value to x.
 * \param t pointer to the timer to be set
 * \param x new value of the timer
 */
void chip8_timer_set(chip8_timer* t, unsigned char x);
#endif
