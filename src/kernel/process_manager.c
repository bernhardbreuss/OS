/*
 * process_manager.c
 *
 *  Created on: 20.03.2013
 *      Author: edi
 */

#include <stdlib.h>
#include <string.h>
#include "process_manager.h"
#include "../hal/generic/irq/irq.h"
#include "../hal/platform.h"

void process_manager_store_context(unsigned int*);
void process_manager_load_context(unsigned int*);

void process_manager_init(ProcessManager_t *processManager) {
	int i;
	for(i = 0; i < MAX_PROCESSES; i++) {
		processManager->prozessSlots[i] = NULL;
	}
}
ProcessId_t process_manager_add_process(ProcessManager_t *processManager, Process_t *theProcess) {
	int i;
	for (i = 0; i < MAX_PROCESSES; i++) {
		if (processManager->prozessSlots[i] == NULL) {
			int* stack = malloc(PROCESS_STACK_SIZE);
			if (stack != NULL) {
				memset(stack, 0, PROCESS_STACK_SIZE);
				memset(theProcess->saved_context, 0, sizeof(theProcess->saved_context));

				theProcess->pid = i;
				theProcess->state = READY;
				theProcess->saved_context[0] = theProcess->func;
				theProcess->saved_context[1] = stack + (PROCESS_STACK_SIZE / sizeof(int*));

				processManager->prozessSlots[i] = theProcess;
				return i;
			} else {
				break;
				/* TODO: return error code */
			}
		}
	}
	return INVALID_PROCESS_ID;
}

void process_manager_change_process(ProcessManager_t *processManager, ProcessId_t processId) {
	Process_t* oldProcess = processManager->prozessSlots[processManager->currentProcessId];
	Process_t* newProcess = processManager->prozessSlots[processId];

	int i;
	for (i = 0; i < PROCESS_CONTEXT_SIZE; i++) {
		oldProcess->saved_context[i] = irq_saved_context[i];
		irq_saved_context[i] = newProcess->saved_context[i];
	}

	processManager->currentProcessId = processId;
}
