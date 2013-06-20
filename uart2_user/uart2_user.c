/*
 * uart2_user.c
 *
 *  Created on: 17.06.2013
 *      Author: edi
 */
#include <os_stdio.h>
#include <os_std.h>

void main(void) {

	driver_msg_t msg;
	msg.data[0] = 0x001A;	//kbaud divider
	msg.data[1] = 0x0;		//2 stop bit
	msg.data[2] = 0x3;		//data length 8
	msg.data[3] = 0x0;		//no parity

	handle_t handle;
	os_open(23, &msg, 16, &handle, DRIVER_MODE_WRITE);

	char* txt = "1 second elapsed\r\n";
	int len = strlen(txt);
	memcpy(msg.buffer, txt, len);

	while(1) {
		os_write(&handle, &msg, len);
		sleep(1000);
	}

}


