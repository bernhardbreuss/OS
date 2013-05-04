/*
 * gptimer.c
 *
 *  Created on: Apr 12, 2013
 *      Author: Bernhard
 */

#include <stdlib.h>
#include "gptimer.h"
#include "../../../service/logger/logger.h"
#include "../../generic/timer/gptimer.h"	// we may want to remove this. "gptimer.h" now includes the generic gptimer.h

// implicit functions
static gptimer_t gptimer_generic_get(int timer_nr, unsigned int* timer_base_address);
static unsigned int gptimer_pwm_calc_resolution(int pwm_frequency, int clock_frequency);
static void pwm_config_timer(gptimer_t* timer, unsigned int resolution, float duty_cycle);

gptimer_t gptimer_get(int timer_nr) {
	return gptimer_generic_get(timer_nr, 0x0);
}

/**
 * Generic way to set the timer_base_address as offset.
 */
static gptimer_t gptimer_generic_get(int timer_nr, unsigned int* timer_base_address) {
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
	*(timer->TOWR) = 0x18D; /* TODO: Muss noch genauer getestet werden, glaube es sind momentan 5 microseconds */

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

/* TODO: remove after setting up callback handlers for timer */
#include "../../../service/logger/logger.h"
#include "../../../kernel/process_manager.h"
extern gptimer_t main_timer;
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
	clear_pending_interrupts(&main_timer);
	*((unsigned int*)0x48200048) = 0x1; /* INTCPS_CONTROL s. 1083 */
}

void clear_pending_interrupts(gptimer_t* const timer) {
	*(timer->TISR) = (GPTIMER_TISR_CAPTURE_FLAG
			| GPTIMER_TISR_MATCH_FLAG | GPTIMER_TISR_OVERFLOW_FLAG);
}

/* ************************* *
 * 			PWM
 * ************************* */

void gptimer_pwm_setup() {
	/* set mode to 4 (GPIO) see p. ~787 of omap35x.pdf */
	// change pins into mode 2
	unsigned int* CONTROL_PADCONF_UART2_CTS = (unsigned int*) 0x48002174; /* GPIO144 15:0 GPIO145 16:32 */
	unsigned int* CONTROL_PADCONF_UART2_TX = (unsigned int*) 0x48002178; /* GPIO146 15:0 */
	*(CONTROL_PADCONF_UART2_CTS) = (2 << 16) | 2;
	*(CONTROL_PADCONF_UART2_TX) &= ~7;
	*(CONTROL_PADCONF_UART2_TX) |= 2;

	unsigned int* PM_PWSTCTRL_CORE = (unsigned int*) 0x48306AE0;
	*(PM_PWSTCTRL_CORE) |= (BIT0 | BIT1);

	// page 567 - set system clock to to 13 MHz
	// TODO: i don't know if this works...
	unsigned int* PRM_CLKSEL = (unsigned int*) 0x48306D40;
	*(PRM_CLKSEL) |= BIT1;

	// page 402, i guess...
	// config clocks
	// GPT9_FCLK ... PRCM.CM_CLKSEL_PER [0:7] CLKSEL_GPT9
	// GPT10_FCLK ... PRCM.CM_CLKSEL_CORE [6] CLKSEL_GPT10
	// GPT11_FCLK ... PRCM.CM_CLKSEL_CORE [7] CLKSEL_GPT11
	unsigned int* CM_CLKSEL_CORE = (unsigned int*) 0x48004A40;
	unsigned int* CM_CLKSEL_PER = (unsigned int*) 0x48005040;
	*(CM_CLKSEL_CORE) |= (BIT6 | BIT7);
	*(CM_CLKSEL_PER) |= BIT7;
}

gptimer_t gptimer_pwm_get(int timer_nr) {
	if (timer_nr < PWM_GPTIMER8 || timer_nr > PWM_GPTIMER11) {
		logger_warn("gptimer: gptimer_pwm_get(): attempted to get an invalid timer... abort.");
	}

	return gptimer_get(timer_nr);
}

void gptimer_pwm_clear(gptimer_t* const timer) {
	*(timer->TCLR) = 0;
}

void gptimer_pwm_init(gptimer_t* const timer, gptimer_pwm_config_t* const config) {
	unsigned int resolution = gptimer_pwm_calc_resolution(13000000, 19200000);
	pwm_config_timer(timer, resolution, config->high_percentage / 100.0);
}

void gptimer_pwm_start(gptimer_t* const timer) {
	gptimer_start(timer);
}

static unsigned int gptimer_pwm_calc_resolution(int pwm_frequency, int clock_frequency) {
    float pwm_period = 1.0 / pwm_frequency;
    float clock_period = 1.0 / clock_frequency;
    return (unsigned int) (pwm_period / clock_period);
}

static void pwm_config_timer(gptimer_t* timer, unsigned int resolution, float duty_cycle) {
    //	/*	OMAP35x.pdf, page 2627
    //	 * [...]
    //	 * In Figure 16-12, the internal overflow pulse is set each time
    //	 * (0xFFFF FFFF - GPTi.TLDR[31:0] LOAD_VALUE + 1) the value is reached,
    //	 * and the internal match pulse is set when the counter reaches the GPTi.TMAR register value.
    //	 * According to the value of the GPTi.TCLR[12] PT and GPTi.TCLR[11:10] TRG bits,
    //	 * the timer provides pulse or PWM event on the output pin (timer PWM).
    //	 * [...]
    //	 */
    unsigned int counter_start = 0xffffffff - resolution;
    unsigned int dc = 0xffffffff - ((unsigned int) (resolution * duty_cycle));

    // Edge condition: the duty cycle is set within two units of the overflow
    // value.  Loading the register with this value shouldn't be done (TRM 16.2.4.6).
    if (0xffffffff - dc <= 2) {
        dc = 0xffffffff - 2;
    }

    // Edge condition: TMAR will be set to within two units of the overflow
    // value.  This means that the resolution is extremely low, which doesn't
    // really make sense, but whatever.
    if (0xffffffff - counter_start <= 2) {
        counter_start = 0xffffffff - 2;
    }

    *(timer->TCLR) = 0; // Turn off
    *(timer->TCRR) = counter_start;
    *(timer->TLDR) = counter_start;
    *(timer->TMAR) = dc;
    *(timer->TCLR) = (
        (BIT0)  | 						// ST -- enable counter
        (BIT1)  | 						// AR -- autoreload on overflow
        (BIT6)  | 						// CE -- compare enabled
        (BIT7)  | 						// SCPWM -- invert pulse
        (PWM_TRG_OVERFLOW_AND_MATCH) | 	// TRG -- overflow and match trigger
        (BIT12)   						// PT -- toggle PWM mode
    );
}
