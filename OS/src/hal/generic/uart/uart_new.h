/*
 * uart_new.h
 *
 *  Created on: 24.05.2013
 *      Author: edi
 */

#ifndef UART_NEW_H_
#define UART_NEW_H_

#include "../../platform.h"

void uart_new_get(int uart_nr, uart_t* uart);
void uart_new_software_reset(uart_t* uart);

#endif /* UART_NEW_H_ */
