/*
 * process_manager.h
 *
 *  Created on: 20.03.2013
 *      Author: edi
 */

#ifndef PROCESS_MANAGER_H_
#define PROCESS_MANAGER_H_


#include "process.h"
#include "../mmu/mmu.h"

extern Process_t* process_manager_current_process;
extern Process_t process_manager_kernel_process;

/**
 * Initializes the processSlots array of the manager with NULL pointers.
 * Initializes the schedule timer and register interrupt handler.
 */
void process_manager_init(mmu_table_t* kernel_page_table);

void process_manager_start_scheduling(void);

Process_t* process_manager_start_process_byfunc(process_func_t func, char* name, ProcessPriority_t priority, unsigned int virtual_address, unsigned int physical_address, unsigned int size);
Process_t* process_manager_start_process_bybinary(binary_t* binary, ProcessPriority_t priority, int argc, char* argv);

void process_manager_change_process(Process_t* process);

uint32_t process_manager_start_managing(ProcessId_t processId);

Process_t* process_manager_get_process_byname(process_name_t processName);
Process_t* process_manager_get_process_byid(ProcessId_t id);

void process_manager_set_process_ready(Process_t* process);

void process_manager_block_current_process(void);

#pragma SWI_ALIAS(process_manager_run_process, 1)
void process_manager_run_process(Process_t* next_process);
#endif /* PROCESS_MANAGER_H_ */
