/*
 * tty.c
 *
 *  Created on: Jun 20, 2013
 *      Author: Bernhard
 */

#include <device.h>
#include <driver.h>
#include <message.h>
#include <ipc.h>
#include <os_stdio.h>
#include <string.h>
#include <argument_helper.h>
#include <system.h>
#include <process.h>

static driver_msg_t msg;
static message_t ipc_msg;
static handle_t handle;
static char command[10];//ARGUMENTS_MAX_LENGTH];
static unsigned int next;

ProcessId_t p;

void start_process(int len) {
	ipc_msg.value.data[0] = SYSTEM_START_COMMAND;
	strncpy(&ipc_msg.value.buffer[sizeof(unsigned int) * 1], command, len);

	unsigned int ipc = ipc_syscall(PROCESS_SYSTEM, IPC_SENDREC, &ipc_msg);

	if (ipc == IPC_OK && ipc_msg.value.data[0] == SYSTEM_OK) {
		p = ipc_msg.value.data[1];
	}
}

void tty_loop(void) {
	if (p != PROCESS_INVALID_ID) {
		int ipc = ipc_syscall(p, IPC_RECEIVE_ASYNC, &ipc_msg);
		if (ipc == IPC_OK) {
			while (ipc_msg.size > 0) {
				int size = (ipc_msg.size < sizeof(msg)) ? ipc_msg.size : sizeof(msg);
				memcpy(msg.buffer, ipc_msg.value.buffer, size);
				os_write(&handle, &msg, size);

				ipc_msg.size -= size;
			}
		}
	}

	int r = os_read(&handle, &msg, sizeof(msg));
	if (r > 0) {
		if (p != PROCESS_INVALID_ID) {
			memcpy(ipc_msg.value.buffer, msg.buffer, r);
			int ipc = ipc_syscall(p, IPC_SEND, &ipc_msg);
			if (ipc == IPC_DEAD) {
				p = PROCESS_INVALID_ID;
			}
		}
		/* not a else because the process could be gone */
		if (p == PROCESS_INVALID_ID) {
			/* echo */
			int i;
			int out = (next >= (sizeof(command) - 1) ? 0 : -1);
			for (i = 0; i < r; i++) {
				if (msg.buffer[i] == '\b') {
					next -= 1;
				} else if (msg.buffer[i] == '\r') {
					msg.buffer[i + 1] = '\n';
					r = i + 1;

					if (next < (sizeof(command) - 1)) {
						command[next] = '\0';
					} else {
						command[sizeof(command) - 1] = '\0';
					}

					os_write(&handle, &msg, i + 1);
					r = 0;

					start_process(next + 1);
					break;
				} else {
					if (next < (sizeof(command) - 1)) {
						command[next] = msg.buffer[i];
						next++;
					} else if (out == -1) {
						out = next;
					}
				}
			}

			if (out == -1) {
				out = r;
			}

			if (out > 0) {
				os_write(&handle, &msg, out);
			}
		}
	}
}

void main(int argc, char* argv[]) {
	if (argc != 2) {
		return;
	}

	Device_t device = strtol(argv[1], NULL, 10);

	msg.data[0] = 0x1A;	//kbaud divider
	msg.data[1] = 0x0;		//2 stop bit
	msg.data[2] = 0x3;		//data length 8
	msg.data[3] = 0x0;		//no parity
	int o = os_open(device, &msg, 16, &handle, DRIVER_MODE_READ_WRITE);
	if (o != 0) {
		return;
	}

	p = PROCESS_INVALID_ID;

	char* hi = "\r\n\r\nThis is OS!\r\n";
	int len = strlen(hi);
	memcpy(msg.buffer, hi, len);
	os_write(&handle, &msg, len);

	next = 0;

	while (1) tty_loop();
}
