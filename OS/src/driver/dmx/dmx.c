/*
 * dmx.c
 *
 *  Created on: 07.06.2013
 *      Author: edi
 */

#include "dmx.h"
#include "../../hal/omap3530/uart/uart.h"
#include "../../service/logger/logger.h"
#include "../../bit.h"

uart_t uart2;

static void dmx_uart_set_send_mode(void);
static void dmx_uart_set_reset_mode(void);
static void dmx_send_reset(void);

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

	dmx_uart_set_send_mode();
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

void dmx_send_continously(uint8_t* channels, int number_of_channels) {

	while(1) {

		//take account of Reset-Reset gap
		int i;
		for(i = 0; i < 2000; i++);
		dmx_send_reset();
		for(i = 0; i < 2000; i++);
		dmx_uart_set_send_mode();

		dmx_send(channels, number_of_channels);
	}
}

// STATIC Methods START ----------------------------------------------------------------

/*
 * Set Modes as stated in BBSRM_latest.pdf on page 96 to use
 * Expansion-Board pin 6 with UART2.
 */
static void dmx_uart_set_send_mode(void) {

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
static void dmx_uart_set_reset_mode(void) {

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
static void dmx_send_reset(void) {

	dmx_uart_set_reset_mode();

	*(DMX_GPIO5_DATAOUT) &= ~(1 << 18); //send low
	int i, a;
	a = 240; //~204 microseconds during testing
	for(i = 0; i < a; i++);

	a = 10; //~10 microseconds
	*(DMX_GPIO5_DATAOUT) |= (1 << 18);
	for(i = 0; i < a; i++);
}

// STATIC Methods END ----------------------------------------------------------------

//	dmx_init();
//	uint8_t channels[256];
//	channels[0] = 0x0;
//	channels[1] = 0x2;
//	channels[2] = 0x0E;
//	channels[3] = 0xCD;
//	channels[4] = 0xEB;
//	channels[5] = 0x0;
//	dmx_send_continously(&channels[0], 6);
