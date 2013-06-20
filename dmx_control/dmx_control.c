/*
 * main.c
 */

#include <dmx_control.h>
#include <os_stdio.h>
#include <os_std.h>
#include <stdio.h>

#define PAN_MAX  			180
#define TILT_MAX			90

int main(int argc, char* argv[]) {

	uint8_t moving_head1[7];
	moving_head1[0] = 0x0;			//listing channel
	moving_head1[1] = 0x50;
	moving_head1[2] = 0x50;
	moving_head1[3] = 0x15;
	moving_head1[4] = 0x4;
	moving_head1[5] = 0x16;
	moving_head1[6] = 0x65;

	dmx_control_moving_head_array(moving_head1);

	handle_t handle;
	driver_msg_t msg;
	os_open(31, &msg, 0, &handle, DRIVER_MODE_WRITE);

	unsigned int current_tild = 0x40;
	unsigned int current_pan = 0x40;
	char received_c = -1;

	msg.buffer[0] = 0x0;
	msg.buffer[1] = current_tild;		//pan (neigen)
	msg.buffer[2] = current_pan;		//tilt (neigen)
	msg.buffer[3] = 0x96; 		//feineinstellung drehung
	msg.buffer[4] = 0x96;		//feineinstellung der neigung
	msg.buffer[5] = 0x20;		//geschwindigkeit der pan und tild bewegung
	msg.buffer[6] = 0x32;		//farbrad
	msg.buffer[7] = 0xB4;		//shutter
	msg.buffer[8] = 0x96;		//dimmer 0 bis 100
	msg.buffer[9] = 0x23;		//gobo rad
	msg.buffer[10] = 0xD;		//gobo rotation
	msg.buffer[11] = 0x10;		//besonderer funktion lutschi lutschi
	msg.buffer[12] = 0x7;		//eingebaute programme

	msg.buffer[13] = 0x2;
	msg.buffer[14] = 0x0;
	msg.buffer[15] = 0xFF;
	msg.buffer[16] = 0x0;
	msg.buffer[17] = 0x0;

	char tilt_up = 0x1;
	char pan_right = 0x1;
	while(1) {

		if(tilt_up == 0x1 && msg.buffer[2] < TILT_MAX) {
			msg.buffer[2]++;
		} else if(msg.buffer[2] > 0){
			msg.buffer[2]--;
			tilt_up = 0x0;
		} else {
			tilt_up = 0x1;
		}

		if(pan_right == 0x1 && msg.buffer[2] < PAN_MAX) {
			msg.buffer[1]++;
		} else if(msg.buffer[1] > 0) {
			msg.buffer[1]--;
			pan_right = 0x0;
		} else {
			pan_right = 0x1;
		}


//		os_write(&handle, &msg, 13);

		msg.buffer[15] ^= 0xFF;
		msg.buffer[16] ^= 0xFF;

		os_write(&handle, &msg, 18);
		sleep(2);
	}

	return 0;
}

//char toggle = 0x0;
//
//if(toggle == 0x0) {
//	msg.buffer[1] = 0x2D;
//	msg.buffer[2] = 0x20;
//	toggle = 0x1;
//} else {
//	msg.buffer[1] = 0x50;
//	msg.buffer[2] = 0x5A;
//	toggle = 0x0;
//}

//received_c = (char) getchar();
//switch(received_c) {
//	case 's':
//		if(msg.buffer[2] > 0) {
//			msg.buffer[2]--;
//		}
//		break;
//	case 'w':
//		if(msg.buffer[2] < TILT_MAX) {
//			msg.buffer[2]++;
//		}
//		break;
//	case 'a' :
//		if(msg.buffer[1] > 0) {
//			msg.buffer[1]--;
//		}
//		break;
//	case 'd' :
//		if(msg.buffer[1] < PAN_MAX) {
//			msg.buffer[1]++;
//		}
//		break;
//	default:
//		//msg.buffer[6] += 5;
//		printf("I don't understand you!\r\n");
//		break;
//}
