/*
 * main.c
 */
#include <os_std.h>
#include <os_stdio.h>

int main(int argc, char* argv[]) {

	int channels_to_send = 0x5;

	if(argc >= 2) {
		//channels_to_send = strtol(argv[1], NULL, 10);
	}
	
	driver_msg_t msg;

	handle_t handle;
	os_open(31, &msg, 0, &handle, DRIVER_MODE_WRITE);

	msg.buffer[0] = 0x0;	 //start byte
	msg.buffer[1] = 0x2;
	msg.buffer[2] = 0x0;	 //red
	msg.buffer[3] = 0x0;  //green
	msg.buffer[4] = 0xFB;
	msg.buffer[5] = 0x0;

	while(1) {
		os_write(&handle, &msg, channels_to_send + 1);

		msg.buffer[3] ^= 0xFF;

		sleep(1000);
	}


	return 0;
}
