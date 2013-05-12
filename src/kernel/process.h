/*
 * ps.h
 *
 *  Created on: 19.03.2013
 *      Author: edi
 */

#ifndef PS_H_
#define PS_H_

#include <inttypes.h>
#include "../hal/platform.h"

typedef uint32_t (*process_func)(void);
typedef int32_t ProcessId_t;

#define PROCESS_ANY -2
#define INVALID_PROCESS_ID -1 /* TODO: rename with prefix PROCESS ... */

#define process_name_t char*

typedef enum {
	PROCESS_READY,
	PROCESS_RUNNING,
	PROCESS_BLOCKED
} ProcessState_t;

typedef enum {
	HIGH,
	MEDIUM,
	LOW
} ProcessPriority_t;

#include "ipc/message.h" /* do not move this up! otherwise circular includion will happen. */

typedef struct {
	ProcessId_t pid;
	process_func func;
	ProcessState_t state;
	struct {
		ProcessId_t other;
		uint8_t call_type;
		message_t* msg;
	} ipc;
	ProcessPriority_t priority;
	void* saved_context[PROCESS_CONTEXT_SIZE];
	char* name;
} Process_t;

#endif /* PS_H_ */
