/*
 * dmx.c
 *
 *  Created on: 07.06.2013
 *      Author: edi
 */

#include "dmx.h"
#include "../../hal/omap3530/uart/uart.h"
#include "../../service/logger/logger.h"
#include <bit.h>

uart_t uart2;

void dmx_init(void){

	logger_debug("Initialize DMX ...");

	uart_get(2, &uart2);
	uart_protocol_format_t protocol;
	protocol.baudrate = 0x000C; //250 kbaud
	protocol.stopbit = 0x1;		//2 stop bit
	protocol.datalen = 0x3;		//length 8
	protocol.use_parity = 0x0;  // no parity
	uart_init_for_dmx(&uart2, 0x0, protocol);

	//Configure RS485 DE (driver output) -----------------------
	//@see CIRCIUT.pdf
	//extension board pin 11, set mode for GPIO
	unsigned int pin_11 = *(SCM_CONTROL_PADCONF_MMC2_DAT2);
	//mode 4 is GPIO
	pin_11 &= ~0x70000;
	pin_11 |= (SCM_MODE_4 << 16);

	//GPIO 135 enable output (OE)
	*(DMX_GPIO5_OE) &= ~0x80; //set bit 7 to low to enable

	//GPIO 135 set data out to high
	*(DMX_GPIO5_DATAOUT) |= 0x80; 	//set bit 7 to high, to enable "Driver output" on RS485
	//*(DMX_GPIO5_DATAOUT) &= 0x80; //disable driver output

	//Configure RS485 DE (driver output) END ---------------------

	//Configure GPIO_146 as output to be able to send resets with it
	*(DMX_GPIO5_OE) &= ~(1 << 18);
}


void dmx_send(uint8_t* channels, int number_of_channels) {
		int i = 0;

		for (; i < number_of_channels; i++, channels++) {
			while (!uart_is_empty_write_queue(&uart2))
				;
			uart_write_uint8_t(&uart2, channels);
		}

		return;
}

void dmx_send_complete_color_spektrum(int time_in_millis, Color_t color, DMX_Colorspektrum_Flow_t flow) {

	int sleep_per_color = time_in_millis / 256;
	if(sleep_per_color < 2) {
		sleep_per_color = 2;
	}

	int i;
	uint8_t channels[6];
	channels[0] = 0x0;	//start byte
	channels[1] = 0x2; 	//set to RGB
	channels[2] = 0x0;  //red
	channels[3] = 0x0;  //green
	channels[4] = 0x0;  //blue
	channels[5] = 0x0;	//no function - no speed
	for(i = 0; i <= 255; i++) {

		dmx_sleep(sleep_per_color);
		dmx_uart_set_reset_mode();
		dmx_send_reset();
		dmx_uart_set_send_mode();
		channels[color] = i * flow;
		dmx_send(&channels[0], 6);
	}
}

void dmx_send_color_fade(DMX_Fade_t fade_mode, DMX_Fade_Speed_t speed) {

	uint8_t channels[6];
	channels[0] = 0x0; //start byte
	channels[1] = fade_mode;
	channels[2] = 0x0;	//red
	channels[3] = 0x0;  //green
	channels[4] = 0x0;  //blue
	channels[5] = speed; //speed

	dmx_send(channels,6);
}

/*
 * Set Modes as stated in BBSRM_latest.pdf on page 96 to use
 * Expansion-Board pin 6 with UART2.
 */
void dmx_uart_set_send_mode(void) {

	//Omap3530x.pdf, modes on page 780
	//Chosen modes: page 96 in BBSRM_latest.pdf
	unsigned int scm_rx_tx = *(SCM_CONTROL_PADCONF_UART2_TX);
	scm_rx_tx &= ~0x70007;
	scm_rx_tx |= SCM_MODE_0;
	scm_rx_tx |= (SCM_MODE_1 << 16);
	*(SCM_CONTROL_PADCONF_UART2_TX) = scm_rx_tx;

	unsigned int scm_rts_cts = *(SCM_CONTROL_PADCONF_UART2_CTS);
	scm_rts_cts &= ~0x70007;
	scm_rts_cts |= SCM_MODE_0;
	scm_rts_cts |= (SCM_MODE_0 << 16);
	*(SCM_CONTROL_PADCONF_UART2_CTS) = scm_rts_cts;
}

/**
 * Set Modes as stated in BBSRM_latest.pdf on page 96 to use
 * Expansion-Board pin 6 with GPIO_146.
 */
void dmx_uart_set_reset_mode(void) {

	//Omap3530x.pdf, modes on page 780
	//Chosen modes: page 96 in BBSRM_latest.pdf
	unsigned int scm_rx_tx = *(SCM_CONTROL_PADCONF_UART2_TX);
	scm_rx_tx &= ~0x7;
	scm_rx_tx |= SCM_MODE_4;
	*(SCM_CONTROL_PADCONF_UART2_TX) = scm_rx_tx;
}

/**
 * Reset: Sending at least 22 bits low (=break) (88 microseconds)
 * then send mark after break (2 bits high) (4 microseconds)
 */
void dmx_send_reset(void) {

	*(DMX_GPIO5_DATAOUT) &= ~(1 << 18); //send low
	int i, a;
	a = 240; //~204 microseconds during testing
	for(i = 0; i < a; i++);

	a = 10; //~10 microseconds
	*(DMX_GPIO5_DATAOUT) |= (1 << 18);
	for(i = 0; i < a; i++);
}

void dmx_sleep(int milliseconds) {
	int i;
	int microseconds = milliseconds * 1000;
	for(i = 0; i < microseconds; i++);
}

//dmx_init();
//dmx_uart_set_reset_mode();
//dmx_send_reset();
//dmx_uart_set_send_mode();
//
//uint8_t channels[256];
//channels[0] = 0x0;
//channels[1] = 0x2;
//channels[2] = 0x0;
//channels[3] = 0xB0;
//channels[4] = 0x0;
//channels[5] = 0x0;
//
//dmx_send(&channels[0], 6);
//
//dmx_sleep(2);
//dmx_uart_set_reset_mode();
//dmx_send_reset();
//
//dmx_uart_set_send_mode();
//channels[3] = 0x0;
//channels[4] = 0xFA;
//dmx_send(&channels[0], 6);
//
//int i = 0;
//while(i < 3000) {
//	i++;
//	dmx_sleep(2);
//	dmx_uart_set_reset_mode();
//	dmx_send_reset();
//	dmx_uart_set_send_mode();
//	dmx_send_color_fade(DMX_FADE_7_MODE_2, DMX_FADE_LOW_SPEED);
//}
//i = 0;
//while(i < 3000) {
//	i++;
//	dmx_sleep(2);
//	dmx_uart_set_reset_mode();
//	dmx_send_reset();
//	dmx_uart_set_send_mode();
//	dmx_send_color_fade(DMX_FADE_3, DMX_FADE_HIGH_SPEED);
//}
//
//int millis = 8000;
//dmx_send_complete_color_spektrum(millis, RED, DMX_COLORSPEC_INTENSIFY);
//dmx_send_complete_color_spektrum(millis, RED, DMX_COLORSPEC_SOFTEN);
//
//dmx_send_complete_color_spektrum(millis, GREEN, DMX_COLORSPEC_INTENSIFY);
//dmx_send_complete_color_spektrum(millis, GREEN, DMX_COLORSPEC_SOFTEN);
//
//dmx_send_complete_color_spektrum(millis, BLUE, DMX_COLORSPEC_INTENSIFY);
//dmx_send_complete_color_spektrum(millis, BLUE, DMX_COLORSPEC_SOFTEN);
