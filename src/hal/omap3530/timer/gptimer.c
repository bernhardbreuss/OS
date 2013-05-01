/*
 * gptimer.c
 *
 *  Created on: Apr 12, 2013
 *      Author: Bernhard
 */

#include "gptimer.h"
#include "../../../service/logger/logger.h"
#include "../../generic/timer/gptimer.h"

gptimer_t gptimer_get(int timer_nr) {
	unsigned int* timer_base_address = 0x0;

	switch(timer_nr) {
		case 1 : timer_base_address = (unsigned int*) GPTIMER1; break;
		case 2 : timer_base_address = (unsigned int*) GPTIMER2; break;
		case 3 : timer_base_address = (unsigned int*) GPTIMER3; break;
		case 4 : timer_base_address = (unsigned int*) GPTIMER4; break;
		case 5 : timer_base_address = (unsigned int*) GPTIMER5; break;
		case 6 : timer_base_address = (unsigned int*) GPTIMER6; break;
		case 7 : timer_base_address = (unsigned int*) GPTIMER7; break;
		case 8 : timer_base_address = (unsigned int*) GPTIMER8; break;
		case 9 : timer_base_address = (unsigned int*) GPTIMER9; break;
		case 10: timer_base_address = (unsigned int*) GPTIMER10; break;
		case 11: timer_base_address = (unsigned int*) GPTIMER11; break;
		default: logger_error("Timer number %u in gptimer_get not supported!", timer_nr);
	}

	gptimer_t timer;
	timer.TIDR = timer_base_address + GPTIMER_BASE_OFFSET_TIDR;
	timer.TIOCP_CFG = timer_base_address + GPTIMER_BASE_OFFSET_TIOCP_CFG;
	timer.TISTAT = timer_base_address + GPTIMER_BASE_OFFSET_TISTAT;
	timer.TISR = timer_base_address + GPTIMER_BASE_OFFSET_TISR;
	timer.TIER = timer_base_address + GPTIMER_BASE_OFFSET_TIER;
	timer.TWER = timer_base_address + GPTIMER_BASE_OFFSET_TWER;
	timer.TCLR = timer_base_address + GPTIMER_BASE_OFFSET_TCLR;
	timer.TCRR = timer_base_address + GPTIMER_BASE_OFFSET_TCRR;
	timer.TLDR = timer_base_address + GPTIMER_BASE_OFFSET_TLDR;
	timer.TTGR = timer_base_address + GPTIMER_BASE_OFFSET_TTGR;
	timer.TWPS = timer_base_address + GPTIMER_BASE_OFFSET_TWPS;
	timer.TMAR = timer_base_address + GPTIMER_BASE_OFFSET_TMAR;
	timer.TCAR1 = timer_base_address + GPTIMER_BASE_OFFSET_TCAR1;
	timer.TSICR = timer_base_address + GPTIMER_BASE_OFFSET_TSICR;
	timer.TCAR2 = timer_base_address + GPTIMER_BASE_OFFSET_TCAR2;
	timer.TPIR = timer_base_address + GPTIMER_BASE_OFFSET_TPIR;
	timer.TNIR = timer_base_address + GPTIMER_BASE_OFFSET_TNIR;
	timer.TCVR = timer_base_address + GPTIMER_BASE_OFFSET_TCVR;
	timer.TOCR = timer_base_address + GPTIMER_BASE_OFFSET_TOCR;
	timer.TOWR = timer_base_address + GPTIMER_BASE_OFFSET_TOWR;
	return timer;
}
void gptimer_init_ms(gptimer_t* const timer, const gptimer_config_t* const config) {

	/* disable all interrupt events */
	*(timer->TIER) &= 0x0;

	/* stop the timer if running already */
	*(timer->TCLR) &= ~0x1;

	clear_pending_interrupts(timer);

	//TODO: use "ticks_in_millis" within configuration object to set up specific tick interval with formula on page 2625 in OMAP35x.pdf

	/* set timer to 1 ms */
	*(timer->TPIR) = 232000; /* OMAP35x.pdf Page 2625 */

	//TODO: remove the warning
	*(timer->TNIR) = -768000;
	*(timer->TLDR) = 0xFFFFFFE0;
	*(timer->TCRR) = 0xFFFFFFE0;

	/* set timer overflow match */
	*(timer->TOCR) = 0x0;
	*(timer->TOWR) = 0x1; /* TODO: Muss noch genauer getestet werden, glaube es sind momentan 5 microseconds */

	/* TODO: set clock source */

	*(timer->TCLR) |= (GPTIMER_TCLR_COMPARE_ENABLE | GPTIMER_TCLR_AUTORELOAD_MODE
			| GPTIMER_TCLR_TRIGER_OVERFLOW_MATCH);

	/* disable prescaler */
	*(timer->TCLR) &= ~GPTIMER_TCLR_PRESCALE;
}

void gptimer_start(gptimer_t* const timer) {
	*(timer->TIER) |= GPTIMER_TIER_OVERFLOW_ENABLE;
	*(timer->TCLR) |= GPTIMER_TCLR_START_STOP_CONTROL;
}

void clear_pending_interrupts(gptimer_t* const timer) {
	*(timer->TISR) = (GPTIMER_TISR_CAPTURE_FLAG
			| GPTIMER_TISR_MATCH_FLAG | GPTIMER_TISR_OVERFLOW_FLAG);
}
