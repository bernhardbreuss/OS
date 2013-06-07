/*
 * kernel.c
 *
 *  Created on: Jun 4, 2013
 *      Author: Bernhard
 */

#include "kernel.h"
#include "process/process_manager.h"
#include "ipc/ipc.h"

static message_t msg;

static unsigned int kernel_start_process(void) {
	if (msg.value.data[1] == KERNEL_START_PROCESS_BYFUNC) {
		msg.value.data[1] = (unsigned int)process_manager_start_process_byfunc(
				(process_func)msg.value.data[2],
				(char*)msg.value.data[6],
				PROCESS_PRIORITY_HIGH,
				msg.value.data[3], msg.value.data[4], msg.value.data[5]);

		return KERNEL_OK;
	} else if (msg.value.data[1] == KERNEL_START_PROCESS_BYPATH) {
		return KERNEL_ERROR; /* TODO: implement */
	} else {
		return KERNEL_ERROR;
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
	unsigned int address_data;

	address = (unsigned int*) msg->value.data[2];
	address_data = msg->value.data[1];
	*address = address_data;

	return KERNEL_OK;
}

void kernel_main_loop(void) {
	while (1) {
		ipc_syscall(PROCESS_ANY, IPC_RECEIVE, &msg);

		switch (msg.value.data[0]) {
		case KERNEL_START_PROCESS:
			msg.value.data[0] = kernel_start_process();
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
