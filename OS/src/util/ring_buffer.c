/*
 * ring_buffer.c
 *
 *  Created on: 05.06.2013
 *      Author: edi
 */

#include "ring_buffer.h"
#include <stdlib.h>

/**
 * Creates and returns the ring buffer or NULL pointer on error.
 * E.g. memory allocation failed.
 */
ring_buffer_t* ring_buffer_create(size_t buffer_size, size_t element_size) {
	ring_buffer_t* ring = malloc(sizeof(ring_buffer_t));
	char* buffer = (char*) malloc(buffer_size * element_size);

	if(buffer == NULL || ring == NULL)
		return NULL;

	ring->buffer = buffer;
	ring->current_elements_in_buffer = 0;
	ring->idx_read_next = 0;
	ring->idx_write_next = 0;
	ring->element_size = element_size;
	ring->buffer_size = buffer_size;
	return ring;
}

void ring_buffer_put(ring_buffer_t* const ring, const char* const first_el_ptr, size_t number_of_elements) {
	size_t e, n, current_el_number;
	e = n = current_el_number = 0;

	if(ring != NULL) {

		const char* source_buffer = first_el_ptr;

		//move to next write location
		char* buffer_ptr  = ring->buffer;
		buffer_ptr += (ring->idx_write_next * ring->element_size);

		while(n < ring->buffer_size									//max write count per function call is the max. buffer size
				&& (current_el_number < number_of_elements)) {		//write exactly as much elements as requested

			current_el_number++;

			//write a single element
			while(e < ring->element_size) {
				*(buffer_ptr) = *(source_buffer);
				buffer_ptr += 1;
				source_buffer += 1;
				e++;
			}
			//after a complete single element is added to the buffer we update the element counter variable
			//counter variables max value is the max buffer size
			if(ring->current_elements_in_buffer <= (ring->buffer_size - 1)) {
				ring->current_elements_in_buffer++;
			}

			//move to next write location
			if(ring->idx_write_next == (ring->buffer_size-1))
				ring->idx_write_next = 0;
			else ring->idx_write_next++;

			e = 0;
			n++;
		}
	}
}

size_t ring_buffer_get(ring_buffer_t* const ring, char* const dest, size_t read_max_elements) {
	size_t e, num_el_read;
	e = num_el_read = 0;

	if(ring != NULL) {

		char* dest_buffer = dest;

		//move to next read location
		char* src_buffer = ring->buffer;
		src_buffer += (ring->idx_read_next * ring->element_size);

		while(ring->current_elements_in_buffer > 0			//we can only read from buffer if at least one element is available
				&& (num_el_read < read_max_elements )) {	//read only as much elements as requested

			num_el_read++;

			//read a single element
			while(e < ring->element_size) {
				*(dest_buffer) = *(src_buffer);
				dest_buffer += 1;
				src_buffer += 1;
				e++;
			}

			//after read a complete element, update buffer element counter variable
			ring->current_elements_in_buffer--;

			//move to next read location
			if(ring->idx_read_next == (ring->buffer_size-1))
				ring->idx_read_next = 0;
			else ring->idx_read_next++;

			e= 0;
		}
	}

	return num_el_read;
}

void ring_buffer_destroy(ring_buffer_t* ring) {
	if(ring != NULL) {
		free(ring->buffer);
		free(ring);
	}
	ring = NULL;
}
