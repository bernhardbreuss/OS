/*
 * os_std.c
 *
 *  Created on: Jun 19, 2013
 *      Author: Bernhard
 */

#include "process.h"
#include "ipc.h"
#include "message.h"
#include "system.h"

void _os_std_exit_process(int code) {
	message_t msg;
	msg.value.data[0] = SYSTEM_END_PROCESS;
	msg.value.data[1] = code;

	int ipc;
	do {
		ipc = ipc_syscall(PROCESS_SYSTEM, IPC_SENDREC, &msg);
	} while (ipc != IPC_OK);
}
