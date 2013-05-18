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

uint8_t process_context_init(Process_t* process) {
	int* stack = malloc(PROCESS_STACK_SIZE);
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
		process->saved_context[14] = stack + (PROCESS_STACK_SIZE / sizeof(int*)); /* R13 */
		process->saved_context[16] = (void*)_get_CPSR(); /* initialize CPSR with current CPSR */

		return 1;
	} else {
		return 0;
	}
}
