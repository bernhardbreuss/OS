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

#include <limits.h>
#include "process.h"

#define MESSAGE_BUFFER_SIZE 512
#define MESSAGE_DATA_SIZE (MESSAGE_BUFFER_SIZE / sizeof(unsigned int))

#define MESSAGE_UNKNOWN INT_MIN
#define MESSAGE_DEVICE_UNKNOWN (MESSAGE_UNKNOWN + 1)

typedef enum {
	MESSAGE_TYPE_BUFFER,
	MESSAGE_TYPE_DATA
} message_type_t;

struct _message_t {
	ProcessId_t source;
	ProcessId_t destination;
	union {
		char buffer[MESSAGE_BUFFER_SIZE];
		unsigned int data[MESSAGE_DATA_SIZE];
	} value;
	uint8_t size;
	message_type_t type;
};

#endif /* MESSAGE_H_ */
