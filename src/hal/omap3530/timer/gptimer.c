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
	*(timer + GPTIMER_BASE_OFFSET_TOWR) = 0x18D; /* TODO: Muss noch genauer getestet werden, glaube es sind momentan 5 microseconds */

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

/* TODO: remove after setting up callback handlers for timer */
#include "../../../service/logger/logger.h"
#include "../../../kernel/process_manager.h"
extern timer_t main_timer;
extern unsigned int currentProcessId;
extern ProcessManager_t processManager;
unsigned volatile int irq_number = 0;

void gptimer_handler(void) {
	/* TODO: call handler function */
	if (++irq_number % 3000 == 0) { /* ~3 seconds */
		currentProcessId = (currentProcessId % 2);

		logger_error("Interrupt number: %u", irq_number);
		logger_debug("Current process = %u", currentProcessId);

		process_manager_change_process(&processManager, currentProcessId++);
	}

	/* clear all pending interrupts */
	clear_pending_interrupts(main_timer);
	*((unsigned int*)0x48200048) = 0x1; /* INTCPS_CONTROL s. 1083 */
}

void clear_pending_interrupts(timer_t timer) {
	*(timer + GPTIMER_BASE_OFFSET_TISR) = (GPTIMER_TISR_CAPTURE_FLAG
			| GPTIMER_TISR_MATCH_FLAG | GPTIMER_TISR_OVERFLOW_FLAG);
}

