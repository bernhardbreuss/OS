/*
 * interrupts.c
 *
 *  Created on: Jun 20, 2013
 *      Author: Bernhard
 */

#include <ring_buffer.h>
#include "message.h"
#include "process.h"
#include "system.h"
#include "ipc.h"

void register_interrupt(ring_buffer_t* buffer, int interrupt_id, char* address, size_t size) {
	message_t msg;
	msg.value.data[0] = REGISTER_INTERRUPT;
	msg.value.data[1] = (unsigned int)buffer;
	msg.value.data[2] = interrupt_id;
	msg.value.data[3] = (unsigned int)address;
	msg.value.data[4] = size;
	ipc_syscall(PROCESS_SYSTEM, IPC_SENDREC, &msg);
}
