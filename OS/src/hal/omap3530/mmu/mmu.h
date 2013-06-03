/*
 * mmu.h
 *
 *  Created on: May 24, 2013
 *      Author: Bernhard
 */

#ifndef OMAP3530_MMU_H_
#define OMAP3530_MMU_H_

#include "../../../bit.h"

#define MMU_USER_N 0x1

#define MMU_FIRST_LEVEL_KERNEL_TABLE_SIZE	0x4000 /* 16KB */
#define MMU_FIRST_LEVEL_USER_TABLE_SIZE		(MMU_FIRST_LEVEL_KERNEL_TABLE_SIZE >> MMU_USER_N)
#define MMU_SECOND_LEVEL_TABLE_SIZE			0x0400 /*  1KB */

#define MMU_PAGE_TABLE_MASK 0xFFFFFC00
#define MMU_SECTION_MASK	0xFFF00000
#define MMU_SMALL_PAGE_MASK	0xFFFFF000

#define MMU_PAGE_TABLE_KERNEL_ALIGNMENT MMU_FIRST_LEVEL_KERNEL_TABLE_SIZE
#define MMU_PAGE_TABLE_USER_ALIGNMENT MMU_FIRST_LEVEL_USER_TABLE_SIZE
#define MMU_SECTION_ALIGNMENT ((~MMU_SECTION_MASK) + 1)
#define MMU_SMALL_PAGE_ALIGNMENT ((~MMU_SMALL_PAGE_MASK) + 1)

#define MMU_PAGE_TABLE_DESCRIPTOR BIT0
#define MMU_SECTION_DESCRIPTOR BIT1
#define MMU_FIRST_LEVEL_DESCRIPTOR BIT0
#define MMU_LARGE_PAGE_DESCRIPTOR BIT0
#define MMU_SMALL_PAGE_DESCRIPTOR BIT1

#define MMU_DOMAIN 0 /* --> Domain 0 */

#define MMU_FIRST_LEVEL_KERNEL_AP BIT10 /* read/write access in privileged mode else no access */
#define MMU_FIRST_LEVEL_USER_AP (BIT11 | BIT10) /* read/write access in privileged mode and user mode */
#define MMU_SECOND_LEVEL_AP (BIT4 | BIT5) /* read/write access in privileged mode and user mode */

#define MMU_FIRST_LEVEL_NOT_GLOBAL BIT17
#define MMU_SECOND_LEVEL_NOT_GLOBAL BIT11

#define MMU_SECTION_INDEX_KERNEL_MASK 0xFFF00000

#define MMU_SECTION_SIZE	((~MMU_SECTION_MASK) + 1)
#define MMU_SMALL_PAGE_SIZE	((~MMU_SMALL_PAGE_MASK) + 1)

void mmu_ttbr_set0(unsigned int value, unsigned int contextidr);
void mmu_ttbr_set1(unsigned int value);

unsigned int mmu_get_ifsr(void);
void* mmu_get_ifar(void);
unsigned int mmu_get_dfsr(void);
void* mmu_get_dfar(void);

#define MMU_ABORT_TRANSLATION_FAULT_SECTION 0x5
#define MMU_ABORT_TRANSLATION_FAULT_PAGE 0x7
#define MMU_ABORT_DEBUG 0x2

#define MMU_ABORT_MASK 0x140F


#endif /* OMAP3530_MMU_H_ */
