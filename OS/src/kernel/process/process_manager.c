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
#include <argument_helper.h>

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

static Process_t* _process_manager_start_process(Process_t* process, mmu_table_t* page_table, ProcessPriority_t priority, char* args, uint8_t load_args) {
	/* copy first argument into kernel for use as pcb */
	process->name = malloc(PROCESS_MAX_NAME_LENGTH);
	strncpy(process->name, args, PROCESS_MAX_NAME_LENGTH);
	process->name[PROCESS_MAX_NAME_LENGTH - 1] = '\0';

	process->ipc.call_type = IPC_NOOP;
	linked_list_init(&process->ipc.sender);
	process->page_table = page_table;
	process->priority = priority;
	process->state = PROCESS_READY;

	if (load_args) {
		void* args_address = &ARGS_ADDR;
		void* args_physical;
		mmu_reserve(page_table, &args_address, &args_physical);
		strncpy(args_physical, args, ARGUMENTS_MAX_LENGTH);
	}

	process->pid = nextProcessId++;
	linked_list_add(&processes, process);
	linked_list_add(&ready_processes[priority], process);

	return process;
}

void process_manager_init(mmu_table_t* kernel_page_table) {
	linked_list_init(&processes);

	int i;
	for (i = 0; i < (sizeof(ready_processes) / sizeof(linked_list_t)); i++) {
		linked_list_init(&ready_processes[i]);
	}

	nextProcessId = PROCESS_SYSTEM;
	process_manager_kernel_process.binary = NULL;
	_process_manager_start_process(&process_manager_kernel_process, kernel_page_table, PROCESS_PRIORITY_HIGH, "System", 0);
	process_manager_kernel_process.state = PROCESS_RUNNING;
	process_manager_current_process = &process_manager_kernel_process;
	process_context_pointer = process_manager_kernel_process.saved_context;

	mmu_start();

	/* start loader */
	static Process_t loader;
	loader.binary = NULL;
	process_context_init_byfunc(&loader, &loader_main, 1);
	_process_manager_start_process(&loader, kernel_page_table, PROCESS_PRIORITY_HIGH, "Loader", 0);
	process_manager_run_process(&loader); /* give the loader process the possibility to initialize before another process could be started */

	/* start idle process */
	process_manager_start_process_byfunc(&idle_process, "idle process", PROCESS_PRIORITY_LOW, (unsigned int)&idle_process_virtual, (unsigned int)&idle_process_physical, (unsigned int)&idle_process_size); /* TODO: check for errors */

	gptimer_get_schedule_timer(&_schedule_timer);
	irq_add_handler(_schedule_timer.interrupt_line_id, &_process_manager_irq_schedule_handler);
	gptimer_schedule_timer_init(&_schedule_timer);
	gptimer_start(&_schedule_timer);
}

Process_t* process_manager_start_process_byfunc(process_func_t func, process_name_t name, ProcessPriority_t priority, unsigned int virtual_address, unsigned int physical_address, unsigned int size) {
	Process_t* process = malloc(sizeof(Process_t));
	if (process == NULL) {
		return NULL;
	}
	mmu_table_t* page_table = mmu_init_process(0);
	if (page_table == NULL) {
		free(process);
		return NULL;
	}

	unsigned int end_address = virtual_address + (unsigned int)size;
	while (virtual_address < end_address) {
		unsigned int mapped = mmu_map(page_table, (void*)virtual_address, (void*)physical_address);
		if (mapped == 0) {
			/* TODO: destroy page table */
			free(process);
			return NULL;
		}

		virtual_address += mapped;
		physical_address += mapped;
	}

	process->binary = NULL;
	process_context_init_byfunc(process, func, 0);
	return _process_manager_start_process(process, page_table, priority, name, 1);
}

Process_t* process_manager_start_process_bybinary(binary_t* binary, ProcessPriority_t priority, char* argv) {
	Process_t* process = malloc(sizeof(Process_t));
	if (process == NULL) {
		return NULL;
	}
	mmu_table_t* page_table = mmu_init_process(0);
	if (page_table == NULL) {
		free(process);
		return NULL;
	}

	process->binary = binary;
	process_context_init_bybinary(process, binary);
	return _process_manager_start_process(process, page_table, priority, argv, 1);
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

Process_t* process_manager_get_process_byname(process_name_t name) {
	linked_list_node_t* node = processes.head;
	while (node != NULL) {
		Process_t* p = (Process_t*)node->value;
		if (strncmp(name, p->name, PROCESS_MAX_NAME_LENGTH) == 0) {
			return p;
		}

		node = node->next;
	}

	return NULL;
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
			Process_t* p = node->value;
			if (p->state == PROCESS_READY || p->state == PROCESS_RUNNING) {
				/* insert process at the end of the queue */
				linked_list_add_node(&ready_processes[i], node);
				return node->value;
			}
		}
	}

	return NULL;
}

void process_manager_end_process(ProcessId_t pid, int exit_code) {
	Process_t* p = process_manager_get_process_byid(pid);
	if (p == NULL) {
		return;
	}

	p->state = PROCESS_ZOMBIE;
	linked_list_node_t* node = ready_processes[p->priority].head;
	while (node != NULL) {
		if (p == node->value) {
			linked_list_remove(&ready_processes[p->priority], node);
			break;
		}
		node = node->next;
	}

	node = p->ipc.sender.head;
	while (node != NULL) {
		Process_t* process = node->value;
		process_manager_set_process_ready(process);
		node = node->next;
	}
	linked_list_clear(&p->ipc.sender);

	/* TODO: free reserved ram (notify mmu/ram manager) */
	/* TODO: set exit_code */
}

void process_manager_set_process_ready(Process_t* process) {
	if (process->state != PROCESS_BLOCKED) {
		return;
	}

	/* logger_debug("process_manager: readying %i:%s", process->pid, process->name); */

	process->state = PROCESS_READY;
	linked_list_insert_begin(&ready_processes[process->priority], process);
}

void process_manager_block_current_process(void) {
	if (process_manager_current_process->state == PROCESS_BLOCKED) {
		return;
	}

	/* logger_debug("process_manager: blocking %i:%s", process_manager_current_process->pid, process_manager_current_process->name); */

	process_manager_current_process->state = PROCESS_BLOCKED;
	linked_list_node_t* node = ready_processes[process_manager_current_process->priority].head;
	while (node != NULL) {
		if (node->value == process_manager_current_process) {
			linked_list_remove(&ready_processes[process_manager_current_process->priority], node);
		}

		node = node->next;
	}
}
