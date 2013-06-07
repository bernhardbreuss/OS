/*
 * uart.h
 *
 *  Created on: 24.05.2013
 *      Author: edi
 */

#ifndef UART_H_
#define UART_H_

#include "../../../platform/platform.h"
#include <inttypes.h>

void uart_get(int uart_nr, uart_t* uart);
void uart_init(uart_t* const uart, int uart_mode, struct uart_protocol_format_t protocol);
void uart_read(uart_t* const uart, char* buffer);
void uart_write(uart_t* const uart, char* buffer);
int uart_is_empty_read_queue(uart_t* const uart);
int uart_is_empty_write_queue(uart_t* const uart);

#endif /* UART_H_ */
