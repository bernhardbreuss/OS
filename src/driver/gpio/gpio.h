/*
 * gpio.h
 *
 *  Created on: May 9, 2013
 *      Author: Bernhard
 */

#ifndef GPIO_H_
#define GPIO_H_

#include <inttypes.h>
#include "../driver.h"


unsigned int* GPIO5_OE = (unsigned int*)0x49056034;
unsigned int* GPIO5_DATAOUT = (unsigned int*)0x4905603C;

#define GPIO_ON		0x1	/* TODO: move to good place */
#define GPIO_TOGGLE	0x2
#define GPIO_OFF	0x3

ProcessId_t gpio_start_driver_process(Device_t device);

uint32_t gpio_main(void); /* TODO: remove when proces loading from fs works */

uint32_t gpio_init(Device_t device);
uint32_t gpio_open(message_t* msg);
uint32_t gpio_close(message_t* msg);
uint32_t gpio_read(message_t* msg);
uint32_t gpio_write(message_t* msg);

Driver_t gpio_driver = {
	"/dev/gpio",
	gpio_init,
	NULL,
	gpio_open,
	gpio_close,
	gpio_read,
	gpio_write
};

#endif /* GPIO_H_ */
