/*
 * mem_io.c
 *
 *	32 bit version
 *
 *  Created on: 05.06.2013
 *      Author: Stephan
 */

#include "mem_io.h"
#include "ipc.h"
#include <stdlib.h>

unsigned int memory_mapped_read(unsigned int address){
	message_t ipc_msg;
	ipc_msg.value.data[0] = MEM_IO_READ;
	ipc_msg.value.data[2] = address;
	ipc_msg.size = 3;
	ipc_msg.type = MESSAGE_TYPE_DATA;
	ipc_syscall( 0, IPC_SENDREC, &ipc_msg);

	return ipc_msg.value.data[1];
}

void memory_mapped_write(unsigned int msg, unsigned int address) {
	message_t ipc_msg;
	ipc_msg.value.data[0] = MEM_IO_WRITE;
	ipc_msg.value.data[1] = msg;
	ipc_msg.value.data[2] = address;
	ipc_msg.size = 3;
	ipc_msg.type = MESSAGE_TYPE_DATA;
	ipc_syscall( 0, IPC_SENDREC, &ipc_msg);
}
