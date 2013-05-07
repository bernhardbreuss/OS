/*
 * omap3530_timer.h
 *
 *  Created on: 06.04.2013
 *      Author: edi
 */

#ifndef OMAP3530_TIMER_H_
#define OMAP3530_TIMER_H_

#include "../../../bit.h"
#include "../mpu_subsystem/intcps.h"


/* OMAPP35x.pdf - page 2632 -> base addresses of GP Timer module instances */
/* each timer 4k bytes */
#define GPTIMER1 		((unsigned int*) 0x48318000)
#define GPTIMER2		((unsigned int*) 0x49032000)
#define GPTIMER3 		((unsigned int*) 0x49034000)
#define GPTIMER4 		((unsigned int*) 0x49036000)
#define GPTIMER5		((unsigned int*) 0x49038000)
#define GPTIMER6 		((unsigned int*) 0x4903A000)
#define GPTIMER7		((unsigned int*) 0x4903C000)
#define GPTIMER8		((unsigned int*) 0x4903E000)
#define GPTIMER9		((unsigned int*) 0x49040000)
#define GPTIMER10		((unsigned int*) 0x48086000)
#define GPTIMER11		((unsigned int*) 0x48088000)

/* GP Timer Register offsets */
#define GPTIMER_BASE_OFFSET_TIDR			(0x00 / 4)
#define GPTIMER_BASE_OFFSET_TIOCP_CFG		(0x10 / 4)
#define GPTIMER_BASE_OFFSET_TISTAT 			(0x14 / 4)
#define GPTIMER_BASE_OFFSET_TISR 			(0x18 / 4)

/* An interrupt can be issued on overflow if the overflow interrupt enable bit is set in the timer interrupt
enable register (GPTi.TIER[1] OVF_IT_ENA bit set to 1) */
#define GPTIMER_BASE_OFFSET_TIER 			(0x1C / 4)	/* timer interrupt enable register */
#define GPTIMER_BASE_OFFSET_TWER 			(0x20 / 4)
#define GPTIMER_BASE_OFFSET_TCLR 			(0x24 / 4)	/* Timer control register */
#define GPTIMER_BASE_OFFSET_TCRR 			(0x28 / 4) 	/* Timer counter register */

/* When the autoreload mode is enabled (the GPTi.TCLR[1] AR bit set to 1), the GPTi.TCRR register is
reloaded with the timer load register (GPTi.TLDR) value after a counting overflow occurs. */
#define GPTIMER_BASE_OFFSET_TLDR			(0x2C / 4)	/* Timer load register */
#define GPTIMER_BASE_OFFSET_TTGR 			(0x30 / 4)
#define GPTIMER_BASE_OFFSET_TWPS 			(0x34 / 4)

/* This register holds the value to be compared with the counter value. */
#define GPTIMER_BASE_OFFSET_TMAR 			(0x38 / 4)	/* Time match register */
#define GPTIMER_BASE_OFFSET_TCAR1 			(0x3C / 4)
#define GPTIMER_BASE_OFFSET_TSICR 			(0x40 / 4)
#define GPTIMER_BASE_OFFSET_TCAR2 			(0x44 / 4)
#define GPTIMER_BASE_OFFSET_TPIR 			(0x48 / 4) 	/* Timer positive increment register */
#define GPTIMER_BASE_OFFSET_TNIR 			(0x4C / 4)	/* Timer negative increment register */

/* This register is used for 1 ms tick generation. The TCVR register defines whether next value loaded
in TCRR will be the sub-period value or the over-period value. */
#define GPTIMER_BASE_OFFSET_TCVR 			(0x50 / 4)	/* Timer counter value register */
#define GPTIMER_BASE_OFFSET_TOCR 			(0x54 / 4)	/* Timer overflow counter register */
#define GPTIMER_BASE_OFFSET_TOWR 			(0x58 / 4)	/* Timer overflow wrapping register */


#define GPTIMER_TISR_CAPTURE_FLAG			BIT2
#define GPTIMER_TISR_OVERFLOW_FLAG			BIT1
#define GPTIMER_TISR_MATCH_FLAG				BIT0

#define GPTIMER_TIER_CAPTURE_ENABLE			BIT2
#define GPTIMER_TIER_OVERFLOW_ENABLE		BIT1
#define GPTIMER_TIER_MATCH_ENABLE			BIT0

#define GPTIMER_TCLR_COMPARE_ENABLE 		BIT6
#define GPTIMER_TCLR_AUTORELOAD_MODE		BIT1
#define GPTIMER_TCLR_START_STOP_CONTROL		BIT0
#define GPTIMER_TCLR_TRIGER_OVERFLOW		BIT10
#define GPTIMER_TCLR_TRIGER_OVERFLOW_MATCH	BIT11
#define GPTIMER_TCLR_PRESCALE				BIT5

typedef struct _gptimer_t {
	intcps_gptimer_mapping_t intcps_mapping_id;
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

/**
 * Expandable configuration object
 *
 * Default values:
 *
 * ticks_in_millis;				//-1 = default = not defined
 */
typedef struct _gp_timer_config_t {
	int ticks_in_millis;
} gptimer_config_t;


void gptimer_handler(void);

/* ************************* *
 * 			PWM
 * ************************* */

#define PWM_GPTIMER8	8
#define PWM_GPTIMER9	9
#define PWM_GPTIMER10	10
#define PWM_GPTIMER11	11

#define PWM_TRG_OVERFLOW					BIT10
#define PWM_TRG_OVERFLOW_AND_MATCH			(BIT10 | BIT11)

#define PWM_SCPWM_DEFAULT_HIGH				BIT7

#define PWM_PT_TOGGLE						BIT12

typedef struct _gptimer_pwm_config_t {
	gptimer_config_t* timer_config;
	unsigned int volatile TRG;			// set pin to toggle on an event
	unsigned int volatile PT;			// set pin to toggle on an event
	unsigned int volatile SCPWM;		// set default value of PWM output. Set or clear the PWM output signal only while the counter is stopped or trigger is off.
	unsigned char high_percentage;		// 0-100. Represents high part of the PWD signal in percent.
} gptimer_pwm_config_t;

void gptimer_pwm_setup();

gptimer_t gptimer_pwm_get(int pwm_timer_nr);

void gptimer_pwm_clear(gptimer_t* const timer);

void gptimer_pwm_init(gptimer_t* const timer, gptimer_pwm_config_t* const config);

void gptimer_pwm_start(gptimer_t* const timer);

#endif /* OMAP3530_TIMER_H_ */
