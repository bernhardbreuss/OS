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
#include <linked_list.h>
#include "../../hal/generic/mmu/mmu.h"
#include <ipc.h>
#include "../../idle_process.h"
#include "../loader/loader.h"

static gptimer_t _schedule_timer;

static void _process_manager_irq_schedule_handler(void);
static Process_t* _process_manager_scheduler_get_next_process(void);

static linked_list_t processes;
static linked_list_t ready_processes[PROCESS_PRIORITY_COUNT]; /* one list for each priority */
static ProcessId_t nextProcessId;

Process_t* process_manager_current_process;
Process_t process_manager_kernel_process;

void* process_context_pointer;

extern void* idle_process_virtual;
extern void* idle_process_physical;
extern void* idle_process_size;

static ProcessId_t _process_manager_start_process(Process_t* process, mmu_table_t* page_table, char* name, ProcessPriority_t priority) {
	process->name = name;
	process->ipc.call_type = IPC_NOOP;
	linked_list_init(&process->ipc.sender);
	process->page_table = page_table;
	process->priority = priority;
	process->state = PROCESS_READY;

	process->pid = nextProcessId++;
	linked_list_add(&processes, process);
	linked_list_add(&ready_processes[priority], process);

	return process->pid;
}

void process_manager_init(mmu_table_t* kernel_page_table) {
	linked_list_init(&processes);

	int i;
	for (i = 0; i < (sizeof(ready_processes) / sizeof(linked_list_t)); i++) {
		linked_list_init(&ready_processes[i]);
	}

	nextProcessId = PROCESS_KERNEL;
	process_manager_kernel_process.binary = NULL;
	_process_manager_start_process(&process_manager_kernel_process, kernel_page_table, "Kernel", PROCESS_PRIORITY_HIGH);
	process_manager_kernel_process.state = PROCESS_RUNNING;
	process_manager_current_process = &process_manager_kernel_process;
	process_context_pointer = process_manager_kernel_process.saved_context;

	mmu_start();

	/* start loader */
	static Process_t loader;
	loader.binary = NULL;
	process_context_init_byfunc(&loader, &loader_main, 1);
	_process_manager_start_process(&loader, kernel_page_table, "Loader", PROCESS_PRIORITY_HIGH);
	process_manager_run_process(&loader); /* give the loader process the possibility to initialize before another process could be started */

	/* start idle process */
	process_manager_start_process_byfunc(&idle_process, "idle process", PROCESS_PRIORITY_LOW, (unsigned int)&idle_process_virtual, (unsigned int)&idle_process_physical, (unsigned int)&idle_process_size); /* TODO: check for errors */

	gptimer_get_schedule_timer(&_schedule_timer);
	irq_add_handler(_schedule_timer.interrupt_line_id, &_process_manager_irq_schedule_handler);
	gptimer_schedule_timer_init(&_schedule_timer);
	gptimer_start(&_schedule_timer);
}

ProcessId_t process_manager_start_process_byfunc(process_func_t func, char* name, ProcessPriority_t priority, unsigned int virtual_address, unsigned int physical_address, unsigned int size) {
	Process_t* process = malloc(sizeof(Process_t));
	if (process == NULL) {
		return INVALID_PROCESS_ID;
	}
	mmu_table_t* page_table = mmu_init_process(0);
	if (page_table == NULL) {
		free(process);
		return INVALID_PROCESS_ID;
	}

	unsigned int end_address = virtual_address + (unsigned int)size;
	while (virtual_address < end_address) {
		unsigned int mapped = mmu_map(page_table, (void*)virtual_address, (void*)physical_address);
		if (mapped == 0) {
			/* TODO: destroy page table */
			free(process);
			return INVALID_PROCESS_ID;
		}

		virtual_address += mapped;
		physical_address += mapped;
	}

	process->binary = NULL;
	process_context_init_byfunc(process, func, 0);
	return _process_manager_start_process(process, page_table, name, priority);
}

ProcessId_t process_manager_start_process_bybinary(binary_t* binary, char* name, ProcessPriority_t priority) {
	Process_t* process = malloc(sizeof(Process_t));
	if (process == NULL) {
		return INVALID_PROCESS_ID;
	}
	mmu_table_t* page_table = mmu_init_process(0);
	if (page_table == NULL) {
		free(process);
		return INVALID_PROCESS_ID;
	}

	process->binary = binary;
	process_context_init_bybinary(process, binary);
	return _process_manager_start_process(process, page_table, name, priority);
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

	mmu_activate_process(process);

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

void process_manager_set_process_ready(Process_t* process) {
	if (process->state != PROCESS_BLOCKED) {
		return;
	}

	process->state = PROCESS_READY;
	linked_list_insert_begin(&ready_processes[process->priority], process);
}

void process_manager_block_current_process(void) {
	if (process_manager_current_process->state == PROCESS_BLOCKED) {
		return;
	}

	process_manager_current_process->state = PROCESS_BLOCKED;
	linked_list_node_t* node = ready_processes[process_manager_current_process->priority].head;
	while (node != NULL) {
		if (node->value == process_manager_current_process) {
			linked_list_remove(&ready_processes[process_manager_current_process->priority], node);
		}

		node = node->next;
	}
}
