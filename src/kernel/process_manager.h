/*
 * process_manager.h
 *
 *  Created on: 20.03.2013
 *      Author: edi
 */

#ifndef PROCESS_MANAGER_H_
#define PROCESS_MANAGER_H_

#include "process.h"

#define MAX_PROCESSES 			3

typedef struct {
	ProcessId_t currentProcessId;
	Process_t *prozessSlots[MAX_PROCESSES];

} ProcessManager_t;

/**
 * Initializes the processSlots array of the manager with NULL pointers.
 */
void process_manager_init(ProcessManager_t *processManager);

/**
 * Adds the Process to the manager.
 * @return The ProcesssId. The ProcessId is set to -1 if the process can not be added.
 */
ProcessId_t process_manager_add_process(ProcessManager_t *processManager, Process_t *theProcess);

void process_manager_change_process(ProcessManager_t *processManager, ProcessId_t processId);

#endif /* PROCESS_MANAGER_H_ */
