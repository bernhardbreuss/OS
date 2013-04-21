/*
 * gptimer.c
 *
 *  Created on: Apr 12, 2013
 *      Author: Bernhard
 */

#include "gptimer.h"
#include "../../generic/timer/gptimer.h"

/* TODO: remove */
typedef volatile unsigned int* address;

void clear_pending_interrupts(timer_t timer);

timer_t gptimer_init_ms() {
	timer_t timer = (timer_t) GPTIMER2;

	unsigned int* tclr = (timer + GPTIMER_BASE_OFFSET_TCLR);

	/* disable all interrupt events */
	*(timer + GPTIMER_BASE_OFFSET_TIER) &= 0x0;

	/* stop the timer if running already */
	*(tclr) &= ~0x1;

	clear_pending_interrupts(timer);

	/* set timer to 1 ms */
	*(timer + GPTIMER_BASE_OFFSET_TPIR) = 232000; /* OMAP35x.pdf Page 2625 */
	*((int*) timer + GPTIMER_BASE_OFFSET_TNIR) = -768000;
	*(timer + GPTIMER_BASE_OFFSET_TLDR) = 0xFFFFFFE0;
	*(timer + GPTIMER_BASE_OFFSET_TCRR) = 0xFFFFFFE0;

	/* set timer overflow match */
	*(timer + GPTIMER_BASE_OFFSET_TOCR) = 0x0;
	*(timer + GPTIMER_BASE_OFFSET_TOWR) = 0x1;

	/* TODO: set clock source */

	*(tclr) |= (GPTIMER_TCLR_COMPARE_ENABLE | GPTIMER_TCLR_AUTORELOAD_MODE
			| GPTIMER_TCLR_TRIGER_OVERFLOW_MATCH);

	/* disable prescale */
	*(tclr) &= ~GPTIMER_TCLR_PRESCALE;

	return timer;
}

void gptimer_start(timer_t timer) {
	*(timer + GPTIMER_BASE_OFFSET_TIER) |= GPTIMER_TIER_OVERFLOW_ENABLE;
	*(timer + GPTIMER_BASE_OFFSET_TCLR) |= GPTIMER_TCLR_START_STOP_CONTROL;
}

void clear_pending_interrupts(timer_t timer) {
	*(timer + GPTIMER_BASE_OFFSET_TISR) = (GPTIMER_TISR_CAPTURE_FLAG
			| GPTIMER_TISR_MATCH_FLAG | GPTIMER_TISR_OVERFLOW_FLAG);
}
