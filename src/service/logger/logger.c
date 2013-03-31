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

#define FINAL_CHAR '\0'

inline static int logger_concat(const char* c1, const char* c2, char** out);

void logger_init() {
	serial_service_init();
}

void logger_debug(char* buffer) {
	char* type = "DEBUG: ";
	char* newBuffer;
	int newBufferLength = logger_concat(type, buffer, &newBuffer);

	serial_service_write(newBuffer, newBufferLength);

	free(newBuffer);
}
void logger_warn(char* buffer) {
	char* type = "WARNING: ";
	char* newBuffer;
	int newBufferLength = logger_concat(type, buffer, &newBuffer);

	serial_service_write(newBuffer, newBufferLength);

	free(newBuffer);
}
void logger_error(char* buffer) {
	char* type = "ERROR: ";
	char* newBuffer;
	int newBufferLength = logger_concat(type, buffer, &newBuffer);

	serial_service_write(newBuffer, newBufferLength);

	free(newBuffer);
}

inline static int logger_concat(const char* c1, const char* c2, char** cout) {
	size_t c1Length = strlen(c1);
	size_t c2Length = strlen(c2);

	free(*cout);

	*cout = (char*) malloc(c1Length + c2Length + 1);
	memcpy(*cout, c1, c1Length);
	memcpy(*cout + c1Length, c2, c2Length + 1);
	int coutLength = strlen(*cout);

	return coutLength;
}
