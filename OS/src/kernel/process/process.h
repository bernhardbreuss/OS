/*
 * ps.h
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

typedef uint32_t (*process_func)(void);
typedef int32_t ProcessId_t;

#define PROCESS_ANY -2
#define INVALID_PROCESS_ID -1 /* TODO: rename with prefix PROCESS ... */
#define PROCESS_KERNEL	0

#define process_name_t char*

typedef enum {
	PROCESS_READY,
	PROCESS_RUNNING,
	PROCESS_BLOCKED
} ProcessState_t;

typedef enum {
	PROCESS_PRIORITY_HIGH = 0,
	PROCESS_PRIORITY_LOW = 1
} ProcessPriority_t;
#define PROCESS_PRIORITY_COUNT 2

#include "../ipc/message.h" /* do not move this up! otherwise circular inclusion will happen. */

typedef struct {
	ProcessId_t pid;
	process_func func;
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
} Process_t;

#endif /* PS_H_ */
