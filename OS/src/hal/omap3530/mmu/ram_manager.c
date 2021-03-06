/*
 * ram_manager.c
 *
 *  Created on: May 26, 2013
 *      Author: Bernhard
 */

#include "../../generic/mmu/ram_manager.h"
#include "../../../platform/platform.h"

uint32_t ram_manager_align_index(unsigned int alignment, unsigned int* index, uint32_t* bit) {
	unsigned int last_aligned = (unsigned int) ram_manager_get_address(*(index), *(bit));
	last_aligned &= ~(alignment - 1);

	unsigned int next_aligned = (last_aligned + alignment);
	unsigned int difference = (last_aligned - RAM_MANAGER_START_ADDRESS) / RAM_MANAGER_PAGE_SIZE;

	unsigned int new_index = (difference / 32);
	uint32_t new_bit = (difference % 32);

	uint32_t bitstep = (next_aligned - last_aligned) / RAM_MANAGER_PAGE_SIZE;

	if (new_index < *(index) || new_bit < *(bit)) {
		new_bit += bitstep;
		new_index += (new_bit / 32);
		new_bit = (new_bit % 32);
	}

	*(index) = new_index;
	*(bit) = new_bit;

	return bitstep;
}

void* ram_manager_get_address(uint32_t index, uint32_t bit) {
	return (void*) (RAM_MANAGER_START_ADDRESS + (RAM_MANAGER_PAGE_SIZE * ((index * 32) + bit)));
}
