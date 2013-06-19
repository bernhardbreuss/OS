/*
 * uart2_user.c
 *
 *  Created on: 17.06.2013
 *      Author: edi
 */
#include <os_stdio.h>

void main(void) {

	driver_msg_t msg;
	msg.data[0] = 0x000C;	//kbaud divider
	msg.data[1] = 0x1;		//2 stop bit
	msg.data[2] = 0x3;		//data length 8
	msg.data[3] = 0x0;		//no parity

	handle_t handle;
	int owner_id = os_open(22, &msg, 16, &handle, DRIVER_MODE_WRITE);

	while(1) {
		msg.buffer[0] = 'a';
		os_write(&handle, &msg, 1);
	}

}


