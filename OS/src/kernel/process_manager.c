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
#include "../hal/generic/timer/gptimer.h"
#include "../hal/generic/irq/irq.h"
#include "../service/logger/logger.h"

static ProcessId_t currentProcessId;
static gptimer_t _schedule_timer;

static void _process_manager_irq_schedule_handler(void);
static ProcessId_t _process_manager_scheduler_get_next_process(void);

Process_t* processSlots[MAX_PROCESSES]; /* TODO: check if dependency in ipc.c could be resolved */

static void* tmp_saved_context[PROCESS_CONTEXT_SIZE]; /* used before process_manager is fully functional */
void* process_context_pointer = tmp_saved_context;

void process_manager_store_context(unsigned int*);
void process_manager_load_context(unsigned int*);

void process_manager_init(void) {
	int i;
	for(i = 0; i < MAX_PROCESSES; i++) {
		processSlots[i] = NULL;
	}

	gptimer_get_schedule_timer(&_schedule_timer);
	irq_add_handler(_schedule_timer.interrupt_line_id, &_process_manager_irq_schedule_handler);
	gptimer_schedule_timer_init(&_schedule_timer);
	currentProcessId = INVALID_PROCESS_ID;
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
	if (processId == INVALID_PROCESS_ID) {
		processId = _process_manager_scheduler_get_next_process();
	}

	Process_t* p;
	if (currentProcessId != INVALID_PROCESS_ID) {
		p = processSlots[currentProcessId];
		if (p != NULL && p->state == PROCESS_RUNNING) {
			p->state = PROCESS_READY;
		}
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

Process_t* process_manager_get_current_process() {
	if (currentProcessId == INVALID_PROCESS_ID) {
		return NULL;
	} else {
		return processSlots[currentProcessId];
	}
}

Process_t* process_manager_get_process_byid(ProcessId_t id) {
	if (id > sizeof(processSlots) || id == INVALID_PROCESS_ID) {
		return NULL;
	}

	return processSlots[id];
}

void process_manager_start_scheduling() {
	gptimer_start(&_schedule_timer);

	process_manager_block_current_process(INVALID_PROCESS_ID);
}

void _process_manager_irq_schedule_handler(void) {
	/* clear all pending interrupts */

	//TODO: this is the wrong place for this code
	gptimer_clear_pending_interrupts(&_schedule_timer);
	*((unsigned int*)0x48200048) = 0x1; /* INTCPS_CONTROL s. 1083 */

	process_manager_change_process(INVALID_PROCESS_ID);
}

ProcessId_t _process_manager_scheduler_get_next_process(void) {
	int i = currentProcessId + 1;
	Process_t* p;
	while (i != currentProcessId) {
		if (i >= MAX_PROCESSES) {
			i = 0;
		}

		p = processSlots[i];
		if (p != NULL && p->state == PROCESS_READY) {
			return p->pid;
		}

		i++;
	}

	return INVALID_PROCESS_ID;
}
