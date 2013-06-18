/*
 * led1_user.c
 *
 *  Created on: May 31, 2013
 *      Author: Bernhard
 */

#include <os_stdio.h>

void main(void) {
	int i;

	driver_msg_t msg;
	msg.data[0] = 22; /* pin 22 of GPIO 5 */
	handle_t handle;
	os_open(5, &msg, 4, &handle, DRIVER_MODE_WRITE);

	msg.data[0] = 0x3; /* toggle */
	while (1) {
		for(i = 0; i < 900000; i++);

		os_write(&handle, &msg, 4);
	}

}
