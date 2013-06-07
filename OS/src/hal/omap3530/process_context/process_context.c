/*
 * process_context.c
 *
 *  Created on: May 11, 2013
 *      Author: Bernhard
 */

#include "../../generic/process_context/process_context.h"
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

static void _process_context_init(Process_t* process) {
	process->saved_context[14] = (void*)0x01000000; /* R13 = Stack begins at 0x10000000 */

	unsigned int cpsr = _get_CPSR();
	cpsr &= 0xFFFFFF90; /* delete mode and interrupt disable bits*/
	cpsr |= 0x00000010; /* set mode to user */
	process->saved_context[16] = (void*)cpsr;
}

void process_context_init_byfunc(Process_t* process, process_func func) {
	process->saved_context[0] = func;
	_process_context_init(process);
}

void process_context_init_bybinary(Process_t* process, binary_t* binary) {
	process->saved_context[0] = binary->entry_point;
	_process_context_init(process);
}

uint8_t process_context_init(Process_t* process) {
	uint8_t* stack = malloc(PROCESS_STACK_SIZE);
	if (stack != NULL) {
		memset(stack, 0, PROCESS_STACK_SIZE);
		memset(process->saved_context, 0, sizeof(process->saved_context));

		/*
		 * context structure:
		 * PC
		 * R0-R14
		 * CPSR
		 */
		process->saved_context[0] = process->func;
		process->saved_context[14] = stack + PROCESS_STACK_SIZE; /* R13 */
		process->saved_context[16] = (void*)_get_CPSR(); /* initialize CPSR with current CPSR */

		return 1;
	} else {
		return 0;
	}
}
