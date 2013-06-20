/*
 * dmx.c
 *
 *  Created on: Jun 19, 2013
 *      Author: Patrick
 */

#include "dmx_control.h"

static uint8_t channel_stream[512];

void dmx_init(){
}

/*
 * http://images4.thomann.de/pics/atg/atgdata/document/manual/c_238185_de_online.pdf
 * page 42ff
 *
 * first value of the array: channel of the moving head
 * next values:
 * channel	value	function
 * 1		0..255	pan (Drehung)
 * 2				tilt (Neigung)
 * 3		0..255	color_wheel
 * 4		0..255	shutter
 * 5		0..255	gobo_wheel
 * 6		0..255	gobo_rotation
 *
 */
void dmx_control_moving_head_array(uint8_t moving_head_channels[7]) {

	dmx_control_moving_head(
			moving_head_channels[0], //listening_channel
			moving_head_channels[1], //pan
			moving_head_channels[2], //tilt
			moving_head_channels[3], //color_wheel
			moving_head_channels[4], //shutter
			moving_head_channels[5], //gobo_wheel
			moving_head_channels[6], //gobo rotation
			6); //channels_length
}

/*
 * function with 6-channel-DMX-mode - moving head.pdf page 42ff
 */
void dmx_control_moving_head(uint16_t listening_channel, uint8_t pan, uint8_t tilt,
		uint8_t color_wheel, uint8_t shutter, uint8_t gobo_wheel,
		uint8_t gobo_rotation, int channels_length) {

	uint8_t channels[6];
	channels[0] = pan;
	channels[1] = tilt;
	channels[2] = color_wheel;
	channels[3] = shutter;
	channels[4] = gobo_wheel;
	channels[5] = gobo_rotation;

	dmx_assemble_channels(listening_channel, channels, channels_length);

}



/*
 * https://ilias.fhv.at/ilias.php?ref_id=62965&cmd=sendfile&cmdClass=ilrepositorygui&cmdNode=4s&baseClass=ilRepositoryGUI
 * page 42ff
 *
 * first value of the array: channel of the headlight
 * next values:
 * channel	value	function
 * 1		0..255	mode RGB (0-63, fading (64-255)
 * 2		0..255	red
 * 3		0..255	green
 * 4		0..255	blue
 * 6		0..255	speed if channel 1 is set "fading" 64-255
 *
 */
void dmx_control_headlight_array(uint8_t moving_head_channels[6]) {

	dmx_control_headlights(
			moving_head_channels[0], //listening_channel
			moving_head_channels[1], //mode
			moving_head_channels[2], //red
			moving_head_channels[3], //green
			moving_head_channels[4], //blue
			moving_head_channels[5], //speed
			5); //channels_length
}

/*
 * function with 5-channel-DMX-mode - par scheinwerfer.pdf page 42ff
 */
void dmx_control_headlights(uint16_t listening_channel, uint8_t mode, uint8_t red,
		uint8_t green, uint8_t blue, uint8_t speed, int channels_length) {

	uint8_t channels[5];
	channels[0] = mode;
	channels[1] = red;
	channels[2] = green;
	channels[3] = blue;
	channels[4] = speed;

	dmx_assemble_channels(listening_channel, channels, channels_length);

}

/*
 * assembles all channels to an array[512]
 */
int dmx_assemble_channels(uint16_t listening_channel, uint8_t channels[], int channels_length) {

	if (listening_channel == 0 || listening_channel > 512) {
		return -1;
	}

	channel_stream[0] = 0x0; //set start byte

	dmx_insert_channels(listening_channel, channel_stream, channels, channels_length);

	//dmx_controller_send_channels();
	return 0;
}

void dmx_insert_channels(uint16_t listening_channel, uint8_t channel_stream[],
		uint8_t channels[], int channels_length) {

	uint16_t i = listening_channel;
	int j = 0;
	while (j < channels_length && i < 512) {
		channel_stream[i] = channels[j];
		 i++;
		 j++;
	}

}

int dmx_test_print_channels(int length, uint8_t channel_stream[]) {

	printf("Startbyte: - %d\n", channel_stream[0]);
	int i = 1;
	/* display using a pointer arthimatic */
	for (; i < length; i++) {
		printf("Channel: %d - %d\n", i, channel_stream[i]);
	}
	return 0;
}

void dmx_controller_send_channels(void) {
//	handle_t handle;
//	driver_msg_t msg;
//	os_open(31, &msg, 0, &handle, DRIVER_MODE_WRITE);
//
//	memcpy(msg.buffer, channel_stream, sizeof(channel_stream));
//
//	while(1) {
//		os_write(&handle, &msg, 7);
//	}
}

