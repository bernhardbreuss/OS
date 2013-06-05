/*
 * ring_buffer_test.c
 *
 *  Created on: 05.06.2013
 *      Author: edi
 */

#include "../util/ring_buffer.h"
#include "../service/logger/logger.h"
#include <stdlib.h>

static void print_characters(char* chars, int num_to_print) {
	int i;
	for(i = 0; i < num_to_print; i++) {
		logger_debug("A char: %c", *(chars+i));
	}
}

void run_ring_buffer_tests(void) {

	char char_test[5];
	size_t numbers_of_elements_retrieved = 0;

	ring_buffer_t* const ring = ring_buffer_create(5, sizeof(char));
	char t = 't';
	char e = 'e';
	char s = 's';
	char tt = 't';
	char a = 'a';
	char b = 'b';
	ring_buffer_put(ring, &t, 1);
	ring_buffer_put(ring, &e, 1);
	ring_buffer_put(ring, &s, 1);
	ring_buffer_put(ring, &tt, 1);
	ring_buffer_put(ring, &a, 1);
	ring_buffer_put(ring, &b, 1);

	numbers_of_elements_retrieved = ring_buffer_get(ring, (char*) &char_test, 3);
	logger_debug("We retrieved %u characters from buffer", numbers_of_elements_retrieved);
	print_characters((char*) &char_test, numbers_of_elements_retrieved);

	numbers_of_elements_retrieved = ring_buffer_get(ring, (char*) &char_test, 3);
	logger_debug("We retrieved %u characters from buffer", numbers_of_elements_retrieved);
	print_characters((char*) &char_test, numbers_of_elements_retrieved);

	ring_buffer_destroy(ring);

	//	char test[7] = { 'a', 'b', 'c', 'd', 'e', 'f', '\0'};
	//	ring = ring_buffer_create(5, sizeof(char));
	//	ring_buffer_put(ring, &test[0], 6);
	//	logger_debug("abcde current elements in buffer %u", ring->current_elements_in_buffer);
	//	ring_buffer_destroy(ring);
	//
	//	ring_buffer_t* i_ring = ring_buffer_create(3, sizeof(int));
	//	int a1 = 10;
	//	int a2 = 20;
	//	int a3 = 30;
	//	int a4 = 40;
	//	ring_buffer_put(i_ring, (char*) &a1, 1);
	//	ring_buffer_put(i_ring, (char*) &a2, 1);
	//	ring_buffer_put(i_ring, (char*) &a3, 1);
	//	ring_buffer_put(i_ring, (char*) &a4, 1);
	//
	//	int* read_buffer = malloc(3 * sizeof(int));
	//	ring_buffer_get(i_ring, (char*) read_buffer, 3);
	//
	//	logger_debug("First integer: %i", *read_buffer);
	//	logger_debug("Second integer: %i", *(read_buffer+1));
	//	logger_debug("Third integer: %i", *(read_buffer+2));
	//	ring_buffer_destroy(i_ring);
}




