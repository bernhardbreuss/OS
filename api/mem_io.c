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

int memory_mapped_read(unsigned int address[], size_t size){
	message_t ipc_msg;
	int i;
	ipc_msg.value.data[0] = MEM_IO_READ;
	for(i = 1; i <= size; i++){
			ipc_msg.value.data[i] = address[i-1];
		}
	ipc_msg.size = size + 1;
	ipc_msg.type = MESSAGE_TYPE_DATA;
	ipc_syscall( 0, IPC_SENDREC, &ipc_msg);
	if((ipc_msg.size - 1) != size){
		return -1;
	}
	for(i = 1; i <= ipc_msg.size; i++){
		address[i-1] = ipc_msg.value.data[i];
	}

	return 0;
}

/*
 * size = size of addresses
 */
void memory_mapped_write(unsigned int msg[], unsigned int address[], size_t size) {

	int l = 0;
	while(size > 0) {
		message_t ipc_msg;
		ipc_msg.value.data[0] = MEM_IO_WRITE;
		int i;
		int min = ((MESSAGE_DATA_SIZE-1)/2 < size) ? (MESSAGE_DATA_SIZE-1)/2 : size;
		for(i = 0; i < min; i++){
			ipc_msg.value.data[i*2 + 1] = msg[((MESSAGE_DATA_SIZE/2) * l) + i];
			ipc_msg.value.data[i*2 + 2] = address[((MESSAGE_DATA_SIZE/2) * l) + i];
		}

		size -= min;
		ipc_msg.size = 1 + (2 * min);
		ipc_msg.type = MESSAGE_TYPE_DATA;
		ipc_syscall( 0, IPC_SENDREC, &ipc_msg);
		l++;
	}
}
