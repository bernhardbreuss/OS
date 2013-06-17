/*
 * led1_user.c
 *
 *  Created on: May 31, 2013
 *      Author: Bernhard
 */

#include <osstd.h>

void main(void) {
	int i;

	driver_msg_t msg;
	msg.data[0] = 21; /* pin 21 of GPIO 5 */
	handle_t handle;
	open(5, &msg, 4, &handle, DRIVER_MODE_WRITE);

	msg.data[0] = 0x3; /* toggle */
	while (1) {
		for(i = 0; i < 450000; i++);

		write(&handle, &msg, 4);
	}
}
