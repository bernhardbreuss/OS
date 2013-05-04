/*
 * ps.h
 *
 *  Created on: 19.03.2013
 *      Author: edi
 */

#ifndef PS_H_
#define PS_H_

#include "../hal/platform.h"

typedef int (*process_func)(void);
typedef int ProcessId_t;

#define INVALID_PROCESS_ID -1

typedef enum {
	READY,
	RUNNING,
	BLOCKED
} ProcessState_t;

typedef enum {
	HIGH,
	MEDIUM,
	LOW
} ProcessPriority_t;

typedef struct {
	ProcessId_t pid;
	process_func func;
	ProcessState_t state;
	ProcessPriority_t priority;
	void* saved_context[PROCESS_CONTEXT_SIZE];
} Process_t;

#endif /* PS_H_ */
