/*
 * process.h
 *
 *  Created on: 19.03.2013
 *      Author: edi
 */

#ifndef PS_H_
#define PS_H_

#include <inttypes.h>
#include "../../platform/platform.h"
#include "../mmu/mmu.h"
#include "../loader/binary.h"
#include <message.h>
#include <process.h>

typedef enum {
	PROCESS_READY,
	PROCESS_RUNNING,
	PROCESS_BLOCKED,
	PROCESS_ZOMBIE
} ProcessState_t;

typedef enum {
	PROCESS_PRIORITY_HIGH = 0,
	PROCESS_PRIORITY_LOW = 1
} ProcessPriority_t;
#define PROCESS_PRIORITY_COUNT 2

typedef struct {
	ProcessId_t pid;
	ProcessState_t state;
	struct {
		ProcessId_t other;
		uint8_t call_type;
		message_t* msg;
		linked_list_t sender;
	} ipc;
	ProcessPriority_t priority;
	void* saved_context[PROCESS_CONTEXT_SIZE];
	char* name;
	binary_t* binary;
	mmu_table_t* page_table;
	ProcessId_t stdin;
	ProcessId_t stdout;
} Process_t;

#endif /* PS_H_ */
