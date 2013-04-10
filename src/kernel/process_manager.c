/*
 * process_manager.c
 *
 *  Created on: 20.03.2013
 *      Author: edi
 */

#include <stdlib.h>
#include "process_manager.h"

void process_manager_init(ProcessManager *processManager) {
	int i;
	for(i = 0; i < MAX_PROCESSES; i++) {
		processManager->prozessSlots[i] = NULL;
	}
}
ProcessId process_manager_add_process(ProcessManager *processManager, Process_t *theProcess) {
	int i;
	ProcessId addedToProcessSlot  = -1;
	for(i = 0; i < MAX_PROCESSES; i++) {
		if(processManager->prozessSlots[i] == NULL) {
			addedToProcessSlot = i;
			processManager->prozessSlots[i] = theProcess;
			break;
		}
	}
	return addedToProcessSlot ;
}



