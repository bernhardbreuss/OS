/*
 * main.c
 */
#include <mem_io.h>
#include <os_stdio.h>
#include <bit.h>
#include <os_std.h>
#include <string.h>
#include <driver.h>

typedef struct _dmx_owner {
	unsigned int owned 		: 1;
	unsigned int owned_id	: 31;
}  dmx_owner_t;

dmx_owner_t owner = { 0x0, 0x1 };

driver_msg_t channels;
driver_msg_t msg;
size_t channel_number_to_send = 0;	//TODO: set the channels in open
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

static void dmx_uart_set_send_mode(void);
static void dmx_uart_set_reset_mode(void);
static void dmx_send_reset(void);

static int dmx_write(void* dmx_handle, driver_msg_t* buf, size_t size) {
	if(owner.owned_id == (unsigned int) dmx_handle) {
		size_t to_copy = (size > sizeof(channels) ? sizeof(channels) : size);
		channel_number_to_send = to_copy;

		memcpy(&channels.buffer[0], buf->buffer, to_copy);
		return 1;
	}
	return 0;
}

static void* dmx_open(driver_msg_t* buf, size_t size, driver_mode_t mode) {
	if(!owner.owned) {
		owner.owned = 0x1;
		return (void*) owner.owned_id;
	}
	return NULL;
}

static int dmx_close(void* dmx_handle) {

	//TODO: don't allow all to close the handle

	owner.owned = 0x0;
	owner.owned_id +=1;
	return 1;
}

static void dmx_async_loop(void) {
	sleep(2);
	dmx_uart_set_reset_mode();
	dmx_send_reset();

	dmx_uart_set_send_mode();

	os_write(&handle, &channels, channel_number_to_send);

	dmx_uart_set_reset_mode();
	sleep(2);
}


Driver_t driver = {
		NULL,
		dmx_open,
		dmx_close,
		NULL,
		dmx_write
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

int main(int argc, char* argv[]) {

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

	channels.buffer[0] = 0x0;
	channels.buffer[1] = 0x2;
	channels.buffer[2] = 0x0;
	channels.buffer[3] = 0xFF;
	channels.buffer[4] = 0x0;
	channels.buffer[5] = 0x0;

	channel_number_to_send = 6;

	driver_init(&dmx_async_loop);
}
