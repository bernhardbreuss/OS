/*
 * message.h
 *
 * First simple message model
 *
 *  Created on: 09.05.2013
 *      Author: Stephan
 */

#ifndef MESSAGE_H_
#define MESSAGE_H_

typedef struct _message_t message_t;

#include "../process/process.h"
//typedef int32_t ProcessId_t; /* forward declaration instead of #include "../process.h" to avoid circular inclusion */

#define BUFFER_SIZE 100
#define DATA_SIZE BUFFER_SIZE / sizeof(unsigned int)

typedef enum {
	MESSAGE_TYPE_BUFFER,
	MESSAGE_TYPE_DATA
} message_type_t;

struct _message_t {
	ProcessId_t source;
	ProcessId_t destination;
	union {
		char buffer[BUFFER_SIZE];
		unsigned int data[DATA_SIZE];
	} value;
	uint8_t size;
	message_type_t type;
};

#endif /* MESSAGE_H_ */
