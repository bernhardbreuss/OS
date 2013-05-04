/*
 * timer.h
 *
 *  Created on: Apr 12, 2013
 *      Author: Bernhard
 */

#ifndef TIMER_H_
#define TIMER_H_

typedef struct _gptimer_t {
	unsigned int* volatile TIDR;
	unsigned int* volatile TIOCP_CFG;
	unsigned int* volatile TISTAT;
	unsigned int* volatile TISR;
	unsigned int* volatile TIER;
	unsigned int* volatile TWER;
	unsigned int* volatile TCLR;
	unsigned int* volatile TCRR;
	unsigned int* volatile TLDR;
	unsigned int* volatile TTGR;
	unsigned int* volatile TWPS;
	unsigned int* volatile TMAR;
	unsigned int* volatile TCAR1;
	unsigned int* volatile TSICR;
	unsigned int* volatile TCAR2;
	unsigned int* volatile TPIR;
	unsigned int* volatile TNIR;
	unsigned int* volatile TCVR;
	unsigned int* volatile TOCR;
	unsigned int* volatile TOWR;
} gptimer_t;

/**
 * Expandable configuration object
 */
typedef struct _gp_timer_config_t {
	int ticks_in_millis;
} gptimer_config_t;

/**
 * Get the timer to the corresponding timer_nr -> GPTIMERx
 *
 * @param timer_nr 	- 1 to 11 are valid values
 * @return 			- the timer
 */
gptimer_t gptimer_get(int timer_nr);

/**
 * @param timer 	- GPTIMER1, GPTIMER2 and GPTIMER10 are valid timers the function expects
 * @param config 	- configuration object you can use to control specific settings during initialization process
 */
void gptimer_init_ms(gptimer_t* const timer, const gptimer_config_t* const config);

void gptimer_start(gptimer_t* const timer);

void clear_pending_interrupts(gptimer_t* const timer);

#endif /* TIMER_H_ */
