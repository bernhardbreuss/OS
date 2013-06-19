/*
 * dmx.h
 *
 *  Created on: 07.06.2013
 *      Author: edi
 *
 * Driver Output is configured over GPIO_135 (bit 7 of GPIO5)
 * GPIO_146 (Bit 18 of GPIO5) is used to write to driver input
 * to send resets ("Break" and "Mark after Break")
 */

#ifndef DRIVER_DMX_H_
#define DRIVER_DMX_H_

#include <inttypes.h>

//all mode settings taken from BBSRM_latest.pdf page 96
//CONTROL_PADCONF_UART2_CTS[15:0]			uart2_cts (mode 0)
//CONTROL_PADCONF_UART2_CTS[31:16] 			uart2_rts (mode 0)
//CONTROL_PADCONF_UART2_TX[15:0]			uart2_tx  (mode 0)
//CONTROL_PADCONF_UART2_TX[31:16]			uart2_rx  (mode 1)
#define SCM_CONTROL_PADCONF_UART2_TX 		(volatile unsigned int*) 0x48002178
#define SCM_CONTROL_PADCONF_UART2_CTS 		(volatile unsigned int*) 0x48002174
//CONTROL_PADCONF_MMC2_DAT2[31:16]			gpio_135 (mode 4)
#define SCM_CONTROL_PADCONF_MMC2_DAT2		(volatile unsigned int*) 0x48002160

#define DMX_GPIO5_OE						(volatile unsigned int*) 0x49056034
#define DMX_GPIO5_DATAOUT					(volatile unsigned int*) 0x4905603C

//Omap3530x.pdf page 780
//the multiplexing modes of System Control Module
#define SCM_MODE_0					 		0x0
#define SCM_MODE_1							0x1
#define SCM_MODE_2							0x2
#define SCM_MODE_3							0x3
#define SCM_MODE_4							0x4
#define SCM_MODE_5							0x5
#define SCM_MODE_6							0x6
#define SCM_MODE_7							0x7

typedef enum {
	RED = 2,
	GREEN = 3,
	BLUE = 4
} Color_t;

typedef enum {
	DMX_FADE_7_MODE_1 = 0x64,
	DMX_FADE_7_MODE_2 = 0x96,
	DMX_FADE_3 = 0xDC
} DMX_Fade_t;

typedef enum {
	DMX_FADE_HIGH_SPEED = 0x64,
	DMX_FADE_MED_SPEED = 0x2D,
	DMX_FADE_LOW_SPEED = 0xB
} DMX_Fade_Speed_t;

typedef enum {
	DMX_COLORSPEC_INTENSIFY = -0x1,
	DMX_COLORSPEC_SOFTEN = 0x1
} DMX_Colorspektrum_Flow_t;

void dmx_init(void);
void dmx_uart_set_send_mode(void);
void dmx_uart_set_reset_mode(void);
void dmx_send_reset(void);
void dmx_sleep(int milliseconds);
void dmx_send(uint8_t* channels, int number_of_channels);
void dmx_send_complete_color_spektrum(int time_in_millis, Color_t color, DMX_Colorspektrum_Flow_t flow);
void dmx_send_color_fade(DMX_Fade_t fade_mode, DMX_Fade_Speed_t speed);

#endif /* DRIVER_DMX_H_ */
