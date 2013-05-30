/*
 * serial_service.h
 *
 *  Created on: 20.03.2013
 *      Author: lumannnn
 */

#ifndef SERIAL_SERVICE_H_
#define SERIAL_SERVICE_H_


#include "../hal/generic/uart/uart.h"

void serial_service_write(uart_t* const uart, char* buffer, int count);
int serial_service_read(uart_t* const uart, char* buffer, int count);


#endif /* SERIAL_SERVICE_H_ */
