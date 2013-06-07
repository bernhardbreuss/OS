/*
 * mmu.c
 *
 *  Created on: May 26, 2013
 *      Author: Bernhard
 */

#include "mmu.h"
#include "../../hal/generic/mmu/mmu.h"
#include "../../platform/platform.h"

static unsigned int kernel_mappings[] =  MMU_KERNEL_MAPPING;
#define KERNEL_MAPPINGS_SIZE (sizeof(kernel_mappings) / sizeof(unsigned int))

mmu_table_t* mmu_init(void) {
	mmu_init_hal();

	/* initialize kernel table */
	mmu_table_t* table = mmu_init_process(1);

	if (table == NULL) {
		return NULL;
	}

	/* kernel mappings */
	int i;
	unsigned int start, end;
	size_t mapped;
	for (i = 0; i < KERNEL_MAPPINGS_SIZE; i += 2) {
		start = kernel_mappings[i];
		end = kernel_mappings[i + 1];

		int j;
		for (j = start; j < end; j += mapped) {
			mapped = mmu_map(table, (void*)j, (void*)j);
			if (!mapped) {
				free(table); /* TODO: destroy table */
				return NULL;
			}
		}
	}

	return table;
}
