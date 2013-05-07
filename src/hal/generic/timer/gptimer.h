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
 * on failure schedule_timer is set internally to NULL
 */
void gptimer_get_schedule_timer(gptimer_t* schedule_timer);


/**
 * Call it to get a PWM timer associated with this id.
 * Other HAL interfaces between HAL and OS will need the initialized timer afterwards
 * on failure timer is set internally to NULL
 *
 * @param id 		- a generic id (call getter again with same id and you will get the same timer object)
 * 					  the id starts by 1 and increments to n (platform specific)
 */
void gptimer_get_pwm_timer(int id, gptimer_t* pwm_timer);

/**
 * Initialize a timer.
 *
 * @param timer 	- the timer object to initialize
 * @param config 	- configuration object you can use to set up specific settings during initialization process
 */
void gptimer_init(gptimer_t* const timer, const gptimer_config_t* const config);

void gptimer_start(gptimer_t* const timer);

void gptimer_clear_pending_interrupts(gptimer_t* const timer);

/**
 * Returns the configuration object with the default values which can be used for timer initialization.
 * Retrieve the configuration and modify it as desired.
 *
 * @return  the configuration object with default values set
 */
gptimer_config_t gptimer_get_default_timer_init_config(void);

#endif /* TIMER_H_ */
