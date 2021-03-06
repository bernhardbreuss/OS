/*
 * init.c
 *
 *  Created on: May 9, 2013
 *      Author: Bernhard
 */

#include <driver_manager.h>
#include <device.h>
#include <linked_list.h>
#include <stdlib.h>
#include <string.h>
#include <ipc.h>
#include <system.h>
#include <process.h>
#include <argument_helper.h>

typedef struct {
	Device_t device;
	void* driver_binary;
	char command_line[ARGUMENTS_MAX_LENGTH];
	ProcessId_t process_id;
} device_driver_map;

static linked_list_t device_list; /* already mapped drivers */
static linked_list_t drivers; /* all available drivers */

static message_t msg;

static void driver_manager_init(void) {
	linked_list_init(&device_list);
	linked_list_init(&drivers);
}

static void driver_manager_add_driver(Device_t device, void* driver_binary, char* command_line) {
	device_driver_map* mapping = malloc(sizeof(device_driver_map));

	if (mapping == NULL) {
		/* TODO: handle too less memory */
		return;
	}

	mapping->device = device;
	mapping->driver_binary = driver_binary;
	strncpy(mapping->command_line, command_line, sizeof(mapping->command_line));

	linked_list_add(&drivers, mapping); /* TODO: check for too less memory */
}

static device_driver_map* driver_manager_get_driver(Device_t device) {
	device_driver_map* mapping;
	linked_list_node_t* node = drivers.head;

	while (node != NULL) {
		mapping = node->value;
		if (mapping->device == device) {
			return mapping;
		}
		node = node->next;
	}

	return NULL;
}

static ProcessId_t driver_manager_start_driver(Device_t device) {
	device_driver_map* mapping = driver_manager_get_driver(device);
	if (mapping == NULL) {
		return PROCESS_INVALID_ID;
	}

	msg.value.data[0] = SYSTEM_START_PROCESS;
	msg.value.data[1] = (unsigned int)mapping->driver_binary;
	msg.value.data[2] = (unsigned int)PROCESS_INVALID_ID;
	strncpy(&msg.value.buffer[sizeof(unsigned int) * 3], mapping->command_line, sizeof(mapping->command_line));

	unsigned int ipc = ipc_syscall(PROCESS_SYSTEM, IPC_SENDREC, &msg);

	if (ipc != IPC_OK || msg.value.data[0] != SYSTEM_OK) {
		return PROCESS_INVALID_ID;
	}

	mapping->process_id = msg.value.data[1];
	linked_list_add(&device_list, mapping);

	return mapping->process_id;
}

static ProcessId_t driver_manager_get_process(Device_t device) {
	device_driver_map* mapping;
	linked_list_node_t* node = device_list.head;

	while (node != NULL) {
		mapping = node->value;
		if (mapping->device == device) {
			return mapping->process_id;
		}

		node = node->next;
	}

	return driver_manager_start_driver(device);
}

static message_t msg;

int main(int argc, char* argv[]) {
	driver_manager_init();
	ProcessId_t source_pid;

	while (1) {
		unsigned int ipc = ipc_syscall(PROCESS_ANY, IPC_RECEIVE, &msg);
		if (ipc == IPC_OK) {
			source_pid = msg.source;
			int return_value = MESSAGE_UNKNOWN;

			switch (msg.value.data[0]) {
			case DRIVER_MANAGER_ADD:
				driver_manager_add_driver(msg.value.data[1], (void*)msg.value.data[2], &msg.value.buffer[12]); /* TODO: add checks */
				return_value = 1;
				break;
			case DRIVER_MANAGER_GET:
				return_value = driver_manager_get_process(msg.value.data[1]);
				break;
			}

			msg.value.data[0] = return_value;
			ipc_syscall(source_pid, IPC_SEND, &msg);
		}
	}
}
