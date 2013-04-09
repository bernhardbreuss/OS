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

enum ProcessState {
	READY,
	RUNNING,
	BLOCKED
};

enum ProcessPriority {
	HIGH,
	MEDIUM,
	LOW
};

typedef struct {
	ProcessId pid;
	process_func func;
	enum ProcessState state;
	enum ProcessPriority priority;
} Process;

#endif /* PS_H_ */
