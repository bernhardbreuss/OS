#include "irq.h"
#include "../timer/gptimer.h"

void* irq_saved_context_pointer = &irq_saved_context;

void irq_save_context();
void irq_load_context();

interrupt void irq_handler() {
	asm(" SUB R14, R14, #4");	/* LR = R14; R14-4 is the return address of the IRQ (see ARM System Developers Guid.pdf page 337 */
	asm(" SUB R13, R13, #4");	/* reserve space for R14 on the stack */
	asm(" STR R14, [R13]");		/* store R14 on the stack */
	irq_save_context();
	asm(" ADD R13, R13, #4");	/* release space for R14 on the stack */

	/* TODO: determine which interrupt and call specific handler */
	gptimer_handler();

	irq_load_context();
}
