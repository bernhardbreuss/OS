/*
 * main.c
 */
#include <mem_io.h>
#include <os_stdio.h>
#include <bit.h>
#include <os_std.h>
#include <string.h>


#define NUMBER_OF_CHANNELS 			256
uint8_t channels[20];
driver_msg_t msg;
handle_t handle;
handle_t handle_gpio5_7;
handle_t handle_gpio5_18;

typedef enum {
	RED = 2,
	GREEN = 3,
	BLUE = 4
} Color_t;

typedef enum {
	DMX_COLORSPEC_INTENSIFY = -0x1,
	DMX_COLORSPEC_SOFTEN = 0x1
} DMX_Colorspektrum_Flow_t;


/*static int dmx_write(void* handle, driver_msg_t* buf, size_t size) {
	os_write(&handle, &msg, len);
}*/
static void dmx_uart_set_send_mode(void);
static void dmx_uart_set_reset_mode(void);
static void dmx_send_reset(void);
static void dmx_send_complete_color_spektrum(int time_in_millis, Color_t color, DMX_Colorspektrum_Flow_t flow);

Driver_t driver = {
		NULL,
		NULL,
		NULL,
		NULL,
		NULL//dmx_write
};

#define SCM_CONTROL_PADCONF_MMC2_DAT2 	0x48002160

#define SCM_CONTROL_PADCONF_UART2_TX 	0x48002178
#define SCM_CONTROL_PADCONF_UART2_CTS 	0x48002174

#define SCM_MODE_0					 		0x0
#define SCM_MODE_1							0x1
#define SCM_MODE_2							0x2
#define SCM_MODE_3							0x3
#define SCM_MODE_4							0x4
#define SCM_MODE_5							0x5
#define SCM_MODE_6							0x6
#define SCM_MODE_7							0x7

static void dmx_uart_set_send_mode(void) {

	//Omap3530x.pdf, modes on page 780
	//Chosen modes: page 96 in BBSRM_latest.pdf
	unsigned int addresses[2] = { SCM_CONTROL_PADCONF_UART2_TX, SCM_CONTROL_PADCONF_UART2_CTS };
	unsigned int values[2];

	memory_mapped_read(addresses, 2);
	values[0] = addresses[0] & ~0x70007;
	values[0] |= SCM_MODE_0;
	values[0] |= (SCM_MODE_1 << 16);
	addresses[0] = SCM_CONTROL_PADCONF_UART2_TX;

	values[1] = addresses[1] & ~0x70007;
	values[1] |= SCM_MODE_0;
	values[1] |= (SCM_MODE_0 << 16);
	addresses[1] = SCM_CONTROL_PADCONF_UART2_CTS;

	memory_mapped_write(values, addresses, 2);
}

/**
 * Set Modes as stated in BBSRM_latest.pdf on page 96 to use
 * Expansion-Board pin 6 with GPIO_146.
 */
static void dmx_uart_set_reset_mode(void) {

	//Omap3530x.pdf, modes on page 780
	//Chosen modes: page 96 in BBSRM_latest.pdf
	unsigned int addresses[1] = { SCM_CONTROL_PADCONF_UART2_TX };
	unsigned int values[1];

	memory_mapped_read(addresses, 1);

	values[0] = addresses[0] & ~0x7;
	values[0] |= SCM_MODE_4;
	addresses[0] = SCM_CONTROL_PADCONF_UART2_TX;

	memory_mapped_write(values, addresses, 1);
}

/**
 * Reset: Sending at least 22 bits low (=break) (88 microseconds)
 * then send mark after break (2 bits high) (4 microseconds)
 */
static void dmx_send_reset(void) {
	msg.data[0] = 0x2;
	os_write(&handle_gpio5_18, &msg, 4); //send low

	/* RESET wait some time */
	int i, a;
	a = 240; //~204 microseconds during testing
	for(i = 0; i < a; i++);


	msg.data[0] = 0x1;
	os_write(&handle_gpio5_18, &msg, 4); //send high

	/* MARK AFTER RESET */
	a = 10; //~10 microseconds
	for(i = 0; i < a; i++);
}

