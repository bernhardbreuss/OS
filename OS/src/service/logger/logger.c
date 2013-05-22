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

void log(char* type, char* format, va_list arglist) {

	static char buffer[1024];
	#define LOGGER_BUFFER_SIZE sizeof(buffer)

	int l = vsnprintf(buffer, LOGGER_BUFFER_SIZE, format, arglist);

	if (l < 0 || l >= (LOGGER_BUFFER_SIZE - 3)) {
		l = (LOGGER_BUFFER_SIZE - 3);
	}
	buffer[l] = '\r';
	buffer[l + 1] = '\n';
	buffer[l + 2] = '\0';

	serial_service_write(type, strlen(type));
	serial_service_write(buffer, (l + 2));
}

void logger_logmode(void) {
	logger_debug("System currently in %s mode", logger_getmode(_get_CPSR()));
}

/**
 * Format has to contain at least one %s. The first occurrence
 * of %s is substituted with the binary value of the register.
 */
void logger_log_register(char* format, volatile unsigned int * register_ptr) {
	char bin_reg_val [32];
	unsigned int register_val = *register_ptr;
	int i = 0;
	for(i = 31; i >= 0; i--) {
		if(register_val & 1) {
			bin_reg_val[i] = '1';
		} else {
			bin_reg_val[i] = '0';
		}
		register_val >>= 1;
	}
	logger_debug(format, bin_reg_val);
}

char* logger_getmode(unsigned int cpsr) {
	char* mode;

	switch (cpsr & 0x1F) {
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

	return mode;
}
