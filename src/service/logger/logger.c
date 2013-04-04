/*
 * logger.c
 *
 *  Created on: 20.03.2013
 *      Author: lumannnn
 */

#include <stdlib.h>
#include <string.h>

#include "logger.h"
#include "../serial_service.h"

#include <stdio.h>

#define FINAL_CHAR '\0'

#define LOG(type, format) { \
		va_list arglist; \
		va_start(arglist, format); \
		log(type, format, arglist); \
		va_end(arglist); \
	}

inline void log(char* type, char* format, va_list arglist);

void logger_init() {
	serial_service_init();
}

void logger_debug(char* format, ...) {
	LOG("DEBUG:\t", format)
}

void logger_warn(char* format, ...) {
	LOG("WARN:\t", format)
}

void logger_error(char* format, ...) {
	LOG("ERROR:\t", format)
}

inline void log(char* type, char* format, va_list arglist) {

	static char buffer[1024];
	#define LOGGER_BUFFER_SIZE sizeof(buffer)

	int l = vsnprintf(buffer, LOGGER_BUFFER_SIZE, format, arglist);

	if (l >= (LOGGER_BUFFER_SIZE - 3)) {
		l = (LOGGER_BUFFER_SIZE - 3);
	}
	buffer[l] = '\r';
	buffer[l + 1] = '\n';
	buffer[l + 2] = '\0';

	serial_service_write(type, strlen(type));
	serial_service_write(buffer, (l + 2));
}

void logger_logmode(void) {
	char* mode;

	switch (_get_CPSR() & 0x1F) {
	case 0x10:
		mode = "User";
		break;
	case 0x11:
		mode = "FIQ";
		break;
	case 0x12:
		mode = "IRQ";
		break;
	case 0x13:
		mode = "Supervisor";
		break;
	case 0x17:
		mode = "Abort";
		break;
	case 0x1B:
		mode = "Undefined";
		break;
	case 0x1F:
		mode = "System";
		break;
	case 0x16:
		mode = "Secure Monitor";
		break;
	default:
		mode = "Unknown";
		break;
	}

	logger_debug("System currently in %s mode", mode);
}
