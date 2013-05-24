#include <stdlib.h>
#include "irq.h"
#include "../mpu_subsystem/intcps.h"
#include "../timer/gptimer.h"
#include "../../../service/logger/logger.h"
#include "../../generic/process_context/process_context.h"

static void (*_irq_handles[INTCPS_IRQ_MAX_COUNT])(void);

interrupt void irq_handler() {
	asm(" SUB R14, R14, #4");	/* LR = R14; R14-4 is the return address of the IRQ (see ARM System Developers Guid.pdf page 337 */
	asm(" SUB R13, R13, #4");	/* reserve space for R14 on the stack */
	asm(" STR R14, [R13]");		/* store R14 on the stack */
	process_context_save();
	asm(" ADD R13, R13, #4");	/* release space for R14 on the stack */

	irq_handle_interrupt();

	process_context_load();
}

void irq_init_handles(void) {
	//initialization with NULL seemingly not necessary for _irq_handles ... all handles are NULL
}

void irq_add_handler(int interrupt_line_id, void(*irq_handle_func)(void)) {

	if(_irq_handles[interrupt_line_id] != NULL) {
		logger_error("IRQ module - already assigned handler to handler: %d", interrupt_line_id);
		return;
	}

	logger_debug("IRQ module - add handle %d", interrupt_line_id);
	intcps_active_interrupt_line(interrupt_line_id);
	_irq_handles[interrupt_line_id] = irq_handle_func;
}

void irq_handle_interrupt(void) {

	//INTCPS_SIR_IRQ contains within the first 6 bits [6-0] the current active interrupt number
	//mask the bits [31-7] to get this 6 bits only
	//see Omap3530x.pdf page 1082
	int currently_active_irq = *((unsigned int*) INTCPS_SIR_IRQ);
	currently_active_irq &= 0x3F;

	//logger_log_register("IRQ module SIR_FIQ: %s", ((unsigned int*) INTCPS_SIR_IRQ));

	if(_irq_handles[currently_active_irq] != NULL) {
		//logger_debug("IRQ module - call interrupt handle: %u", currently_active_irq);
		_irq_handles[currently_active_irq]();
	}
	intcps_enable_new_irq_generation();
}
