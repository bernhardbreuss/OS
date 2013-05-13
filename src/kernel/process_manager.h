/*
 * process_manager.h
 *
 *  Created on: 20.03.2013
 *      Author: edi
 */

#ifndef PROCESS_MANAGER_H_
#define PROCESS_MANAGER_H_

#include "process.h"

#define MAX_PROCESSES 			10

/**
 * Initializes the processSlots array of the manager with NULL pointers.
 * Initializes the schedule timer and register interrupt handler.
 */
void process_manager_init(void);

void process_manager_start_scheduling(void);

/**
 * Adds the Process to the manager.
 * @return The ProcesssId. The ProcessId is set to -1 if the process can not be added.
 */
ProcessId_t process_manager_add_process(Process_t *theProcess);

void process_manager_change_process(ProcessId_t processId);

ProcessId_t process_manager_get_process(process_name_t processName);

uint32_t process_manager_start_managing(ProcessId_t processId);

Process_t* process_manager_get_current_process();

Process_t* process_manager_get_process_byid(ProcessId_t id);

void process_manager_block_current_process(void);
#endif /* PROCESS_MANAGER_H_ */
