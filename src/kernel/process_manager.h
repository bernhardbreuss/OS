/*
 * process_manager.h
 *
 *  Created on: 20.03.2013
 *      Author: edi
 */

#ifndef PROCESS_MANAGER_H_
#define PROCESS_MANAGER_H_

#include "process.h"

#define MAX_PROCESSES 			2

typedef struct {
	Process *prozessSlots[MAX_PROCESSES];

} ProcessManager;

/**
 * Initializes the processSlots array of the manager with NULL pointers.
 */
void process_manager_init(ProcessManager *processManager);

/**
 * Adds the Process to the manager.
 * @return The ProcesssId. The ProcessId is set to -1 if the process can not be added.
 */
ProcessId process_manager_add_process(ProcessManager *processManager, Process *theProcess);

#endif /* PROCESS_MANAGER_H_ */
