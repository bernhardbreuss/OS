/*
 * osstd.c
 *
 *  Created on: Jun 12, 2013
 *      Author: Bernhard
 */

#include "os_stdio.h"
#include "message.h"
#include "driver.h"
#include "ipc.h"
#include "process.h"
#include "driver_manager.h"

static message_t msg;

#define OSSTD_COPY_MSG(buf, size) { \
	msg.value.data[2] = (size < DRIVER_BUFFER_SIZE) ? size : DRIVER_BUFFER_SIZE; \
	memcpy(&msg.value.data[3], buf, msg.value.data[1]); \
}

static int osstd_send_handle(int operation, handle_t* handle, driver_msg_t* buf, size_t size) {
	if (handle->driver == PROCESS_INVALID_ID) {
		return MESSAGE_DEVICE_UNKNOWN;
	}

	msg.value.data[0] = operation;
	msg.value.data[1] = (unsigned int)handle->handle;

	OSSTD_COPY_MSG(buf, size)
	int ipc = ipc_syscall(handle->driver, IPC_SENDREC, &msg);

	if (ipc == IPC_OTHER_NOT_FOUND) {
		return MESSAGE_DEVICE_UNKNOWN;
	}

	return ipc;
}

static int osstd_send_device(int operation, Device_t device, driver_msg_t* buf, size_t size) {
	msg.value.data[0] = DRIVER_MANAGER_GET;
	msg.value.data[1] = device;
	int ipc = ipc_syscall(PROCESS_DRIVER_MANAGER, IPC_SENDREC, &msg);
	if (ipc != IPC_OK || (ProcessId_t)msg.value.data[0] == PROCESS_INVALID_ID) {
		return MESSAGE_DEVICE_UNKNOWN;
	}

	handle_t handle;
	handle.driver = msg.value.data[0];
	handle.handle = NULL;

	return osstd_send_handle(operation, &handle, buf, size);
}

int os_ioctl(Device_t device, driver_msg_t* buf, size_t size) {
	int ipc = osstd_send_device(DRIVER_IOCTL, device, buf, size);
	if (ipc == IPC_OK) {
		return msg.value.data[0];
	} else  {
		return ipc;
	}
}

int os_open(Device_t device, driver_msg_t* buf, size_t size, handle_t* handle, driver_mode_t mode) {
	msg.value.data[0] = DRIVER_MANAGER_GET;
	msg.value.data[1] = device;
	int ipc = ipc_syscall(PROCESS_DRIVER_MANAGER, IPC_SENDREC, &msg);

	if (ipc != IPC_OK) {
		return MESSAGE_DEVICE_UNKNOWN;
	}

	handle->driver = msg.value.data[0];
	handle->handle = (void*)mode;

	ipc = osstd_send_handle(DRIVER_OPEN, handle, buf, size);

	if (ipc == MESSAGE_DEVICE_UNKNOWN) {
		handle->driver = PROCESS_INVALID_ID;
		handle->handle = NULL;
		return MESSAGE_DEVICE_UNKNOWN;
	}

	handle->handle = (void*)msg.value.data[0];

	return ipc;
}

/* closes device and handle is set to NULL */
int close(handle_t* handle) {
	int return_val = osstd_send_handle(DRIVER_CLOSE, handle, NULL, 0);
	handle->driver = PROCESS_INVALID_ID;
	handle->handle = NULL;
	return return_val;
}

int os_write(handle_t* handle, driver_msg_t* buf, size_t size) {
	return osstd_send_handle(DRIVER_WRITE, handle, buf, size);
}

int os_read(handle_t* handle, driver_msg_t* buf, size_t size) {
	return osstd_send_handle(DRIVER_WRITE, handle, buf, size);
}
