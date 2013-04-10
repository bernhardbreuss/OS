/*
 * process_manager_test.c
 *
 *  Created on: 20.03.2013
 *      Author: edi
 */
#include <stdlib.h>
#include "../kernel/process_manager.h"
#include "../kernel/process.h"
#include "process_manager_test.h"

int foo(void) {
	return 0;
}

int test_process_manager_1(void) {

	int i;
	int max_processes_plus_one = (MAX_PROCESSES + 1);
	Process_t *processes = malloc(sizeof(Process_t) * max_processes_plus_one);
	Process_t *firstProcess = processes;
	ProcessManager processManager;

	//last process is to much for the manager
	for(i = 0; i < max_processes_plus_one; i++) {
		processes->pid = 1;
		processes->priority = HIGH;
		processes->state = READY;
		processes->func = foo;
		processes->func(); 		//call it for fun
		processes++;
	}
	processes = firstProcess;
	process_manager_init(&processManager);

	for(i = 0; i < max_processes_plus_one; i++) {
		processes->pid = process_manager_add_process(&processManager, processes);
		processes++;
	}
	processes = firstProcess;
	for(i = 0; i < MAX_PROCESSES; i++) {
		if(processes->pid != i) {
			return -1;
		}
		processes++;
	}
	//this process has been to much for the manager
	if(processes->pid != -1) {
		return -2;
	}

	processes = firstProcess;
	free(processes);
	processes = 0;
	return 1;
}


