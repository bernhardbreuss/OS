/*
 * dmx.h
 *
 *  Created on: Jun 19, 2013
 *      Author: Patrick
 */

#ifndef DMX_CONTROL_H_
#define DMX_CONTROL_H_

#include "stdint.h"
#include "driver.h"
#include "os_stdio.h"

void dmx_init();

void dmx_control_moving_head_array(uint8_t moving_head_channels[7]);

void dmx_control_moving_head(uint16_t listening_channel, uint8_t pan,
		uint8_t tilt, uint8_t color_wheel, uint8_t shutter, uint8_t gobo_wheel,
		uint8_t gobo_rotation, int channels_length);

void dmx_control_headlight_array(uint8_t moving_head_channels[6]);

void dmx_control_headlights(uint16_t listening_channel, uint8_t mode, uint8_t red,
		uint8_t green, uint8_t blue, uint8_t speed, int channels_length);

int dmx_assemble_channels(uint16_t listening_channel, uint8_t channels[], int channels_length);

void dmx_insert_channels(uint16_t listening_channel, uint8_t channel_stream[], uint8_t channels[], int channels_length);

int dmx_test_print_channels(int length, uint8_t channels[]);

void dmx_controller_send_channels(void);

#endif /* DMX_CONTROL_H_ */
