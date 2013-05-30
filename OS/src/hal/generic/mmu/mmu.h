/*
 * mmu.h
 *
 *  Created on: May 26, 2013
 *      Author: Bernhard
 */

#ifndef GENERIC_MMU_H_
#define GENERIC_MMU_H_

#include "../../../kernel/mmu/mmu.h"
#include <inttypes.h>

mmu_table_t* mmu_init_process(uint8_t kernel);
uint8_t mmu_map(mmu_table_t* table, void* virtual_address, void* physical_address);
uint8_t mmu_load(mmu_table_t* table, void* virtual_address);

#endif /* GENERIC_MMU_H_ */
