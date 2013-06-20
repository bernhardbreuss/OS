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
#include <string.h>
#include "driver/dmx/dmx.h"

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

static binary_t* binaries[6];
static char BINARY_led0_user[] = BINARY_led0_user_out;
static char BINARY_driver_manager[] = BINARY_driver_manager_out;
static char BINARY_gpio[] = BINARY_gpio_out;
static char BINARY_uart[] = BINARY_uart_out;
static char BINARY_uart2_user[] = BINARY_uart2_user_out;
static char BINARY_dmx[] = BINARY_dmx_out;
static char BINARY_cpu_info[] = BINARY_cpu_info_out;
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

Process_t* driver_manager;
void add_driver(binary_t* binary, char* command_line, Device_t device) {
	message_t msg;
	msg.value.data[0] = DRIVER_MANAGER_ADD;
	msg.value.data[1] = device;
	msg.value.data[2] = (unsigned int)binary;
	strncpy(&(msg.value.buffer[12]), command_line, PROCESS_MAX_NAME_LENGTH);
	/* TODO: check return value */
	ipc_syscall(driver_manager->pid, IPC_SENDREC, &msg);
}

#include <std_adapter.h>
void main(void) {
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

	ram_manager_init();
	mmu_table_t* page_table = mmu_init();

	/* init led stuff */
	turnoff_rgb();

	process_manager_init(page_table);

	binaries[0] = osx_init(&BINARY_driver_manager, &mem_elf_read);
	driver_manager = process_manager_start_process_bybinary(binaries[0], PROCESS_PRIORITY_HIGH, PROCESS_DRIVER_MANAGER_NAME);

	/* add drivers to the driver manager */
	binaries[1] = osx_init(&BINARY_gpio, &mem_elf_read);
	add_driver(binaries[1], "GPIO", GPIO5);

	/* add drivers to the driver manager */
	binaries[4] = osx_init(&BINARY_uart, &mem_elf_read);
	add_driver(binaries[4], "UART 21", UART1);
	add_driver(binaries[4], "UART 22", UART2);
	add_driver(binaries[4], "UART 23", UART3);

	binaries[2] = osx_init(&BINARY_led0_user, &mem_elf_read);
	process_manager_start_process_bybinary(binaries[2], PROCESS_PRIORITY_HIGH, "LED(fast) 21 100 100");
	process_manager_start_process_bybinary(binaries[2], PROCESS_PRIORITY_HIGH, "LED(slow) 22 1000");

	/*
	// test code for binary_map
	binary_map_t* map = malloc(sizeof(binary_map_t));
	if (map == NULL) {
		logger_debug("failed to allocate memory for binary_map_t*");
	} else {
		int status = binary_map_init(map);
		if (status == ERROR) {
			logger_debug("failed to initialize binary_map");
		} else {
			binary_map_add(map, "driver_manager", binaries[0]);
			binary_map_add(map, "gpio", binaries[1]);
			binary_map_add(map, "led0_user", binaries[2]);
			binary_map_add(map, "uart", binaries[4]);

			binary_t* b1 = binary_map_get_binary(map, "driver_manager");
			binary_t* b2 = binary_map_get_binary(map, "gpio");
			binary_t* b3 = binary_map_get_binary(map, "led0_user");
			binary_t* b4 = binary_map_get_binary(map, "uart");
			binary_t* b5 = binary_map_get_binary(map, "definately_not_in_map");

			if (b1 == binaries[0]) {
				logger_debug("SUCCESS: binary mapping successfully found for name 'driver_manager'");
			} else {
				logger_error("FAILURE: binary mapping not found for name 'driver_manager'");
			}
			if (b2 == binaries[1]) {
				logger_debug("SUCCESS: binary mapping successfully found for name 'gpio'");
			} else {
				logger_error("FAILURE: binary mapping not found for name 'gpio'");
			}
			if (b3 == binaries[2]) {
				logger_debug("SUCCESS: binary mapping successfully found for name 'led0_user'");
			} else {
				logger_error("FAILURE: binary mapping not found for name 'led0_user'");
			}
			if (b4 == binaries[4]) {
				logger_debug("SUCCESS: binary mapping successfully found for name 'uart'");
			} else {
				logger_error("FAILURE: binary mapping not found for name 'uart'");
			}
			if (b5 == NULL) {
				logger_debug("SUCCESS: binary mapping not found for 'definately_not_in_map'");
			} else {
				logger_error("FAILURE: What? 'definately_not_in_map' was found? you're kidding me, right?");
			}

		}
	}
	*/

	dmx_uart_set_send_mode();

	//binaries[3] = osx_init(&BINARY_uart2_user, &mem_elf_read);
	//process_manager_start_process_bybinary(binaries[3], PROCESS_PRIORITY_HIGH, "UART2_user_proc");

	binaries[3] = osx_init(&BINARY_dmx, &mem_elf_read);
	// TODO: add_driver(binaries[3], "DMX", DMX);
	process_manager_start_process_bybinary(binaries[3], PROCESS_PRIORITY_HIGH, "DMX");

	binaries[5] = osx_init(&BINARY_cpu_info, &mem_elf_read);
	process_manager_start_process_bybinary(binaries[5], PROCESS_PRIORITY_HIGH, "CPU_INFO");


	logger_debug("System started ...");

	system_main_loop();
}
