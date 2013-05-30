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
	/* initialize kernel table */
	mmu_table_t* table = mmu_init_process(1);

	if (table == NULL) {
		return NULL;
	}

	int i;
	unsigned int start, end;
	for (i = 0; i < KERNEL_MAPPINGS_SIZE; i += 2) {
		start = kernel_mappings[i];
		end = kernel_mappings[i + 1];

		int j;
		for (j = start; j < end; j += MMU_KERNEL_PAGE_SIZE) {
			if (!mmu_map(table, (void*)j, (void*)j)) {
				free(table); /* TODO: create mmu_destroy function */
				return NULL;
			}
		}
	}

	return table;
}
