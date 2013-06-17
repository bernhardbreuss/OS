/*
 * mmu.h
 *
 *  Created on: May 26, 2013
 *      Author: Bernhard
 */

#ifndef PLATFORM_OMAP3530_MMU_H_
#define PLATFORM_OMAP3530_MMU_H_

#define MMU_SMALLEST_PAGE_SIZE 4096

#define MMU_KERNEL_MAPPING { \
	0x40200000, 0x4020FFE4, /* SRAM and interrupt vectors */ \
	0x80000000, 0x84000000, /* DDR */ \
	0x48000000, 0x49FFFFFF  /* memory mapped IO (brute-force method) */ \
}

#define MMU_KERNEL_PAGE_SIZE 0x100000 /* 1MB */

#endif /* PLATFORM_OMAP3530_MMU_H_ */
