/*
 * mmu.c
 *
 *  Created on: May 24, 2013
 *      Author: Bernhard
 */

#include <stdlib.h>
#include <string.h>
#include "../../../kernel/mmu/mmu.h"
#include "mmu.h"
#include "../../../platform/platform.h"
#include "../../../kernel/mmu/ram_manager.h"
#include "../../../kernel/loader/loader.h"

extern unsigned int* kernel_master_table;

static uint8_t _mmu_init_pagetable(mmu_table_t* table) {
	size_t table_size;

	switch (table->page_size) {
	case MMU_SMALL_PAGE:
	case MMU_LARGE_PAGE:
		table_size = MMU_SECOND_LEVEL_TABLE_SIZE;
		break;
	case MMU_SECTION:
	case MMU_SUPERSECTION:
		if (table->kernel_table) {
			table_size = MMU_FIRST_LEVEL_KERNEL_TABLE_SIZE;
		} else {
			table_size = MMU_FIRST_LEVEL_USER_TABLE_SIZE;
		}
		break;
	default:
		return 0;
	}

	memset(table->address, 0, table_size);

	return 1;
}

mmu_table_t* mmu_init_process(uint8_t kernel) {
	mmu_table_t* table = malloc(sizeof(mmu_table_t));
	if (table == NULL) {
		return NULL;
	}

	if (kernel) {
		table->address = ram_manager_reserve_aligned(MMU_FIRST_LEVEL_KERNEL_TABLE_SIZE, MMU_PAGE_TABLE_KERNEL_ALIGNMENT);
	} else {
		table->address = ram_manager_reserve_aligned(MMU_FIRST_LEVEL_USER_TABLE_SIZE, MMU_PAGE_TABLE_USER_ALIGNMENT);
	}

	if (table->address == NULL) {
		free(table);
		return NULL;
	}

	table->page_size = MMU_SECTION;
	table->kernel_table = kernel;

	if (_mmu_init_pagetable(table)) {
		return table;
	} else {
		free(table);
		return NULL;
	}
}

static unsigned int* _mmu_get_first_level_descriptor_address(mmu_table_t* table, void* virtual_address) {
	unsigned int table_index = (unsigned int)virtual_address;

	if (table->kernel_table) {
		table_index &= 0xFFF00000;
	} else {
		table_index &= 0xFFF00000 & ((unsigned int)~0 >> MMU_USER_N);
	}

	return (void*)(((unsigned int)table->address) + (table_index >> 18));
}

static unsigned int* _mmu_get_second_level_descriptor_address(mmu_table_t* table, void* virtual_address) {
	unsigned int table_index = (((unsigned int)virtual_address) & 0x000FF000) >> 10;
	return (void*)(((unsigned int)table->address) + (table_index >> 18));
}

static mmu_table_t _mmu_get_second_level_table(mmu_table_t* table, void* virtual_address) {
	mmu_table_t second_table = { NULL, MMU_SMALL_PAGE, 0 };

	unsigned int* first_level_descriptor = _mmu_get_first_level_descriptor_address(table, virtual_address);

	if (((unsigned int)first_level_descriptor & MMU_PAGE_TABLE_DESCRIPTOR) != MMU_PAGE_TABLE_DESCRIPTOR) {
		/* create second level page table */
#if (RAM_MANAGER_PAGE_SIZE / MMU_SECOND_LEVEL_TABLE_SIZE) > 0
		static void* second_level_page_table_address = NULL;
		static uint8_t second_level_page_table_free = 0;

		if (second_level_page_table_free == 0) {
			second_level_page_table_address = ram_manager_reserve(MMU_SECOND_LEVEL_TABLE_SIZE);
			if (second_level_page_table_address == NULL) {
				return second_table;
			}

			second_table.address = second_level_page_table_address;
			second_level_page_table_free = (RAM_MANAGER_PAGE_SIZE / MMU_SECOND_LEVEL_TABLE_SIZE) - 1;
		} else {
			second_level_page_table_free--;
			second_table.address = second_level_page_table_address;
		}
		second_level_page_table_address = (void*)(((unsigned int) second_level_page_table_address) + MMU_SECOND_LEVEL_TABLE_SIZE);
#else
		second_table.address = ram_manager_reserve(MMU_SECOND_LEVEL_TABLE_SIZE);
		if (second_table.address == NULL) {
			return second_table;
		}
#endif

		unsigned int pte = ((unsigned int)second_table.address & MMU_PAGE_TABLE_MASK) | MMU_DOMAIN | MMU_PAGE_TABLE_DESCRIPTOR;
		*(first_level_descriptor) = pte;
	} else {
		/* second level page table already exists */
		second_table.address = (void*) ((unsigned int)first_level_descriptor & MMU_PAGE_TABLE_MASK);
	}

	return second_table;
}

static void _mmu_map_section(mmu_table_t* table, void* virtual_address, void* physical_address) {
	unsigned int pte = ((unsigned int)physical_address & MMU_SECTION_MASK) | MMU_FIRST_LEVEL_NOT_GLOBAL | MMU_FIRST_LEVEL_AP | MMU_DOMAIN | MMU_SECTION_DESCRIPTOR;
	unsigned int* pte_address = _mmu_get_first_level_descriptor_address(table, virtual_address);
	*(pte_address) = pte;
}

static uint8_t _mmu_map_small_page(mmu_table_t* table, void* virtual_address, void* physical_address) {
	mmu_table_t second_table = _mmu_get_second_level_table(table, virtual_address);
	if (second_table.address == NULL) {
		return 0;
	}

	unsigned int pte = ((unsigned int)physical_address & MMU_SMALL_PAGE_MASK) | MMU_SECOND_LEVEL_NOT_GLOBAL | MMU_SECOND_LEVEL_AP | MMU_SMALL_PAGE_DESCRIPTOR; /* TODO: set execute never bit */
	unsigned int* pte_address = _mmu_get_second_level_descriptor_address(&second_table, virtual_address);
	*(pte_address) = pte;

	return 1;
}

uint8_t mmu_map(mmu_table_t* table, void* virtual_address, void* physical_address) {
	if (table->kernel_table) {
		_mmu_map_section(table, virtual_address, physical_address);
		return 1;
	} else {
		return _mmu_map_small_page(table, virtual_address, physical_address);
	}
}

static uint8_t _mmu_load_section(mmu_table_t* table, void* virtual_address) {
	void* physical_address = ram_manager_reserve_aligned(MMU_SECTION_SIZE, MMU_SECTION_ALIGNMENT);
	if (physical_address == NULL) {
		return 0;
	}

	_mmu_map_section(table, virtual_address, physical_address);
	loader_load(virtual_address, MMU_SECTION_SIZE);

	return 1;
}

static uint8_t _mmu_load_small_page(mmu_table_t* table, void* virtual_address) {
	void* physical_address = ram_manager_reserve_aligned(MMU_SMALL_PAGE_SIZE, MMU_SMALL_PAGE_ALIGNMENT);
	if (physical_address == NULL || !_mmu_map_small_page(table, virtual_address, physical_address)) {
		return 0;
	}

	loader_load(virtual_address, MMU_SMALL_PAGE_SIZE);

	return 1;
}

uint8_t mmu_load(mmu_table_t* table, void* virtual_address) {
	if (table->kernel_table) {
		/* for the kernel we use only 1MB pages (sections) */
		return _mmu_load_section(table, virtual_address);
	} else {
		/* for user processes we use only 4KB pages (small pages) */
		return _mmu_load_small_page(table, virtual_address);
	}
}
