/*
 * ring_buffer.h
 *
 *  Created on: 05.06.2013
 *      Author: edi
 */

#ifndef RING_BUFFER_H_
#define RING_BUFFER_H_

#include <stdlib.h>

typedef struct _ring_buffer_t {
	size_t current_elements_in_buffer;
	size_t idx_read_next;
	size_t idx_write_next;
	size_t element_size;
	size_t buffer_size;
	char* buffer;
} ring_buffer_t;

ring_buffer_t* ring_buffer_create(size_t buffer_size, size_t element_size);
void ring_buffer_put(ring_buffer_t* const ring, const char* const first_el_ptr, size_t number_of_elements);
size_t ring_buffer_get(ring_buffer_t* const ring, char* const buffer, size_t max_elements);
void ring_buffer_destroy(ring_buffer_t* ring);

#endif /* RING_BUFFER_H_ */
