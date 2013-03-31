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

enum PROCESS_STATE {
	READY,
	RUNNING,
	BLOCKED
};

enum PROCESS_PRIORITY {
	HIGH,
	MEDIUM,
	LOW
} ;

typedef struct {
	ProcessId pid;
	process_func func;
	enum PROCESS_STATE state;
	enum PROCESS_PRIORITY priority;
} Process;

#endif /* PS_H_ */
