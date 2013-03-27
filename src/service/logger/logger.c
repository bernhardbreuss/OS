/*
 * logger.c
 *
 *  Created on: 20.03.2013
 *      Author: lumannnn
 */

#include <string.h>
#include <stdio.h>

#include "logger.h"
#include "../serial_service.h"

#define FINAL_CHAR '\0'

inline static int calculate_length(char* buffer);

void logger_init() {
	serial_service_init();
}

void logger_debug(char* buffer) {
//	int bufferLength = calculate_length(buffer) + 7;
//	char temp[bufferLength];
//	strcpy(temp, "DEBUG: ");
//	strcat(temp, buffer);
//	printf(temp);

	serial_service_write(buffer, 1);
}
void logger_warn(char* buffer) {
//	int bufferLength = calculate_length(buffer);
//	char temp[6 + bufferLength];
//	strcpy(temp, "WARN: ");
//	strcat(temp, buffer);
//	printf(temp);

	serial_service_write(buffer, 1);
}
void logger_error(char* buffer) {
//	int bufferLength = calculate_length(buffer);
//	char temp[7 + bufferLength];
//	strcpy(temp, "ERROR: ");
//	strcat(temp, buffer);
//	printf(temp);

	serial_service_write(buffer, 1);
}

inline static int calculate_length(char* buffer) {
	int bufferLength = 0;
	int index = 0;
	// calculate the length of the buffer
	while ((*(buffer + index++)) != FINAL_CHAR) {
		bufferLength++;
	}

	return bufferLength;
}
