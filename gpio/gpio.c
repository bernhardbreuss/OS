/*
 * gpio.c
 *
 *  Created on: May 9, 2013
 *      Author: Bernhard
 */

#include "gpio.h"
#include <stdlib.h>
#include <stdio.h>
#include <ipc.h>
#include <mem_io.h>
#include <driver.h>

static void* gpio_open(driver_msg_t* buf, size_t size, driver_mode_t mode) {
	if (size < 4) {
		return NULL;
	}

	unsigned int oe = memory_mapped_read(GPIO5_OE);

	switch (mode) {
	case DRIVER_MODE_READ:
		oe |= (1 << buf->data[0]);
		break;
	case DRIVER_MODE_WRITE:
		oe &= ~(1 << buf->data[0]);
		break;
	default:
		return NULL;
	}

	return (void*)buf->data[0];
}

static int gpio_read(void* handle, driver_msg_t* buf, size_t size) {
	unsigned int pin = ((unsigned int)handle % 32);
	unsigned int value = memory_mapped_read(GPIO5_OE);

	buf->data[0] = (value & (1 << pin)) >> pin;

	return 1;
}

static int gpio_write(void* handle, driver_msg_t* buf, size_t size) {
	unsigned int pin = ((unsigned int)handle % 32);
	unsigned int value = memory_mapped_read(GPIO5_DATAOUT);

	switch (buf->data[0]) {
	case GPIO_ON:
		value |= (1 << pin);
		break;
	case GPIO_OFF:
		value &= ~(1 << pin);
		break;
	case GPIO_TOGGLE:
		value ^= (1 << pin);
		break;
	default:
		return -1;
	}

	memory_mapped_write(value, GPIO5_DATAOUT);
	return 1;
}

Driver_t driver = {
	NULL,
	gpio_open,
	NULL,
	gpio_read,
	gpio_write
};

void main(void) {
	driver_init();
}
