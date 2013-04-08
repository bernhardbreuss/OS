/*
 * omap3530_timer.h
 *
 *  Created on: 06.04.2013
 *      Author: edi
 */

#ifndef OMAP3530_TIMER_H_
#define OMAP3530_TIMER_H_

/* OMAPP35x.pdf - page 2632 -> base addresses of GP Timer module instances */
/* each timer 4k bytes */
#define GPTIMER1_BASE 		(address)0x48318000
#define GPTIMER2_BASE 		(address)0x49032000
#define GPTIMER3_BASE 		(address)0x49034000
#define GPTIMER4_BASE 		(address)0x49036000
#define GPTIMER5_BASE 		(address)0x49038000
#define GPTIMER6_BASE 		(address)0x4903A000
#define GPTIMER7_BASE 		(address)0x4903C000
#define GPTIMER8_BASE 		(address)0x4903E000
#define GPTIMER9_BASE 		(address)0x49040000
#define GPTIMER10_BASE 		(address)0x48086000
#define GPTIMER11_BASE 		(address)0x48088000

/* GP Timer Register offsets */
#define GPTIMER_BASE_OFFSET_TIDR			0x00
#define GPTIMER_BASE_OFFSET_TIOCP_CFG		0x10
#define GPTIMER_BASE_OFFSET_TISTAT 			0x14
#define GPTIMER_BASE_OFFSET_TISR 			0x18

/* An interrupt can be issued on overflow if the overflow interrupt enable bit is set in the timer interrupt
enable register (GPTi.TIER[1] OVF_IT_ENA bit set to 1) */
#define GPTIMER_BASE_OFFSET_TIER 			0x1C	/* timer interrupt enable register */
#define GPTIMER_BASE_OFFSET_TWER 			0x20
#define GPTIMER_BASE_OFFSET_TCLR 			0x24	/* Timer control register */
#define GPTIMER_BASE_OFFSET_TCRR 			0x28 	/* Timer counter register */

/* When the autoreload mode is enabled (the GPTi.TCLR[1] AR bit set to 1), the GPTi.TCRR register is
reloaded with the timer load register (GPTi.TLDR) value after a counting overflow occurs. */
#define GPTIMER_BASE_OFFSET_TLDR			0x2C	/* Timer load register */
#define GPTIMER_BASE_OFFSET_TTGR 			0x30
#define GPTIMER_BASE_OFFSET_TWPS 			0x34

/* This register holds the value to be compared with the counter value. */
#define GPTIMER_BASE_OFFSET_TMAR 			0x38	/* Time match register */
#define GPTIMER_BASE_OFFSET_TCAR1 			0x3C
#define GPTIMER_BASE_OFFSET_TSICR 			0x40
#define GPTIMER_BASE_OFFSET_TCAR2 			0x44
#define GPTIMER_BASE_OFFSET_TPIR 			0x48 	/* Timer positive increment register */
#define GPTIMER_BASE_OFFSET_TNIR 			0x4C	/* Timer negative increment register */

/* This register is used for 1 ms tick generation. The TCVR register defines whether next value loaded
in TCRR will be the sub-period value or the over-period value. */
#define GPTIMER_BASE_OFFSET_TCVR 			0x50	/* Timer counter value register */
#define GPTIMER_BASE_OFFSET_TOCR 			0x54	/* Timer overflow counter register */
#define GPTIMER_BASE_OFFSET_TOWR 			0x58	/* Timer overflow wrapping register */


#define GPTIMER_TISR_CAPTURE_FLAG_OFFSET		2
#define GPTIMER_TISR_OVERFLOW_FLAG_OFFSET		1
#define GPTIMER_TISR_MATCH_FLAG_OFFSET			0


#define GPTIMER_TCLR_COMPARE_ENABLE_OFFSET 		6
#define GPTIMER_TCLR_AUTORELOAD_MODE_OFFSET		1
#define GPTIMER_TCLR_START_STOP_CONTROL_OFFSET	0

#endif /* OMAP3530_TIMER_H_ */

/*
 * Quotes from OMAP35x.pf
 *
 * 1-ms Tick Generation (Only GPTIMER1, GPTIMER2, and GPTIMER10)
 *
 * Because the timer input clock is 32,768 Hz, the interrupt period is not exactly 1 ms.
 *
 * The values of the GPTi.TPIR and GPTi.TNIR registers are calculated using the following formula:
 * - Positive increment value = ( (INTEGER[ Fclk * Ttick] + 1) * 1e6) - (Fclk * Ttick * 1e6)
 * - Negative increment value = (INTEGER[ Fclk * Ttick] * 1e6) - (Fclk * Ttick * 1e6)
 *
 * NOTE:
 * Fclk clock frequency (kHz)
 * Ttick tick period (ms)
 *
 * For 1-ms tick with a 32,768-Hz clock:
 * - GPTi.TPIR[31:0] POSITIVE_INC_VALUE = 232000
 * - GPTi.TNIR[31:0] NEGATIVE_INC_VALUE = -768000
 * - GPTi.TLDR[31:0] LOAD_VALUE = 0xFFFFFFE0
 *
 */
