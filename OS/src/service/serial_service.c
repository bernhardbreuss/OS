/*
 * serial_service.c
 *
 *  Created on: 20.03.2013
 *      Author: lumannnn
 */

#include "../hal/generic/uart/uart.h"
#include "serial_service.h"

#define FALSE 0
#define TRUE 1
#define FINAL_CHAR '\0'

void serial_service_write(uart_t* const uart, char* buffer, int count) {
	int i = 0;

	for (; i < count; i++, buffer++) {
		// block while queue is full
		while (!uart_is_empty_write_queue(uart))
			;
		uart_write(uart, buffer);
	}

	return;
}

int serial_service_read(uart_t* const uart, char* buffer, int count) {
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

