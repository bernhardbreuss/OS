/*
 * process_manager.c
 *
 *  Created on: 20.03.2013
 *      Author: edi
 */

#include <stdlib.h>
#include <string.h>
#include "process_manager.h"
#include "../../hal/generic/process_context/process_context.h"
#include "../../hal/generic/timer/gptimer.h"
#include "../../hal/generic/irq/irq.h"
#include "../../service/logger/logger.h"
#include "../mmu/mmu.h"
#include "../../util/linked_list.h"
#include "../ipc/ipc.h"
#include "../../hal/generic/mmu/mmu.h"

static gptimer_t _schedule_timer;

static void _process_manager_irq_schedule_handler(void);
static Process_t* _process_manager_scheduler_get_next_process(void);

static linked_list_t processes;
static linked_list_t ready_processes[PROCESS_PRIORITY_COUNT]; /* one list for each priority */
static ProcessId_t nextProcessId;

Process_t* process_manager_current_process;

void* process_context_pointer;

static uint32_t idle_task(void) {
	while (1) ; /* TODO: look manual for HALT command or similar to reduce power consumption */
}

void process_manager_init(mmu_table_t* kernel_page_table) {
	linked_list_init(&processes);

	int i;
	for (i = 0; i < (sizeof(ready_processes) / sizeof(linked_list_t)); i++) {
		linked_list_init(&ready_processes[i]);
	}

	static Process_t kernel;
	kernel.binary = NULL;
	kernel.ipc.call_type = IPC_NOOP;
	linked_list_init(&kernel.ipc.sender);
	kernel.name = "Kernel";
	kernel.page_table = kernel_page_table;
	kernel.pid = 0;
	kernel.priority = PROCESS_PRIORITY_HIGH;

	process_manager_current_process = &kernel;
	process_context_pointer = kernel.saved_context;
	linked_list_add(&processes, &kernel);
	linked_list_add(&ready_processes[kernel.priority], &kernel);
	nextProcessId = 1;


	/* TODO: start mmu */

	/* start idle process */
	static Process_t idle_process;
	idle_process.binary = NULL;
	idle_process.name = "Idle process";
	idle_process.priority = PROCESS_PRIORITY_LOW;
	idle_process.func = &idle_task;
	process_manager_add_process(&idle_process); /* TODO: check for errors */

	gptimer_get_schedule_timer(&_schedule_timer);
	irq_add_handler(_schedule_timer.interrupt_line_id, &_process_manager_irq_schedule_handler);
	gptimer_schedule_timer_init(&_schedule_timer);
	gptimer_start(&_schedule_timer);
}

ProcessId_t process_manager_add_process(Process_t *theProcess) {
	theProcess->ipc.call_type = IPC_NOOP;
	linked_list_init(&theProcess->ipc.sender);
	theProcess->page_table = mmu_init_process(0); /* TODO: check page_table */
	theProcess->state = PROCESS_READY;
	if (!process_context_init(theProcess)) {
		/* TODO: destroy page table */
		return INVALID_PROCESS_ID;
	}
	theProcess->pid = nextProcessId++;

	linked_list_add(&processes, theProcess);
	linked_list_add(&ready_processes[theProcess->priority], theProcess);

	return theProcess->pid;
}

void process_manager_change_process(Process_t* process) {
	if (process == NULL || process->state == PROCESS_BLOCKED) {
		process = _process_manager_scheduler_get_next_process();
	}

	if (process == process_manager_current_process) {
		return;
	}

	if (process_manager_current_process->state == PROCESS_RUNNING) {
		process_manager_current_process->state = PROCESS_READY;
	}

	process_context_pointer = process->saved_context;
	process->state = PROCESS_RUNNING;
	process_manager_current_process = process;

	logger_debug("Current process: #%u %s", process->pid, process->name);
}

Process_t* process_manager_get_process_byid(ProcessId_t id) {
	linked_list_node_t* node = processes.head;
	while (node != NULL) {
		Process_t* p = (Process_t*)node->value;
		if (p->pid == id) {
			return p;
		}

		node = node->next;
	}

	return NULL;
}

static void _process_manager_irq_schedule_handler(void) {
	/* clear all pending interrupts */
	gptimer_clear_pending_interrupts(&_schedule_timer);

	process_manager_change_process(NULL);
}

Process_t* _process_manager_scheduler_get_next_process(void) {
	int i;
	for (i = 0; i < PROCESS_PRIORITY_COUNT; i++) {
		/* pop the first ready process out of the queue */
		linked_list_node_t* node = linked_list_pop_head(&ready_processes[i]);
		if (node != NULL) {
			/* insert process at the end of the queue */
			linked_list_add_node(&ready_processes[i], node);
			return node->value;
		}
	}

	return NULL;
}

binary_t* process_manager_get_binary(void) {
	if (process_manager_current_process == NULL) {
		return NULL;
	}

	return process_manager_current_process->binary;
}

void process_manager_set_process_ready(Process_t* process) {
	process->state = PROCESS_READY;
	linked_list_add(&ready_processes[process->priority], process);
}

#pragma SWI_ALIAS(process_manager_run_process, 1)
void process_manager_run_process(Process_t* next_process);

void process_manager_block_current_process(Process_t* next_process) {
	process_manager_current_process->state = PROCESS_BLOCKED;
	linked_list_node_t* node = ready_processes[process_manager_current_process->priority].head;
	while (node != NULL) {
		if (node->value == process_manager_current_process) {
			linked_list_remove(&ready_processes[process_manager_current_process->priority], node);
		}

		node = node->next;
	}

	process_manager_run_process(next_process);
}
