/*
 * process_manager.c
 *
 *  Created on: 20.03.2013
 *      Author: edi
 */

#include <stdlib.h>
#include <string.h>
#include "process_manager.h"
#include "../hal/platform.h"
#include "../hal/generic/process_context/process_context.h"
#include "../service/logger/logger.h"

static ProcessId_t currentProcessId;
Process_t* processSlots[MAX_PROCESSES]; /* TODO: rename to english process */

void* process_context_pointer;

void process_manager_store_context(unsigned int*);
void process_manager_load_context(unsigned int*);

void process_manager_init() {
	int i;
	for(i = 0; i < MAX_PROCESSES; i++) {
		processSlots[i] = NULL;
	}
}
ProcessId_t process_manager_add_process(Process_t *theProcess) {
	int i;
	for (i = 0; i < MAX_PROCESSES; i++) {
		if (processSlots[i] == NULL) {
			if (process_context_init(theProcess)) {
				theProcess->pid = i;
				theProcess->state = PROCESS_READY;

				processSlots[i] = theProcess;
				return i;
			} else {
				break;
				/* TODO: return error code */
			}
		}
	}
	return INVALID_PROCESS_ID;
}

void process_manager_change_process(ProcessId_t processId) {
	Process_t* p = processSlots[currentProcessId];
	if (p != NULL && p->state == PROCESS_RUNNING) {
		p->state = PROCESS_READY;
	}

	p = processSlots[processId];

	process_context_pointer = p->saved_context;
	p->state = PROCESS_RUNNING;
	currentProcessId = processId;

	logger_debug("Current process: #%u %s", p->pid, p->name);
}

ProcessId_t process_manager_get_process(process_name_t processName) {
	int i;
	for (i = 0; i < MAX_PROCESSES; i++) {
		if (processSlots[i]->pid != NULL) {
			if (strcmp(processSlots[i]->name, processName) == 0) {
				return processSlots[i]->pid;
			}
		}
	}

	return INVALID_PROCESS_ID;
}

uint32_t process_manager_start_managing(ProcessId_t idleProcessId) {
	process_manager_change_process(idleProcessId);
	return 0; /* TODO: ... */
}

Process_t* process_manager_get_current_process() {
	return processSlots[currentProcessId];
}

Process_t* process_manager_get_process_byid(ProcessId_t id) {
	if (id > sizeof(processSlots) || id == INVALID_PROCESS_ID) {
		return NULL;
	}

	return processSlots[id];
}

#pragma TASK(process_manager_block_current_process_c);
void process_manager_block_current_process_c(void) {
	processSlots[currentProcessId]->state = PROCESS_BLOCKED;

	int i;
	for (i = 0; i < MAX_PROCESSES; i++) {
		Process_t* p = processSlots[i];
		if (p != NULL && p->state == PROCESS_READY) {
			currentProcessId = p->pid;
			break;
		}
	}

	process_manager_change_process(currentProcessId);
	asm("\t SWI #0x2"); /* process_context_load */
	/* TODO: blocking process should be possible with one SWI */
}