static void dmx_send_complete_color_spektrum(int time_in_millis, Color_t color, DMX_Colorspektrum_Flow_t flow) {
	int sleep_per_color = time_in_millis / 255;
	if(sleep_per_color < 2) {
		sleep_per_color = 2;
	}

	int i;
	channels[0] = 0x0;	//start byte
	channels[1] = 0x2; 	//set to RGB
	channels[2] = 0x0;  //red
	channels[3] = 0x0;  //green
	channels[4] = 0x0;  //blue
	channels[5] = 0x0;	//no function - no speed
	for(i = 1; i <= 255; i++) {

		sleep(sleep_per_color);
		dmx_uart_set_reset_mode();
		dmx_send_reset();
		dmx_uart_set_send_mode();
		channels[color] = i;
		memcpy(msg.buffer, channels, sizeof(channels));
		os_write(&handle, &msg, sizeof(channels));
	}
}

int main(void) {
	msg.data[0] = 0xC;	//kbaud divider
	msg.data[1] = 0x1;		//2 stop bit
	msg.data[2] = 0x3;		//data length 8
	msg.data[3] = 0x0;		//no parity

	os_open(22, &msg, 16, &handle, DRIVER_MODE_WRITE);

	msg.data[0] = 7;
	os_open(5, &msg, 4, &handle_gpio5_7, DRIVER_MODE_WRITE);
	msg.data[0] = 18;
	os_open(5, &msg, 4, &handle_gpio5_18, DRIVER_MODE_WRITE);

	msg.data[0] = 0x1;
	os_write(&handle_gpio5_7, &msg, 4); 	//set bit 7 to high, to enable "Driver output" on RS485

	msg.data[0] = 0x1;
	os_write(&handle_gpio5_18, &msg, 4);	//set bit 18 to high

	//Configure RS485 DE (driver output) -----------------------
	//@see CIRCIUT.pdf
	//extension board pin 11, set mode for GPIO
	
	unsigned int address[1] = { SCM_CONTROL_PADCONF_MMC2_DAT2 };
	unsigned int values[1];
	memory_mapped_read(address, 1);

	values[0] = address[0] & ~0x70000;
	values[0] |= (SCM_MODE_4 << 16);
	address[0] = SCM_CONTROL_PADCONF_MMC2_DAT2;

	memory_mapped_write(values, address, 1);

	channels[0] = 0x0;
	channels[1] = 0x2;
	channels[2] = 0x0;
	channels[3] = 0x0;
	channels[4] = 0xFB;
	channels[5] = 0x0;

	memcpy(msg.buffer, channels, sizeof(channels));

	int millis = 2000;

//	while (1) {
//
//		dmx_send_complete_color_spektrum(millis, BLUE, DMX_COLORSPEC_SOFTEN);
//		dmx_send_complete_color_spektrum(millis, BLUE, DMX_COLORSPEC_INTENSIFY);
//
//		dmx_send_complete_color_spektrum(millis, GREEN, DMX_COLORSPEC_SOFTEN);
//		dmx_send_complete_color_spektrum(millis, GREEN, DMX_COLORSPEC_INTENSIFY);
//
//		dmx_send_complete_color_spektrum(millis, RED, DMX_COLORSPEC_SOFTEN);
//		dmx_send_complete_color_spektrum(millis, RED, DMX_COLORSPEC_INTENSIFY);
//
//	}

	int ctr = 0;
	while (1) {

		dmx_uart_set_reset_mode();
		dmx_send_reset();
		dmx_uart_set_send_mode();

		ctr++;
		if(ctr > 254)
			ctr = 0;

//		channels[3] = ctr;
//		memcpy((void*)msg.buffer[3], (void*) channels[3], 1);
		msg.buffer[3] = ctr;
		os_write(&handle, &msg, sizeof(channels));

		sleep(2);
	}
	//driver_init();
}
