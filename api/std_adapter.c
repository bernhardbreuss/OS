/*
 * std_adapter.c
 *
 *  Created on: Jun 17, 2013
 *      Author: Bernhard
 */

#include <file.h>
#include <stdio.h>
#include "ipc.h"
#include "process.h"
#include <stdlib.h>

static message_t msg;

static int std_adapter_open(const char* path, unsigned flags, int llv_fd) {
	return 0;
}

static int std_adapter_close(int dev_fd) {
	return 0;
}

static int std_adapter_read(int dev_fd, char* bu, unsigned count) {
	int ipc = ipc_syscall(PROCESS_STDIN, IPC_RECEIVE, &msg);
	if (ipc != IPC_OK) {
		return -1;
	}

	memcpy(bu, &msg.value.data[1], msg.value.data[0]);
	return msg.value.data[0];
}

static int std_adapter_write(int dev_fd, const char* buf, unsigned count) {
	while (count > 0) {
		size_t to_copy = (count > (sizeof(msg.value) - sizeof(unsigned int)) ? (sizeof(msg.value) - sizeof(unsigned int)) : count);
		msg.value.data[0] = to_copy;
		memcpy(&msg.value.data[1], buf, to_copy);

		int ipc = ipc_syscall(PROCESS_STDOUT, IPC_SEND, &msg);
		if (ipc != IPC_OK) {
			return -1;
		}
		count -= to_copy;
	}
	return 0;
}

static off_t std_adapter_lseek(int dev_fd, off_t offset, int origin) {
	return -1;
}

static int std_adapter_unlink(const char* path) {
	return -1;
}

static int std_adapter_rename(const char* old_name, const char* new_name) {
	return -1;
}

void std_adapter_adapt(void) {
	/* http://processors.wiki.ti.com/index.php/Tips_for_using_printf#Using_printf.28.29_to_output_to_a_user-defined_device */
	add_device("std_adapter", _SSA, std_adapter_open, std_adapter_close, std_adapter_read, std_adapter_write, std_adapter_lseek, std_adapter_unlink, std_adapter_rename);
	fopen("std_adapter", "w");
	freopen("std_adapter:", "w", stdout); /* redirect stdout */
	fopen("std_adapter", "r");
	freopen("std_adapter:", "r", stdin); /* redirect stdin */
	setvbuf(stdout, NULL, _IONBF, 0); /* turn off buffering for stdout */
	setvbuf(stdin, NULL, _IONBF, 0); /* turn off buffering for stdin */
}
