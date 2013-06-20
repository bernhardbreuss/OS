/*
 * interrupts.h
 *
 *  Created on: Jun 20, 2013
 *      Author: Bernhard
 */

#ifndef INTERRUPTS_H_
#define INTERRUPTS_H_

#include <ring_buffer.h>

void register_interrupt(ring_buffer_t* buffer, int interrupt_id, char* address, size_t size);

#endif /* INTERRUPTS_H_ */
