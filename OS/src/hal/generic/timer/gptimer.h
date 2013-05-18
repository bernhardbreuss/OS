/*
 * timer.h
 *
 * Interface methods to work with hardware specific timer interrupts.
 *
 * Hardware specific type mapping objects and registers defined in ../../omap3530/timer/gptimer.h
 *
 * If hardware changes, implementation of this interface methods have to change too.
 *
 *  Created on: Apr 12, 2013
 *      Author: Bernhard
 */

#ifndef TIMER_H_
#define TIMER_H_


#include "../../omap3530/timer/gptimer.h"

/**
 * Call it to get the schedule timer which is able to work as millisecond precision timer.
 * Other HAL interfaces between HAL and OS will need the initialized schedule_timer afterwards
 * on failure schedule_timer registers are set to NULL
 */
void gptimer_get_schedule_timer(gptimer_t* schedule_timer);

//TODO: should not be declared here
//move it to src/hal/oamp3530/timer/timer.h
//current dependency by pwm_get_timer.h
void gptimer_get(int timer_nr, gptimer_t* timer);

/**
 * Initializes the schedule timer got from gptimer_get_schedule_timer( ... )
 *
 * @param timer 	- the timer object to initialize
 * @param config 	- configuration object you can use to set up specific settings during initialization process
 */
void gptimer_schedule_timer_init(gptimer_t* const timer);

void gptimer_start(gptimer_t* const timer);

void gptimer_stop(gptimer_t* const timer);

#endif /* TIMER_H_ */
