/*
 * init.c
 *
 *  Created on: May 9, 2013
 *      Author: Bernhard
 */

#include "driver_manager.h"
#include <device.h>
#include <linked_list.h>
#include <stdlib.h>
#include <string.h>
#include <ipc.h>
#include <kernel.h>

typedef struct {
	Device_t device;
	Driver_t* driver;
	ProcessId_t process_id;
	ProcessId_t (*start_process)(Device_t device); /* later this should be the path to load the driver from fs and a IPC call to process manager to start that process */
	driver_type_t type;
	char* driver_path;
	process_func main_func;
	unsigned int virtual_address;
	unsigned int physical_address;
	unsigned int size;
	char* name;
} device_driver_map;

static linked_list_t device_list; /* already mapped drivers */
static linked_list_t drivers; /* all available drivers */

static message_t msg;

void driver_manager_init(void) {
	linked_list_init(&device_list);
	linked_list_init(&drivers);
}

void driver_manager_add_driver(Device_t device, Driver_t* driver, ProcessId_t (*start_process)(Device_t device)) {
	device_driver_map* mapping = malloc(sizeof(device_driver_map));

	if (mapping == NULL) {
		/* TODO: oO */
		return;
	}

	mapping->device = device;
	mapping->driver = driver;
	mapping->start_process = start_process;

	linked_list_add(&drivers, mapping);
}

device_driver_map* get_driver(Device_t device) {
	device_driver_map* mapping;
	linked_list_node_t* node = drivers.head;

	while (node != NULL) {
		mapping = node->value;
		if (mapping->device == device) {
			return mapping;
		}
	}

	return NULL;
}

ProcessId_t start_driver(Device_t device) {
	device_driver_map* mapping = get_driver(device);
	if (mapping == NULL) {
		return INVALID_PROCESS_ID;
	}

	linked_list_add(&device_list, mapping);

	msg.value.data[0] = KERNEL_START_PROCESS;
	if (mapping->type == driver_type_intern) {
		msg.value.data[1] = KERNEL_START_PROCESS_BYFUNC;
		msg.value.data[2] = (unsigned int)mapping->main_func;
		msg.value.data[3] = mapping->virtual_address;
		msg.value.data[4] = mapping->physical_address;
		msg.value.data[5] = mapping->size;
		memcpy(&msg.value.data[6], mapping->name, strlen(mapping->name));
	} else {
		msg.value.data[1] = KERNEL_START_PROCESS_BYPATH;
		memcpy(&msg.value.data[2], mapping->driver_path, strlen(mapping->driver_path));
	}

	ipc_syscall(PROCESS_KERNEL, IPC_SENDREC, &msg);

	mapping->process_id = msg.value.data[1];
	return mapping->process_id;
}

ProcessId_t driver_manager_get_process(Device_t device) {
	device_driver_map* mapping;
	linked_list_node_t* node = device_list.head;

	while (node != NULL) {
		mapping = node->value;
		if (mapping->device == device) {
			return mapping->process_id;
		}
	}

	return start_driver(device);
}

void main(void) {
	driver_manager_init();
}

/* TODO: driver manager as own process */
