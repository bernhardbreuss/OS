/*
 * pwm.h
 *
 *  Created on: 10.05.2013
 *      Author: lumannnn
 */

#ifndef PWM_H_
#define PWM_H_

#include "../timer/gptimer.h"

/**
 * Call it to get a PWM timer associated with this id.
 * Other HAL interfaces between HAL and OS will need the initialized timer afterwards
 * on failure timer is set internally to NULL
 *
 * @param id 		- a generic id (call getter again with same id and you will get the same timer object)
 * 					  the id starts by 1 and increments to n (platform specific)
 */
void pwm_get_timer(int id, gptimer_t* pwm_timer);

#define PWM_TRG_OVERFLOW					BIT10
#define PWM_TRG_OVERFLOW_AND_MATCH			BIT11

#define PWM_SCPWM_DEFAULT_HIGH				BIT7

#define PWM_PT_TOGGLE						BIT12

typedef struct _pwm_config_t {
	gptimer_config_t* timer_config;
	unsigned char duty_cycle;			// 0-100. Represents high part of the PWD signal in percent.
	unsigned int period_time;
	unsigned int pwm_frequenz;
} pwm_config_t;

void pwm_setup();

void pwm_clear(gptimer_t* const timer);

void pwm_config(gptimer_t* const timer, pwm_config_t* const config);

void pwm_start(gptimer_t* const timer);

void pwm_stop(gptimer_t* const timer);

#endif /* PWM_H_ */
