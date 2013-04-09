#ifndef SYSTEM_CALL_H_
#define SYSTEM_CALL_H_

/*  -------------------------
 *	Process management
 *  -------------------------
 */

// TODO: change 'status' to a typedef...

#include "../process.h"

/**
 * Creates a new process.
 * @returns process id.
 */
ProcessId sc_fork();

/**
 * Wait for a child to terminate
 * @returns process id.
 */
ProcessId sc_wait_for_pid(int pid);

/**
 * Terminate process execution and return status
 */
void sc_exit(ProcessState status);

/**
 * Return the caller's process id.
 * @returns process id.
 */
ProcessId sc_get_pid();

#endif /* SYSTEM_CALL_H_ */