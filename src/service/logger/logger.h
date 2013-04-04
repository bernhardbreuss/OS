/*
 * logger.h
 *
 *  Created on: 20.03.2013
 *      Author: Lukas Ender <lukas.ender@students.fhv.at>
 */

#ifndef LOGGER_H_
#define LOGGER_H_

#include "../serial_service.h"

void logger_init();
void logger_debug(char* format, ...);
void logger_warn(char* format, ...);
void logger_error(char* format, ...);
void logger_logmode(void);

#endif /* LOGGER_H_ */
