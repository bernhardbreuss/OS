/*
 * kernel.c
 *
 *  Created on: Jun 4, 2013
 *      Author: Bernhard
 */

#include "system.h"
#include "process/process_manager.h"
#include <ipc.h>
#include "string.h"

static message_t msg;

static unsigned int system_start_process(void) {
	Process_t* p = process_manager_start_process_bybinary((binary_t*)msg.value.data[1], PROCESS_PRIORITY_HIGH, msg.value.buffer[sizeof(unsigned int) * 3], &msg.value.buffer[sizeof(unsigned int) * 4]);
	if (p != NULL) {
		msg.value.data[1] = p->pid;
	} else {
		msg.value.data[1] = (unsigned int)PROCESS_INVALID_ID;

		ProcessId_t pid = msg.value.data[2];
		if (pid != PROCESS_INVALID_ID) {
			Process_t* other = process_manager_get_process_byid(pid);

			if (other != NULL) {
				other->stdin = pid;
				p->stdout = pid;
			}
		}
	}

	return SYSTEM_OK;
}

static unsigned int system_find_process(void) {
	Process_t* p = process_manager_get_process_byname(&msg.value.buffer[sizeof(unsigned int)]);
	if (p == NULL) {
		return SYSTEM_ERROR;
	} else {
		msg.value.data[1] = p->pid;
		return SYSTEM_OK;
	}
}

static unsigned int mem_io_read(message_t *msg){

	unsigned int* address;
	unsigned int address_data;
	int i;

	for( i = 1 ; i < msg->size; i++ ){
		address = (unsigned int*) msg->value.data[i];
		address_data = *address;
		msg->value.data[i] =  address_data;
	}
	return SYSTEM_OK;

}

static unsigned int mem_io_write(message_t *msg){

	unsigned int* address;
	int i;

	for( i = 1 ; i < msg->size; i += 2 ){
		address = (unsigned int*) msg->value.data[i+1];
		*address = msg->value.data[i];
	}

	return SYSTEM_OK;
}

void system_main_loop(void) {
	while (1) {
		ipc_syscall(PROCESS_ANY, IPC_RECEIVE, &msg);

		switch (msg.value.data[0]) {
		case SYSTEM_START_PROCESS:
			msg.value.data[0] = system_start_process();
			break;
		case SYSTEM_FIND_PROCESS:
			msg.value.data[0] = system_find_process();
			break;
		case MEM_IO_READ:
			msg.value.data[0] = mem_io_read(&msg);
			break;
		case MEM_IO_WRITE:
			msg.value.data[0] = mem_io_write(&msg);
			break;
		default:
			msg.value.data[0] = SYSTEM_ERROR;
		}

		ipc_syscall(msg.source, IPC_SEND, &msg);
	}
}
