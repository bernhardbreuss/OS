/*
 * ps.h
 *
 *  Created on: 19.03.2013
 *      Author: edi
 */

#ifndef PS_H_
#define PS_H_

typedef int (*process_func)(void);
typedef int ProcessId;

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
	ProcessId pid;
	process_func func;
	ProcessState_t state;
	ProcessPriority_t priority;
} Process_t;

#endif /* PS_H_ */
