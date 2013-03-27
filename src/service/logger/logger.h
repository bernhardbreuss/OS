/*
 * logger.h
 *
 *  Created on: 20.03.2013
 *      Author: lumannnn
 */

#ifndef LOGGER_H_
#define LOGGER_H_

#include "../serial_service.h"

void logger_init();
void logger_debug(char* buffer);
void logger_warn(char* buffer);
void logger_error(char* buffer);

#endif /* LOGGER_H_ */
