/*
 * init.c
 *
 *  Created on: May 9, 2013
 *      Author: Bernhard
 */

#include "driver_manager.h"
#include "../hal/platform.h"
#include "../devices/device.h"
#include "../util/linked_list.h"
#include <stdlib.h>
#include <string.h>


typedef struct {
	Device_t device;
	Driver_t* driver;
	ProcessId_t process_id;
	ProcessId_t (*start_process)(Device_t device); /* later this should be the path to load the driver from fs and a IPC call to process manager to start that process */
} device_driver_map;

static linked_list_t device_list; /* already mapped drivers */
static linked_list_t drivers; /* all available drivers */

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
		//TODO: really can return NULL here?
		return INVALID_PROCESS_ID;
	}

	linked_list_add(&device_list, mapping);
	mapping->process_id = mapping->start_process(device);
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

/* TODO: driver manager as own process */
