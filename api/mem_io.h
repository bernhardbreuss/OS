/*
 * mem_io.h
 *
 *  Created on: 05.06.2013
 *      Author: Stephan
 */

#ifndef MEM_IO_H_
#define MEM_IO_H_

#include <inttypes.h>
#include "message.h"

#define MEM_IO_READ 16
#define MEM_IO_WRITE 17

typedef struct {
	uint32_t msg_type;
	uint32_t address;
	uint32_t msg;
}io_msg_t;

uint8_t memory_mapped_read(unsigned int* msg, unsigned int* address);

uint8_t memory_mapped_write(unsigned int msg, unsigned int* address);

#endif /* MEM_IO_H_ */
