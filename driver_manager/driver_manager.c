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
#include <kernel.h>
#include <process.h>

typedef struct {
	Device_t device;
	void* driver_binary;
	process_name_t name;
	ProcessId_t process_id;
} device_driver_map;

static linked_list_t device_list; /* already mapped drivers */
static linked_list_t drivers; /* all available drivers */

static message_t msg;

static void driver_manager_init(void) {
	linked_list_init(&device_list);
	linked_list_init(&drivers);
}

static void driver_manager_add_driver(Device_t device, void* driver_binary, process_name_t name) {
	device_driver_map* mapping = malloc(sizeof(device_driver_map));

	if (mapping == NULL) {
		/* TODO: handle too less memory */
		return;
	}

	mapping->device = device;
	mapping->driver_binary = driver_binary;
	mapping->name = name;

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
		return INVALID_PROCESS_ID;
	}

	msg.value.data[0] = KERNEL_START_PROCESS;
	msg.value.data[1] = (unsigned int)mapping->driver_binary;
	strncpy(&msg.value.buffer[sizeof(unsigned int) * 2], mapping->name, sizeof(PROCESS_MAX_NAME_LENGTH));

	unsigned int ipc = ipc_syscall(PROCESS_KERNEL, IPC_SENDREC, &msg);

	if (ipc != IPC_OK || msg.value.data[0] != KERNEL_OK) {
		return INVALID_PROCESS_ID;
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
	}

	return driver_manager_start_driver(device);
}

static message_t msg;

void main(void) {
	driver_manager_init();
	process_name_t name;
	ProcessId_t source_pid;

	while (1) {
		unsigned int ipc = ipc_syscall(PROCESS_ANY, IPC_RECEIVE, &msg);
		if (ipc == IPC_OK) {
			source_pid = msg.source;
			int return_value = MESSAGE_UNKNOWN;

			switch (msg.value.data[0]) {
			case DRIVER_MANAGER_ADD:
				name = malloc(PROCESS_MAX_NAME_LENGTH);
				if (name != NULL) {
					strncpy(name, &msg.value.buffer[12], sizeof(PROCESS_MAX_NAME_LENGTH));
				} else {
					name = "/* NO NAME BECAUSE NOT ENOUGH MEMORY */";
				}
				driver_manager_add_driver(msg.value.data[1], (void*)msg.value.data[2], name); /* TODO: add checks */
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
