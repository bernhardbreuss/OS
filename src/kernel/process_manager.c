/*
 * process_manager.c
 *
 *  Created on: 20.03.2013
 *      Author: edi
 */

#include "process_manager.h"

void process_manager_init(ProcessManager *processManager) {
	int i;
	for(i = 0; i < MAX_PROCESSES; i++) {
		processManager->prozessSlots[i] = 0;
	}
}
ProcessId process_manager_add_process(ProcessManager *processManager, Process *theProcess) {
	int i;
	ProcessId addedToProcessSlot  = -1;
	for(i = 0; i < MAX_PROCESSES; i++) {
		if(processManager->prozessSlots[i] == 0) {
			addedToProcessSlot = i;
			processManager->prozessSlots[i] = theProcess;
			break;
		}
	}
	return addedToProcessSlot ;
}



