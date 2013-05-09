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

#include "../process.h"

#define BUFFER_SIZE 100

typedef struct {
	uint_32 source;
	uint_32 destination;
	char buffer[BUFFER_SIZE];
	uint8_t size;
}message_t;

#endif /* MESSAGE_H_ */
