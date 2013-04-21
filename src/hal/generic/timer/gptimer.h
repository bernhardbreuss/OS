/*
 * timer.h
 *
 *  Created on: Apr 12, 2013
 *      Author: Bernhard
 */

#ifndef TIMER_H_
#define TIMER_H_

typedef unsigned int* timer_t;

timer_t gptimer_init_ms();
void gptimer_start(timer_t timer);

#endif /* TIMER_H_ */
