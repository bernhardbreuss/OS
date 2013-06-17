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
#include <driver_manager.h>
#include <ipc.h>
#include "tests/pwm_test.h"
#include "tests/ring_buffer_test.h"
#include "kernel/system.h"
#include "kernel/loader/binary.h"
#include "kernel/loader/osx.h"
#include "kernel/loader/loader.h"
#include "kernel/mmu/mmu.h"
#include "kernel/mmu/ram_manager.h"
#include "service/serial_service.h"
#include "binary.h"
#include "hal/generic/irq/irq.h"

#pragma INTERRUPT(udef_handler, UDEF);
interrupt void udef_handler() {
	logger_error("KERNEL PANIC: udef handler.");
	while(1);
}

gptimer_t main_timer;

#define GPIO5_OUT			(unsigned int*) 0x4905603C
#define GPIO_ON		0x1
#define GPIO_TOGGLE	0x2
#define GPIO_OFF	0x3

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


uart_t uart3;

static binary_t* binaries[4];
static char BINARY_led0_user[] = BINARY_led0_user_out;
static char BINARY_led1_user[] = BINARY_led1_user_out;
static char BINARY_driver_manager[] = BINARY_driver_manager_out;
static char BINARY_gpio[] = BINARY_gpio_out;
uint32_t mem_elf_read(void* ident, void* dst, uint32_t offset, size_t length) {
	if (length == 0) {
		return 0;
	}

	memcpy(dst, ((uint8_t*)ident + offset), length);
	return 1;
}

void uart3_irq_handler(void);
void uart3_irq_handler(void) {
	char received_char = *((char*) 0x49020000);
	logger_debug("UART3 - Received a character: %c", received_char);
}

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
	irq_add_handler(UART3_INTCPS_MAPPING_ID, &uart3_irq_handler);

	logger_debug("\r\n\r\nSystem initialize ...");
	logger_logmode();

	/* init led stuff */
	turnoff_rgb();

	process_manager_init(page_table);

	binaries[0] = osx_init(&BINARY_driver_manager, &mem_elf_read);
	ProcessId_t driver_manager = process_manager_start_process_bybinary(binaries[0], PROCESS_DRIVER_MANAGER_NAME, PROCESS_PRIORITY_HIGH);

	/* add drivers to the driver manager */
	binaries[1] = osx_init(&BINARY_gpio, &mem_elf_read);
	message_t msg;
	msg.value.data[0] = DRIVER_MANAGER_ADD;
	msg.value.data[1] = GPIO5;
	msg.value.data[2] = (unsigned int)(binaries[1]);
	process_name_t name = "GPIO";
	memcpy(&(msg.value.buffer[12]), name, sizeof(name));
	ipc_syscall(driver_manager, IPC_SENDREC, &msg); /* TODO: check return value */

	binaries[2] = osx_init(&BINARY_led0_user, &mem_elf_read);
	process_manager_start_process_bybinary(binaries[2], "LED0 User (fast)", PROCESS_PRIORITY_HIGH);

	binaries[3] = osx_init(&BINARY_led1_user, &mem_elf_read);
	process_manager_start_process_bybinary(binaries[3], "LED1 User (slow)", PROCESS_PRIORITY_HIGH);

	logger_debug("System started ...");

	kernel_main_loop();
}
