/*
 * serial_service.c
 *
 *  Created on: 20.03.2013
 *      Author: lumannnn
 */

#include "../hal/uart/uart.h"
#include "serial_service.h"

#define FALSE 0
#define TRUE 1
#define FINAL_CHAR '\0'

extern int init_uart_rs232_ = FALSE;

void serial_service_init() {
	if (init_uart_rs232_ == FALSE) {
		uart_init((mem_address_t*) UART3, UART_MODE_16X, uart_protocol_rs232,
				UART_FLOW_CONTROL_DISABLE_FLAG);
		init_uart_rs232_ = TRUE;
		return;
	}
	return;
}

void serial_service_write(char* buffer, int count) {
	mem_address_t* uart = (mem_address_t*) UART3;
	int i = 0;

	for (; i < count; i++, buffer++) {
		// block while queue is full
		while (!uart_is_empty_write_queue(uart))
			;
		uart_write(uart, buffer);
	}

	return;
}

int serial_service_read(char* buffer, int count) {
	mem_address_t* uart = (mem_address_t*) UART3;
	int i = 0;

	for (; i < count; i++) {
		// block while waiting for data
		while (uart_is_empty_read_queue(uart))
			;
		uart_read(uart, &(buffer[i]));

		// stop reading when receiving a return
		// TODO: just one char - change func?
		//if(buffer[i] == '\r')  break;
	}

	return i;
}

