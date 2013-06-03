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
#include "../../../kernel/process/process.h"

mmu_table_t* mmu_init_process(uint8_t kernel);
unsigned int mmu_map(mmu_table_t* table, void* virtual_address, void* physical_address);

void mmu_init_hal(void);
void mmu_activate_process(Process_t* process);
void mmu_start(void);

#endif /* GENERIC_MMU_H_ */
