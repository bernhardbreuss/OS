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
	if (size < sizeof(unsigned int)) {
		return NULL;
	}
	unsigned int values[1];
	unsigned int addresses[1];
	values[0] = GPIO5_OE;
	unsigned int oe = memory_mapped_read(values, 1);

	switch (mode) {
	case DRIVER_MODE_READ:
		values[0] |= (1 << buf->data[0]);
		break;
	case DRIVER_MODE_WRITE:
		values[0] &= ~(1 << buf->data[0]);
		break;
	default:
		return NULL;
	}
	addresses[0] = GPIO5_OE;
	memory_mapped_write(values, addresses, 1);

	return (void*)buf->data[0];
}

static int gpio_read(void* handle, driver_msg_t* buf, size_t size) {
	unsigned int pin = ((unsigned int)handle % 32);
	unsigned int addresses[1];
	addresses[0] = GPIO5_OE;
	unsigned int value = memory_mapped_read(addresses, 1);

	//TODO: buf-> data check for size
	buf->data[0] = (value & (1 << pin)) >> pin;

	return 1;
}

static int gpio_write(void* handle, driver_msg_t* buf, size_t size) {
	unsigned int pin = ((unsigned int)handle % 32);
	unsigned int values[1];
	unsigned int addresses[1];

	values[0] = GPIO5_DATAOUT;
	memory_mapped_read(values, 1);

	switch (buf->data[0]) {
	case GPIO_ON:
		values[0] |= (1 << pin);
		break;
	case GPIO_OFF:
		values[0] &= ~(1 << pin);
		break;
	case GPIO_TOGGLE:
		values[0] ^= (1 << pin);
		break;
	default:
		return -1;
	}

	addresses[0] = GPIO5_DATAOUT;

	memory_mapped_write(values, addresses, 1);
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
