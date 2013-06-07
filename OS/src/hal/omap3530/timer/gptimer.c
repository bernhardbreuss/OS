/*
 * gptimer.c
 *
 *  Created on: Apr 12, 2013
 *      Author: Bernhard
 */

#include "gptimer.h"

/*
* @param timer_nr	- 	the timer number
* @param timer 		- 	timer object afterwards contains all hardware specific registers of the requested timer
* 						or timer is set to NULL on failure
*/
void gptimer_get(int timer_nr, gptimer_t* timer) {
	unsigned int* timer_base_address = 0x0;
	unsigned int* volatile cm_clksel_per = PRCM_CM_CLKSEL_PER;

	//set up timer specific stuff
	switch(timer_nr) {
		case 1 : 	timer_base_address = GPTIMER1;
					timer->interrupt_line_id = GPTIMER1_INTCPS_MAPPING_ID; break;
		case 2 : 	timer_base_address = GPTIMER2;
					timer->interrupt_line_id = GPTIMER2_INTCPS_MAPPING_ID; break;
		case 3 : 	timer_base_address = GPTIMER3;
					timer->interrupt_line_id = GPTIMER3_INTCPS_MAPPING_ID; break;
		case 4 : 	timer_base_address = GPTIMER4;
					timer->interrupt_line_id = GPTIMER4_INTCPS_MAPPING_ID; break;
		case 5 : 	timer_base_address = GPTIMER5;
					timer->interrupt_line_id = GPTIMER5_INTCPS_MAPPING_ID; break;
		case 6 : 	timer_base_address = GPTIMER6;
					timer->interrupt_line_id = GPTIMER6_INTCPS_MAPPING_ID; break;
		case 7 : 	timer_base_address = GPTIMER7;
					timer->interrupt_line_id = GPTIMER7_INTCPS_MAPPING_ID; break;
		case 8 : 	timer_base_address = GPTIMER8;
					timer->interrupt_line_id = GPTIMER8_INTCPS_MAPPING_ID; break;
		case 9 : 	timer_base_address = GPTIMER9;
					timer->interrupt_line_id = GPTIMER9_INTCPS_MAPPING_ID; break;
		case 10: 	timer_base_address = GPTIMER10;
					timer->interrupt_line_id = GPTIMER10_INTCPS_MAPPING_ID; break;
		case 11: 	timer_base_address = GPTIMER11;
					timer->interrupt_line_id = GPTIMER11_INTCPS_MAPPING_ID; break;
		default: {
			logger_error("Timer number %u in gptimer_get not supported!", timer_nr);
			gptimer_t gptimter_error = { -1, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL ,
					NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
			*(timer) = 	gptimter_error;
		}
	}

	if(timer->interrupt_line_id != -1) {
		//set up common stuff
		timer->TIDR = timer_base_address + GPTIMER_BASE_OFFSET_TIDR;
		timer->TIOCP_CFG = timer_base_address + GPTIMER_BASE_OFFSET_TIOCP_CFG;
		timer->TISTAT = timer_base_address + GPTIMER_BASE_OFFSET_TISTAT;
		timer->TISR = timer_base_address + GPTIMER_BASE_OFFSET_TISR;
		timer->TIER = timer_base_address + GPTIMER_BASE_OFFSET_TIER;
		timer->TWER = timer_base_address + GPTIMER_BASE_OFFSET_TWER;
		timer->TCLR = timer_base_address + GPTIMER_BASE_OFFSET_TCLR;
		timer->TCRR = timer_base_address + GPTIMER_BASE_OFFSET_TCRR;
		timer->TLDR = timer_base_address + GPTIMER_BASE_OFFSET_TLDR;
		timer->TTGR = timer_base_address + GPTIMER_BASE_OFFSET_TTGR;
		timer->TWPS = timer_base_address + GPTIMER_BASE_OFFSET_TWPS;
		timer->TMAR = timer_base_address + GPTIMER_BASE_OFFSET_TMAR;
		timer->TCAR1 = timer_base_address + GPTIMER_BASE_OFFSET_TCAR1;
		timer->TSICR = timer_base_address + GPTIMER_BASE_OFFSET_TSICR;
		timer->TCAR2 = timer_base_address + GPTIMER_BASE_OFFSET_TCAR2;
		timer->TPIR = timer_base_address + GPTIMER_BASE_OFFSET_TPIR;
		timer->TNIR = (int *) timer_base_address + GPTIMER_BASE_OFFSET_TNIR;
		timer->TCVR = timer_base_address + GPTIMER_BASE_OFFSET_TCVR;
		timer->TOCR = timer_base_address + GPTIMER_BASE_OFFSET_TOCR;
		timer->TOWR = timer_base_address + GPTIMER_BASE_OFFSET_TOWR;
	}
}

void gptimer_get_schedule_timer(gptimer_t* schedule_timer) {
	gptimer_get(2, schedule_timer);
}

void gptimer_schedule_timer_init(gptimer_t* const timer) {

	int schedule_time_in_millis = 10000;

	//set to 32 kHz clock
	*(PRCM_CM_CLKSEL_PER) &= ~BIT0;

	/* disable all interrupt events */
	*(timer->TIER) &= 0x0;

	/* stop the timer if running already */
	*(timer->TCLR) &= ~0x1;

	gptimer_clear_pending_interrupts(timer);

	/* set timer to 1 millisecond precision - OMAP35x.pdf Page 2625 */
	//Timer counter registers is reset with Timer load register on overflow
	//both are initially set to FFFF FFFF - 31
	//32,768 clock ticks 32,768 per millisecond (~32)
	//due to this is not exact, we use TPIR and TNIR as pointed out in OMAP3530x.pdf (page 2625)
	//to minimize the millisecond error
	*(timer->TLDR) = 0xFFFFFFE0;
	*(timer->TCRR) = 0xFFFFFFE0;
	*(timer->TPIR) = 232000;
	*(timer->TNIR) = -768000;

	/* set timer overflow match */
	*(timer->TOCR) = 0x0;
	//Timer overflow watch register defines the amount of overflows before the interrupt triggers
	//Timer overflow control keeps track of current overflow status
	*(timer->TOWR) = (schedule_time_in_millis -1 );

	*(timer->TCLR) |= (GPTIMER_TCLR_COMPARE_ENABLE | GPTIMER_TCLR_AUTORELOAD_MODE
			| GPTIMER_TCLR_TRIGER_OVERFLOW_MATCH);

	/* disable prescaler */
	*(timer->TCLR) &= ~GPTIMER_TCLR_PRESCALE;
}

void gptimer_start(gptimer_t* const timer) {
	*(timer->TIER) |= GPTIMER_TIER_OVERFLOW_ENABLE;
	*(timer->TCLR) |= GPTIMER_TCLR_START_STOP_CONTROL;
}

void gptimer_stop(gptimer_t* const timer) {
	*(timer->TIER) &= ~GPTIMER_TIER_OVERFLOW_ENABLE;
	*(timer->TCLR) &= ~GPTIMER_TCLR_START_STOP_CONTROL;
}

void gptimer_clear_pending_interrupts(gptimer_t* const timer) {
	*(timer->TISR) = (GPTIMER_TISR_CAPTURE_FLAG
			| GPTIMER_TISR_MATCH_FLAG | GPTIMER_TISR_OVERFLOW_FLAG);
}
