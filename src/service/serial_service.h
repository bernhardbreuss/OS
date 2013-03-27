/*
 * serial_service.h
 *
 *  Created on: 20.03.2013
 *      Author: lumannnn
 */

#ifndef SERIAL_SERVICE_H_
#define SERIAL_SERVICE_H_


#include "../hal/uart/uart.h"

void serial_service_init();
void serial_service_write(char* buffer, int count);
int serial_service_read(char* buffer, int count);


#endif /* SERIAL_SERVICE_H_ */
