/*
 * mmu.h
 *
 *  Created on: May 24, 2013
 *      Author: Bernhard
 */

#ifndef MMU_H_
#define MMU_H_

#include <stdlib.h>
#include <inttypes.h>

typedef enum {
	MMU_SMALL_PAGE		=    1,	/* 0x0001000 =  4KB */
	MMU_LARGE_PAGE		=   16,	/* 0x0010000 = 64KB */
	MMU_SECTION			=  256,	/* 0x0100000 =  1MB */
	MMU_SUPERSECTION	= 4096	/* 0x1000000 = 16MB */
} page_size_t;

typedef struct _mmu_table_t {
	void* address;
	page_size_t page_size;
	uint8_t kernel_table;
} mmu_table_t;

mmu_table_t* mmu_init(void);

#endif /* MMU_H_ */
