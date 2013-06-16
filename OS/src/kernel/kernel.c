/*
 * kernel.c
 *
 *  Created on: Jun 4, 2013
 *      Author: Bernhard
 */

#include "kernel.h"
#include "process/process_manager.h"
#include <ipc.h>
#include "string.h"

static message_t msg;

static unsigned int kernel_start_process(void) {
	process_name_t name = malloc(sizeof(process_name_t));
	strncpy((char*)name, &msg.value.buffer[sizeof(unsigned int) * 2], sizeof(PROCESS_MAX_NAME_LENGTH));

	ProcessId_t pid = process_manager_start_process_bybinary((binary_t*)msg.value.data[1], name, PROCESS_PRIORITY_HIGH);
	msg.value.data[1] = pid;

	return KERNEL_OK;
}

static unsigned int kernel_find_process(void) {
	Process_t* p = process_manager_get_process_byname(&msg.value.buffer[sizeof(unsigned int)]);
	if (p == NULL) {
		return KERNEL_ERROR;
	} else {
		msg.value.data[1] = p->pid;
		return KERNEL_OK;
	}
}

static unsigned int mem_io_read(message_t *msg){

	unsigned int* address;
	unsigned int address_data;

	address = (unsigned int*) msg->value.data[2];
	address_data = *address;
	msg->value.data[1] =  address_data;

	return KERNEL_OK;

}

static unsigned int mem_io_write(message_t *msg){

	unsigned int* address;

	address = (unsigned int*) msg->value.data[2];
	*address = msg->value.data[1];

	return KERNEL_OK;
}

void kernel_main_loop(void) {
	while (1) {
		ipc_syscall(PROCESS_ANY, IPC_RECEIVE, &msg);

		switch (msg.value.data[0]) {
		case KERNEL_START_PROCESS:
			msg.value.data[0] = kernel_start_process();
			break;
		case KERNEL_FIND_PROCESS:
			msg.value.data[0] = kernel_find_process();
			break;
		case MEM_IO_READ:
			msg.value.data[0] = mem_io_read(&msg);
			break;
		case MEM_IO_WRITE:
			msg.value.data[0] = mem_io_write(&msg);
			break;
		default:
			msg.value.data[0] = KERNEL_ERROR;
		}

		ipc_syscall(msg.source, IPC_SEND, &msg);
	}
}
