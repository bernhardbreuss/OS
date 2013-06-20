/*
 * driver.c
 *
 *  Created on: Jun 12, 2013
 *      Author: Bernhard
 */

#include "driver.h"
#include "process.h"
#include "ipc.h"
#include <stdlib.h>

static message_t msg;

static void driver_msg_received(void) {
	int return_value = MESSAGE_UNKNOWN;
	ProcessId_t src = msg.source;
	switch (msg.value.data[0]) {
	case DRIVER_IOCTL:
		if (driver.ioctl != NULL) {
			return_value = driver.ioctl((driver_msg_t*)&msg.value.data[3], msg.value.data[2]);
		}
		break;
	case DRIVER_OPEN:
		if (driver.open != NULL) {
			return_value = (unsigned int)driver.open((driver_msg_t*)&msg.value.data[3], msg.value.data[2], (driver_mode_t)msg.value.data[1]);
		}
		break;
	case DRIVER_CLOSE:
		if (driver.close != NULL) {
			return_value = driver.close((void*)msg.value.data[1]);
		}
		break;
	case DRIVER_READ:
		if (driver.read != NULL) {
			return_value = driver.read((void*)msg.value.data[1], (driver_msg_t*)&msg.value.data[3], msg.value.data[2]);
		}
		break;
	case DRIVER_WRITE:
		if (driver.write != NULL) {
			return_value = driver.write((void*)msg.value.data[1], (driver_msg_t*)&msg.value.data[3], msg.value.data[2]);
		}
		break;
	}

	msg.value.data[0] = return_value;

	ipc_syscall(src, IPC_SEND, &msg);
}

void driver_init(void (*async_loop)(void)) {
	if (async_loop == NULL) {
		while (1) {
			unsigned int ipc = ipc_syscall(PROCESS_ANY, IPC_RECEIVE, &msg);
			if (ipc == IPC_OK) {
				driver_msg_received();
			}
		}
	} else {
		while (1) {
			int ipc = ipc_syscall(PROCESS_ANY, IPC_RECEIVE_ASYNC, &msg);
			if (ipc == IPC_OK) {
				driver_msg_received();
			}
			async_loop();
		}
	}
}
