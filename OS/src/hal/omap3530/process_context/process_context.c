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
#include "../../../platform/platform.h"

static void _process_context_init(Process_t* process, uint8_t privileged) {

	unsigned int cpsr = _get_CPSR();
	cpsr &= 0xFFFFFF90; /* delete mode and interrupt disable bits*/
	if (privileged) {
		process->saved_context[14] = ((uint8_t*)malloc(MMU_SMALLEST_PAGE_SIZE) + MMU_SMALLEST_PAGE_SIZE); /* R13 = Stack begins 4kB out of the kernel system memory */
		cpsr |= 0x0000001F; /* set mode to system */
	} else {
		process->saved_context[14] = (void*)0x7FFFFFFC; /* R13 = Stack begins at 0x10000000 */
		cpsr |= 0x00000010; /* set mode to user */
	}
	process->saved_context[16] = (void*)cpsr;
}

void process_context_init_byfunc(Process_t* process, process_func_t func, uint8_t privileged) {
	process->saved_context[0] = func;
	_process_context_init(process, privileged);
}

void process_context_init_bybinary(Process_t* process, binary_t* binary) {
	process->saved_context[0] = binary->entry_point;
	_process_context_init(process, 0);
}
