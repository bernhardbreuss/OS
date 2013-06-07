/*
 * ram_manager.c
 *
 *  Created on: May 24, 2013
 *      Author: Bernhard
 */

#include "ram_manager.h"
#include <inttypes.h>
#include <string.h>
#include "../../platform/platform.h"
#include "../../hal/generic/mmu/ram_manager.h"

static uint32_t mapping[RAM_MANAGER_PHYSICAL_MEMORY / RAM_MANAGER_PAGE_SIZE / 32]; /* TODO: make platform dependent */
#define MAPPING_SIZE sizeof(mapping)

static unsigned int last_index;
static uint8_t last_bit;

void ram_manager_init(void) {
	last_index = 0;
	last_bit = 31;
	memset(mapping, 0, sizeof(mapping));
}

void* ram_manager_reserve(size_t size) {
	return ram_manager_reserve_aligned(size, 0);
}

void* ram_manager_reserve_aligned(size_t size, unsigned int alignment) {
	if (size == 0) {
		return NULL;
	}

	size_t num_pages = (((size - 1) / RAM_MANAGER_PAGE_SIZE) + 1); /* rounding up */

	unsigned int index = (last_index + (last_bit / 32) % MAPPING_SIZE);
	uint32_t bit = (last_bit + 1) % 32; /* use uint32 to make large alignment possible */

	uint32_t bitstep;
	if (alignment == 0) {
		bitstep = 1;
	} else {
		bitstep = ram_manager_align_index(alignment, &index, &bit);
	}


	size_t free_pages = 0;

	while (index != last_index || bit != last_bit) {
		if (index > MAPPING_SIZE) {
			index = (index % MAPPING_SIZE);
		}

		while (bit < 32) { /* TODO: make reserving atomic */
			if ((mapping[index] & (1 << bit)) == 0) {

				++free_pages;
				if (free_pages == num_pages) {
					/* num_pages free physical pages found */

					last_index = index;
					last_bit = bit;

					/* reserve pages */
					while (free_pages > 0) {
						mapping[index] |= (1 << bit);

						if (bit == 0) {
							bit = 31;
							index--;
						} else {
							bit--;
						}

						free_pages--;
					}

					bit = ((bit + 1) % 32);
					index += (bit / 32);

					return ram_manager_get_address(index, bit);
				}
				bit++;
			} else if (free_pages > 0) {
				bit += (bitstep - free_pages);
				free_pages = 0;
			} else {
				bit += bitstep;
			}
		}

		index += (bit / 32);
		bit = (bit % 32);
	}

	return NULL;
}
