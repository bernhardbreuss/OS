/*
 * main.c
 */

#include <inttypes.h>
#include "service/logger/logger.h"
#include "hal/generic/timer/gptimer.h"
#include "hal/generic/pwm/pwm.h"
#include "kernel/process.h"
#include "kernel/process_manager.h"
#include "driver/driver_manager.h" /* TODO: move to kernel */
#include "kernel/ipc/ipc.h"
#include "tests/pwm_test.h"

#pragma INTERRUPT(udef_handler, UDEF);
interrupt void udef_handler() {
	logger_error("KERNEL PANIC: udef handler.");
	while(1);
}

#pragma INTERRUPT(pabt_handler, PABT);
interrupt void pabt_handler() {
	logger_error("KERNLE PANIC: Prefetch abort.");
	while(1);
}

#pragma INTERRUPT(dabt_handler, DABT);
interrupt void dabt_handler() {
	//the address of the disassemble instruction where the data abort happened
	//is located at (R14_ABT-8) see "Table 9.4" in "Arm System Developers Guide"
	logger_error("KERNEL PANIC: data abort");
	while(1);
}

gptimer_t main_timer;

#define LED0_PIN			(1 << 21)
#define LED1_PIN			(1 << 22)
#define GPIO5_OUT			(unsigned int*) 0x4905603C
#define GPIO_ON		0x1
#define GPIO_TOGGLE	0x2
#define GPIO_OFF	0x3

Process_t process1;
Process_t process2;

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

uint32_t idle_task(void) {
	while (1) ; /* TODO: look manual for HALT command or similar to reduce power consumption */
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

extern Driver_t gpio_driver;
ProcessId_t gpio_start_driver_process(Device_t device);
void main(void) {
	logger_init();
	logger_debug("\r\n\r\nSystem init...");
	logger_logmode();

	asm("\t CPS #0x10");
	logger_logmode();

	/* init led stuff */
	turnoff_rgb();
	#define GPIO5_DIR  			(unsigned int*) 0x49056094
	*(GPIO5_DIR) |= LED0_PIN | LED1_PIN;

	driver_manager_init();
	driver_manager_add_driver(GPIO5, &gpio_driver, &gpio_start_driver_process);

	process_manager_init();

	process1.func = &ipc_process1;
	process1.name = "LED 0 (IPC, fast)";
	process_manager_add_process(&process1);

	process2.func = &led1;
	process2.name = "LED 1 (slow)";
	process_manager_add_process(&process2);

	/* idle task */
	Process_t idle_process;
	idle_process.func = &idle_task;
	idle_process.name = "idle process";
	process_manager_add_process(&idle_process);

	/* start scheduling */
	process_manager_start_scheduling();
}
