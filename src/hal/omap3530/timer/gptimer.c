/*
 * gptimer.c
 *
 *  Created on: Apr 12, 2013
 *      Author: Bernhard
 */

#include <stdlib.h>
#include "gptimer.h"
#include "prcm.h"
#include "../../../service/logger/logger.h"

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
					timer->intcps_mapping_id = GPTIMER1_INTCPS_MAPPING_ID; break;
		case 2 : 	timer_base_address = GPTIMER2;
					timer->intcps_mapping_id = GPTIMER2_INTCPS_MAPPING_ID;
					*(cm_clksel_per) &= ~BIT0;	//set to 32 kHz clock
					logger_log_register("\r\n\r\nSet gp timer2 clock to 32 khz... %s\n", cm_clksel_per);
					break;
		case 3 : 	timer_base_address = GPTIMER3;
					timer->intcps_mapping_id = GPTIMER3_INTCPS_MAPPING_ID; break;
		case 4 : 	timer_base_address = GPTIMER4;
					timer->intcps_mapping_id = GPTIMER4_INTCPS_MAPPING_ID; break;
		case 5 : 	timer_base_address = GPTIMER5;
					timer->intcps_mapping_id = GPTIMER5_INTCPS_MAPPING_ID; break;
		case 6 : 	timer_base_address = GPTIMER6;
					timer->intcps_mapping_id = GPTIMER6_INTCPS_MAPPING_ID; break;
		case 7 : 	timer_base_address = GPTIMER7;
					timer->intcps_mapping_id = GPTIMER7_INTCPS_MAPPING_ID; break;
		case 8 : 	timer_base_address = GPTIMER8;
					timer->intcps_mapping_id = GPTIMER8_INTCPS_MAPPING_ID; break;
		case 9 : 	timer_base_address = GPTIMER9;
					timer->intcps_mapping_id = GPTIMER9_INTCPS_MAPPING_ID; break;
		case 10: 	timer_base_address = GPTIMER10;
					timer->intcps_mapping_id = GPTIMER10_INTCPS_MAPPING_ID; break;
		case 11: 	timer_base_address = GPTIMER11;
					timer->intcps_mapping_id = GPTIMER11_INTCPS_MAPPING_ID; break;
		default: {
			logger_error("Timer number %u in gptimer_get not supported!", timer_nr);
			timer = NULL;
		}
	}

	if(timer) {
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

gptimer_config_t gptimer_get_default_timer_init_config(void) {
	gptimer_config_t default_conf = { -1 };
	return default_conf;
}


void gptimer_init(gptimer_t* const timer, const gptimer_config_t* const config) {

	int pos_inc = ((((int)(32.768))+1)*1000000)-(32.768*1000000);
	int neg_inc = (((int)(32.768))*1000000)-(32.768*1000000);

	/* disable all interrupt events */
	*(timer->TIER) &= 0x0;

	/* stop the timer if running already */
	*(timer->TCLR) &= ~0x1;

	gptimer_clear_pending_interrupts(timer);

	/* set timer to 1 ms - OMAP35x.pdf Page 2625 */
	*(timer->TPIR) = pos_inc;
	*(timer->TNIR) = neg_inc;
	*(timer->TLDR) = 0xFFFFFFE0;
	*(timer->TCRR) = 0xFFFFFFE0;

	/* set timer overflow match */
	*(timer->TOCR) = 0x0;
	*(timer->TOWR) = 0x18D;

	/* TODO: Muss noch mit clock source getestet werde, jetzt 1ms */

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

/* TODO: remove after setting up callback handlers for timer */
#include "../../../kernel/process_manager.h"
extern gptimer_t main_timer;
int currentProcessId = -1;
unsigned volatile int irq_number = 0;
extern Process_t* prozessSlots[MAX_PROCESSES];

void gptimer_handler(void) {
	/* TODO: call handler function */
	if (++irq_number % 8 == 0) { /* ~3 seconds */
		int i = currentProcessId + 1;
		Process_t* p;
		while (i != currentProcessId) {
			if (i >= MAX_PROCESSES) {
				i = 0;
			}
			p = prozessSlots[i];
			if (p != NULL && p->state == PROCESS_READY) {
				currentProcessId = p->pid;
				logger_error("Interrupt number: %u", irq_number);
				break;
			}

			i++;
		}

		process_manager_change_process(currentProcessId);
	}

	/* clear all pending interrupts */
	gptimer_clear_pending_interrupts(&main_timer);
	*((unsigned int*)0x48200048) = 0x1; /* INTCPS_CONTROL s. 1083 */
}
void gptimer_clear_pending_interrupts(gptimer_t* const timer) {
	*(timer->TISR) = (GPTIMER_TISR_CAPTURE_FLAG
			| GPTIMER_TISR_MATCH_FLAG | GPTIMER_TISR_OVERFLOW_FLAG);
}
