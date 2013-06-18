/*
 * uart2_user.c
 *
 *  Created on: 17.06.2013
 *      Author: edi
 */
#include <osstd.h>

void main(void) {

	driver_msg_t msg;
	msg.data[0] = 0x16;
	ioctl(22, &msg, 8);

	msg.data[0] = 0x000C;
	msg.data[1] = 0x1;
	msg.data[2] = 0x3;
	msg.data[3] = 0x0;
	handle_t handle;
	int owner_id = open(22, &msg, 16, &handle, DRIVER_MODE_WRITE);

	int i;
	while(1) {
		for(i = 0; i < 450000; i++);
		msg.buffer[0] = 'a';
		write((void*)&owner_id, &msg, 1);
	}

}


