/*
 * main.c
 * For playing with the MMU
 */
#include <inttypes.h>
#include "service/logger/logger.h"
#include "hal/omap3530_timer.h"

typedef volatile unsigned int* address;

/*asm("\t .bss masterTableAddress, 4");
asm("\t .global masterTableAddress");
asm("_masterTableAddress .field _masterTableAddress, 32");

extern volatile unsigned int* _masterTableAddress;

void enableMMU(void) {
	asm("\t MRC p15, #0, r1, c1, c0, #0");
	// MRC Move to ARM register from coprocessor (CP15 register to ARM register)
	// p15	ist der coprocessor
	// #0	is a coprocessor-specific opcode
	// r1	is the ARM destination register
	// c1	first  coprocessor source register (c1 ist das Control Register des coprocessor; sihe S. 153 cortexA8.pdf)
	// c0	second coprocessor source register
	// #0	is an optional coprocessor-specific opcode

	asm("\t ORR r1, r1, #0x1");
	// ORR Logical OR
	// r1	is the destination register
	// r1	is the register holding the first operand
	// #0x1	is a flexible second operand (hier einfach hex 1)

	asm("\t MCR p15, #0, r1, c1, c0, #0");
	// MCR Move to coprocessor from ARM registers
	// p15	ist der coprocessor
	// #0	is a coprocessor-specific opcode
	// r1	is the ARM destination register
	// c1	first coprocessor source register
	// c0	second coprocessor source register
	// #0	is an optional coprocessor-specific opcode

	// Kurz
}

void setTranslationTableBase(void) {
	asm("\t LDR r0, _masterTableAddress");
    asm("\t LDR r0, [r0]\n");
	asm("\t MCR p15, #0, r0, c2, c0, #0");
}*/

#pragma SWI_ALIAS(make_swi, 47);
unsigned int make_swi(unsigned int foo, char* bar);

//#pragma TASK(swi_handler);
#pragma INTERRUPT(swi_handler,SWI);
extern interrupt unsigned int swi_handler(unsigned int foo, char* bar) {
	logger_debug("got %u: %s", foo, bar);
	logger_logmode();
	//asm(" LDMFD   R13!, {R0-R12, PC}^");
	return 4711;
}

#pragma INTERRUPT(irq_handler, IRQ)
void irq_handler() {
	logger_error("timer interrupt occurred ....");
}

void main(void) {

	logger_init();
	logger_debug("System init...");
	logger_warn("System warning...");
	logger_error("System error...");
	logger_logmode();

    asm("\t CPS 0x10"); // enter User Mode
	logger_logmode();

    int c = make_swi(4712, "foobar");
    logger_debug("swi returned: %u", c);

	logger_logmode();

    logger_debug("Done with SW interrupts, up next timer interrupts ....");

	address tier = (address)((int)GPTIMER1_BASE + GPTIMER_BASE_OFFSET_TIER);
	address tclr = (address)((int)GPTIMER1_BASE + GPTIMER_BASE_OFFSET_TCLR);
	address tcrr = (address)((int)GPTIMER1_BASE + GPTIMER_BASE_OFFSET_TCRR);
	address tldr = (address)((int)GPTIMER1_BASE + GPTIMER_BASE_OFFSET_TLDR);
	address tmar = (address)((int)GPTIMER1_BASE + GPTIMER_BASE_OFFSET_TMAR);

	/* disable all interrupt events */
	*(tier) &= 0x0;
	/* stop the timer if running already */
	*(tclr) &= ~0x1;
	/* reset the counter register and load register*/
	*(tcrr) &= 0x0;
	*(tldr) &= 0x0;

	/* set timer match register */
	*(tmar) = 32000;

	/* enable compare and auto reload modes */
	*(tclr) |= (1 < GPTIMER_TCLR_COMPARE_ENABLE_OFFSET);
	*(tclr) |= (1 < GPTIMER_TCLR_AUTORELOAD_MODE_OFFSET);

	/* enable the match interrupt event */
	*(tier) |= (1 < GPTIMER_TISR_MATCH_FLAG_OFFSET);

	/* start the timer */
	*(tclr) |= (1 < GPTIMER_TCLR_START_STOP_CONTROL_OFFSET);

    while (1) ;
}
