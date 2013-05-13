/*
 * gpio.c
 *
 *  Created on: May 9, 2013
 *      Author: Bernhard
 */

#include "gpio.h"
#include "../../kernel/process.h"
#include "../../kernel/process_manager.h"
#include "../../devices/device.h"
#include "../../kernel/ipc/ipc.h"
#include "../../service/logger/logger.h"
#include <stdlib.h>
#include <stdio.h>


uint32_t gpio_main(void) {
	message_t msg;

	while (1) {
		if (ipc_syscall(PROCESS_ANY, IPC_RECEIVE, &msg) == IPC_OK) {
			if (msg.type == MESSAGE_TYPE_DATA) { /* TODO: is msg->type really necessary? */
				switch (msg.value.data[0]) {
				case DEVICE_OPEN:
					/* TODO: rewrite for generic GPIO (needs MMU) */
					*(GPIO5_OE) &= ~(1 << msg.value.data[1]); /* TODO: should be done through kernel (IPC?) */
				case DEVICE_WRITE: /* DEVICE_OPEN is falling through */
					if (msg.value.data[2] == GPIO_ON) { /* on */
						*(GPIO5_DATAOUT) |= (1 << msg.value.data[1]);
					} else if (msg.value.data[2] == GPIO_TOGGLE) { /* toggle */
						*(GPIO5_DATAOUT) ^= (1 << msg.value.data[1]);
					} else if (msg.value.data[2] == GPIO_OFF) { /* off */
						*(GPIO5_DATAOUT) &= ~(1 << msg.value.data[1]);
					}

					break;

					/* TODO: read, close, ... in an abstract way, so that not every driver has to implement it */
				}
			}

			ipc_syscall(msg.source, IPC_SEND, &msg); /* TODO: error handling */
		} else {
			logger_error("GPIO driver gone");
			while (1) ;
		}
	}

	return 0;
}

static Process_t gpio_process;
ProcessId_t gpio_start_driver_process(Device_t device) {
	gpio_process.func = &gpio_main;
	gpio_process.priority = MEDIUM;
	gpio_process.state = PROCESS_READY;
	gpio_process.name = malloc(10);
	if (gpio_process.name == NULL) {
		gpio_process.name = "GPIO driver";
	} else {
		sprintf(gpio_process.name, "GPIO #%u", device);
	}

	return process_manager_add_process(&gpio_process);
}

uint32_t gpio_init(Device_t device) {
	/* TODO: tell OS that this driver is using GPIO */

	/* TODO: notify new process so that he knows the device */

	return 0;
}

uint32_t gpio_open(message_t* msg) {
	return 0;
}

uint32_t gpio_close(message_t* msg) {

	return 0;
}

uint32_t gpio_read(message_t* msg) {

	return 0;
}

uint32_t gpio_write(message_t* msg) {

	return 0;
}
