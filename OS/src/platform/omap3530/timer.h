/*
 * gptimer.h
 *
 *  Created on: 22.05.2013
 *      Author: edi
 */

#ifndef OMAP3530_TIMER_PLATFORM_H_
#define OMAP3530_TIMER_PLATFORM_H_

#define SCHEDULER_TIME_SLICE_DURATION 3000

typedef struct _gptimer_t {
	int interrupt_line_id;
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
	int* volatile TNIR;
	unsigned int* volatile TCVR;
	unsigned int* volatile TOCR;
	unsigned int* volatile TOWR;
} gptimer_t;

#endif /* OMAP3530_TIMER_PLATFORM_H_ */
