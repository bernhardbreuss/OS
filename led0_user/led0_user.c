/*
 * led1_user.c
 *
 *  Created on: May 31, 2013
 *      Author: Bernhard
 */

#include <os_stdio.h>

int main(int argc, char* argv[]) {
	int pin = 21;
	int rate = 450000;

	if (argc == 3) {
		pin = strtol(argv[1], NULL, 10);
		rate = strtol(argv[2], NULL, 10);
	}
	int i;

	driver_msg_t msg;
	msg.data[0] = pin; /* pin 21 of GPIO 5 */
	handle_t handle;
	os_open(5, &msg, 4, &handle, DRIVER_MODE_WRITE);

	msg.data[0] = 0x3; /* toggle */
	while (1) {
		for(i = 0; i < rate; i++);

		os_write(&handle, &msg, 4);
	}
}
