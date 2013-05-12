/*
 * pwm.h
 *
 *  Created on: 10.05.2013
 *      Author: lumannnn
 */

#include "../../generic/pwm/pwm.h"
#include "pwm.h"
#include "../../../service/logger/logger.h"

// implicit functions
static unsigned int pwm_calculate_load(unsigned int start, unsigned int load, float duty_cycle);
static void pwm_wait_until_reset_occured(unsigned int* TISAT_x);

void pwm_setup() {
	// change pins into mode 2
	*(CONTROL_PADCONF_UART2_CTS) = (2 << 16) | 2;
	*(CONTROL_PADCONF_UART2_TX) &= ~7;
	*(CONTROL_PADCONF_UART2_TX) |= 2;

	*(PM_PWSTCTRL_CORE) |= (BIT0 | BIT1);
	*(PM_PWSTCTRL_PER) |= (BIT0 | BIT1);

	*(TIOCP_CFG_9) |= (	BIT1 | 	// no idle -> use BIT4 for 'smart idle'
						BIT2 |	// enable wake up (important for 'smart-idle' only).
						BIT3	// software reset
					);
	pwm_wait_until_reset_occured(TISTAT_9);

	*(TIOCP_CFG_10) |= (BIT1 | 	// no idle -> use BIT4 for 'smart idle'
						BIT2 |	// enable wake up (important for 'smart-idle' only).
						BIT3	// software reset
						);
	pwm_wait_until_reset_occured(TISTAT_10);

	*(TIOCP_CFG_11) |= (BIT1 | 	// no idle -> use BIT4 for 'smart idle'
						BIT2 |	// enable wake up (important for 'smart-idle' only).
						BIT3	// software reset
						);
	pwm_wait_until_reset_occured(TISTAT_11);
}

void pwm_get_timer(int id, gptimer_t* timer) {
	switch(id) {
		case 1: id = 9; break;
		case 2: id = 10; break;
		case 3: id = 11; break;
		default: id = -1;
	}
	gptimer_get(id, timer);
}


void pwm_clear(gptimer_t* const timer) {
	*(timer->TCLR) = 0;
	gptimer_clear_pending_interrupts(timer);
}

void pwm_config(gptimer_t* const timer, pwm_config_t* const config) {
	//	/*	OMAP35x.pdf, page 2627
	//	 * [...]
	//	 * In Figure 16-12, the internal overflow pulse is set each time
	//	 * (0xFFFF FFFF - GPTi.TLDR[31:0] LOAD_VALUE + 1) the value is reached,
	//	 * and the internal match pulse is set when the counter reaches the GPTi.TMAR register value.
	//	 * According to the value of the GPTi.TCLR[12] PT and GPTi.TCLR[11:10] TRG bits,
	//	 * the timer provides pulse or PWM event on the output pin (timer PWM).
	//	 * [...]
	//	 */
	unsigned int counter_start = 0xFFFFFFFF - (config->period_time*100/config->pwm_frequenz);
	unsigned int load = counter_start;
	unsigned int match = pwm_calculate_load(counter_start, 0xFFFFFFFF, config->duty_cycle);

	// ensure overflow protection according to datasheet
	if (0xFFFFFFFF - match <= 2) {
		match = 0xFFFFFFFF - 2;
	}

	// ensure overflow protection
	if (0xFFFFFFFF - counter_start <= 2) {
		counter_start = 0xFFFFFFFF - 2;
	}

	*(timer->TCLR) = 0;
	*(timer->TCRR) = counter_start;
	*(timer->TLDR) = load;
	*(timer->TMAR) = match;
	*(timer->TCLR) |= (
		(BIT0)  | 						// ST -- enable counter
		(BIT1)  | 						// AR -- autoreload on overflow
		(BIT6)  | 						// CE -- compare enabled
		(BIT7)  | 						// SCPWM -- invert pulse
		(PWM_TRG_OVERFLOW_AND_MATCH) | 	// TRG -- overflow and match trigger
		(BIT12)   						// PT -- toggle PWM mode
	);
}

void pwm_start(gptimer_t* const timer) {
	gptimer_start(timer);
}

void pwm_stop(gptimer_t* const timer) {
	gptimer_stop(timer);
}

static unsigned int pwm_calculate_load(unsigned int start, unsigned int load, float duty_cycle) {
	unsigned int period = load - start;
	return load - (period * duty_cycle / 100.0);
}

/**
 * according to OMAP35x.pdf we should wait until the software reset was done before continue to use the GPs.
 * OMAP35x.pdf, page 2619
 * -> page 2637 and page 2639
 */
static void pwm_wait_until_reset_occured(unsigned int* TISAT_x) {
	while (*(TISAT_x) != (*(TISAT_x) | BIT0))
		;
}
