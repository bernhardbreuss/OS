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

// implicit functions
static unsigned int gptimer_pwm_calc_resolution(int pwm_frequency, int clock_frequency);
static void pwm_config_timer(gptimer_t* timer, unsigned int resolution, float duty_cycle);
static void gptimer_get(int timer_nr, gptimer_t* timer);
static void gptimer_clear_pending_interrupts(gptimer_t* const timer);

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

/* TODO: remove after setting up callback handlers for timer */
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

	test_clock();

	/* clear all pending interrupts */
	gptimer_clear_pending_interrupts(&main_timer);
	*((unsigned int*)0x48200048) = 0x1; /* INTCPS_CONTROL s. 1083 */
}

void gptimer_clear_pending_interrupts(gptimer_t* const timer) {
	*(timer->TISR) = (GPTIMER_TISR_CAPTURE_FLAG
			| GPTIMER_TISR_MATCH_FLAG | GPTIMER_TISR_OVERFLOW_FLAG);
}

void test_clock(void) {

	/* set mode to 4 (GPIO) see p. ~787 of omap35x.pdf */
	unsigned int* CONTROL_PADCONF_UART2_CTS = (unsigned int*)0x48002174; /* GPIO144 15:0 GPIO145 16:32 */
	unsigned int* CONTROL_PADCONF_UART2_TX = (unsigned int*)0x48002178; /* GPIO146 15:0 */
	*(CONTROL_PADCONF_UART2_CTS) = (4 << 16) | 4;
	*(CONTROL_PADCONF_UART2_TX) &= ~7;
	*(CONTROL_PADCONF_UART2_TX) |= 4;



	logger_debug("Test Clock:");
	unsigned int EXPANSION_BIT = (1<<3);

	/* turn off rgb led on dmx interface
	 *  GPIO 144, 146, 145 --> GPIO 5 */
	unsigned int* GPIO5_OE = (unsigned int*)0x49056034;
	unsigned int* GPIO5_DATAOUT = (unsigned int*)0x4905603C;
	unsigned int rgb = (1 << (144 % 32)) | (1 << (146 % 32)) | (1 << (145 % 32)); //bit maske
	*(GPIO5_OE) &= ~rgb; //write
	*(GPIO5_DATAOUT) &= ~rgb; //in / out
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

void gptimer_get_pwm_timer(int id, gptimer_t* timer) {
	switch(id) {
		case 1: id = 9; break;
		case 2: id = 10; break;
		case 3: id = 11; break;
		default: id = -1;
	}
	gptimer_get(id, timer);
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
