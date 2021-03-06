/*
 * process.c
 *
 *  Created on: Jun 12, 2013
 *      Author: Bernhard
 */

#include "process.h"
#include "message.h"
#include "ipc.h"
#include "system.h"
#include "string.h"

ProcessId_t process_find(process_name_t name) {
	message_t msg;
	msg.value.data[0] = SYSTEM_FIND_PROCESS;
	strncpy(&msg.value.buffer[sizeof(unsigned int)], name, PROCESS_MAX_NAME_LENGTH);

	uint32_t status = ipc_syscall(PROCESS_SYSTEM, IPC_SENDREC, &msg);

	if (status != IPC_OK || msg.value.data[0] != SYSTEM_OK) {
		return PROCESS_INVALID_ID;
	} else {
		return msg.value.data[1];
	}
}
