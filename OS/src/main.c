/*
 * main.c
 */

#include <inttypes.h>
#include "service/logger/logger.h"
#include "hal/generic/timer/gptimer.h"
#include "hal/generic/pwm/pwm.h"
#include "kernel/process/process.h"
#include "kernel/process/process_manager.h"
#include "hal/generic/uart/uart.h"
#include "driver/driver_manager.h" /* TODO: move to kernel */
#include "kernel/ipc/ipc.h"
#include "tests/pwm_test.h"
#include "kernel/loader/binary.h"
#include "kernel/loader/elf.h"
#include "kernel/loader/loader.h"
#include "kernel/mmu/mmu.h"
#include "kernel/mmu/ram_manager.h"
#include "service/serial_service.h"
//#include "binary.h"
#include "hal/generic/irq/irq.h"

#pragma INTERRUPT(udef_handler, UDEF);
interrupt void udef_handler() {
	logger_error("KERNEL PANIC: udef handler.");
	while(1);
}

gptimer_t main_timer;

#define LED0_PIN			(1 << 21)
#define LED1_PIN			(1 << 22)
#define GPIO5_OUT			(unsigned int*) 0x4905603C
#define GPIO_ON		0x1
#define GPIO_TOGGLE	0x2
#define GPIO_OFF	0x3

Process_t process1, process2, process3;

uint32_t led0(void) {
	int i;
	logger_debug("Led 0");
	while (1) {
		for(i = 0; i < 450000; i++) ;
		*(GPIO5_OUT) ^= LED0_PIN;
	}
}

uint32_t led1(void) {
	int i;
	logger_debug("Led 1");
	while (1) {
		for(i = 0; i < 900000; i++);
		*(GPIO5_OUT) ^= LED1_PIN;
	}
}

void turnoff_rgb(void) {
	/* set mode to 4 (GPIO) see p. ~787 of omap35x.pdf */
	unsigned int* CONTROL_PADCONF_UART2_CTS = (unsigned int*)0x48002174; /* GPIO144 15:0 GPIO145 16:32 */
	unsigned int* CONTROL_PADCONF_UART2_TX = (unsigned int*)0x48002178; /* GPIO146 15:0 */
	*(CONTROL_PADCONF_UART2_CTS) = (4 << 16) | 4;
	*(CONTROL_PADCONF_UART2_TX) &= ~7;
	*(CONTROL_PADCONF_UART2_TX) |= 4;

	/* turn off rgb led on dmx interface
	 *  GPIO 144, 146, 145 --> GPIO 5 */
	unsigned int* GPIO5_OE = (unsigned int*)0x49056034;
	unsigned int* GPIO5_DATAOUT = (unsigned int*)0x4905603C;
	unsigned int rgb = (1 << (144 % 32)) | (1 << (146 % 32)) | (1 << (145 % 32));
	*(GPIO5_OE) &= ~rgb;
	*(GPIO5_DATAOUT) &= ~rgb;
}

void do_pwm(void);

uint32_t ipc_process1(void) {
	//GPIO led 0 toggle via IPC
	message_t msg;
	msg.type = MESSAGE_TYPE_DATA;
	msg.value.data[0] = DEVICE_OPEN;
	//led0 is accessible over the twenty-first pin on GPIO5
	//omap3530x.pdf page 3383
	//BBSRM_latest.pdf page 64
	msg.value.data[1] = 21;
	msg.value.data[2] = GPIO_TOGGLE;
	msg.size = 3;

	int i;
	while (1) {
		ipc_syscall_device(GPIO5, IPC_SENDREC, &msg); /* send device GPIO5 something */
		msg.value.data[0] = DEVICE_WRITE; //device opened already, we want to write data
		for (i = 0; i < 450000; i++) ;
	}
}

uart_t uart3;

uint32_t BB_read(void* ident, void* dst, uint32_t offset, size_t length) {
#ifndef BINARY_BeagleBlink_out /* ensure that everyone can build */
#define BINARY_BeagleBlink_out { '\0' }
#endif
	static char BeagleBlink[] = BINARY_BeagleBlink_out;

	if (length == 0 || (offset + length) > sizeof(BeagleBlink)) {
		return 0;
	}

	memcpy(dst, &BeagleBlink[offset], length);
	return 1;
}

extern uint32_t led1_user(void);
extern unsigned int led1_user_virtual;
extern unsigned int led1_user_physical;
extern unsigned int led1_user_size;

void uart3_irq_handler(void);
void uart3_irq_handler(void) {
	char received_char = *((char*) 0x49020000);
	logger_debug("UART3 - Received a character: %c", received_char);
}


extern Driver_t gpio_driver;
ProcessId_t gpio_start_driver_process(Device_t device);
void main(void) {
	ram_manager_init();
	mmu_table_t* page_table = mmu_init();

	/* logger_init() */
	uart_get(3, &uart3);
	uart_protocol_format_t protocol;
	protocol.baudrate = 0x001A; //115.2Kbps		138;	//9.6 Kbps
	protocol.stopbit = 0x0;		//1 stop bit
	protocol.datalen = 0x3;		//length 8
	protocol.use_parity = 0x0;
	uart_init(&uart3, 0x00, protocol);

	logger_debug("\r\n\r\nSystem initialize ...");
	logger_logmode();



	/*asm("\t CPS #0x10");
	logger_logmode();*/

	/* init led stuff */
	turnoff_rgb();

	driver_manager_init();
	driver_manager_add_driver(GPIO5, &gpio_driver, &gpio_start_driver_process);

	process_manager_init(page_table);

	process1.func = &ipc_process1;
	process1.name = "LED 0 (IPC, fast)";
	process_manager_add_process(&process1);

	/*process2.func = &led1_user;
	process2.name = "LED 1 (slow)";
	process_manager_add_process(&process2);*/
	process_manager_start_process_byfunc(&led1_user, "LED1 User", PROCESS_PRIORITY_HIGH, (unsigned int)&led1_user_virtual, (unsigned int)&led1_user_physical, (unsigned int)&led1_user_size);

	/* Loader test stuff
	binary_t* binary = elf_init(NULL, BB_read);
	process_manager_start_process_bybinary(binary, "BeagleBlink", PROCESS_PRIORITY_HIGH);*/

	irq_add_handler(UART3_INTCPS_MAPPING_ID, &uart3_irq_handler);

	logger_debug("Kernel started ...");
	/* TODO: start IPC */
	while (1) ;
}
