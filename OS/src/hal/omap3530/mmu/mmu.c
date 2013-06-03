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
#include "../../generic/mmu/mmu.h"
#include "../../../platform/platform.h"
#include "../../../kernel/mmu/ram_manager.h"
#include "../../../kernel/loader/loader.h"
#include "../../../kernel/process/process.h"
#include "../../../kernel/process/process_manager.h"

static mmu_table_t* kernel_master_table;

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
		mmu_ttbr_set1((unsigned int) table->address);
		mmu_ttbr_set0((unsigned int) table->address, 0);
		kernel_master_table = table;
	} else {
		table->address = ram_manager_reserve_aligned(MMU_FIRST_LEVEL_USER_TABLE_SIZE, MMU_PAGE_TABLE_USER_ALIGNMENT);
	}

	if (table->address == NULL) {
		free(table);
		return NULL;
	}

	/* ensure that new page table is mapped in the kernel page table by 1:1 mapping */
	mmu_map(kernel_master_table, table->address, table->address);

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
	return (void*)(((unsigned int)table->address) + table_index);
}

static mmu_table_t _mmu_get_second_level_table(mmu_table_t* table, void* virtual_address) {
	mmu_table_t second_table = { NULL, MMU_SMALL_PAGE, 0 };

	unsigned int* first_level_descriptor = _mmu_get_first_level_descriptor_address(table, virtual_address);

	if ((*(first_level_descriptor) & MMU_PAGE_TABLE_DESCRIPTOR) != MMU_PAGE_TABLE_DESCRIPTOR) {
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

		_mmu_init_pagetable(&second_table);

		unsigned int pte = ((unsigned int)second_table.address & MMU_PAGE_TABLE_MASK) | MMU_DOMAIN | MMU_PAGE_TABLE_DESCRIPTOR;
		*(first_level_descriptor) = pte;
	} else {
		/* second level page table already exists */
		second_table.address = (void*) ((unsigned int)first_level_descriptor & MMU_PAGE_TABLE_MASK);
	}

	return second_table;
}

static void _mmu_map_section(mmu_table_t* table, void* virtual_address, void* physical_address) {
	unsigned int pte = ((unsigned int)physical_address & MMU_SECTION_MASK) | MMU_DOMAIN | MMU_SECTION_DESCRIPTOR;
	if (table->kernel_table) {
		pte |= MMU_FIRST_LEVEL_KERNEL_AP;
	} else {
		pte |= MMU_FIRST_LEVEL_USER_AP;
	}

	unsigned int* pte_address = _mmu_get_first_level_descriptor_address(table, virtual_address);
	*(pte_address) = pte;
}

static unsigned int _mmu_map_small_page(mmu_table_t* table, void** virtual_address, void* physical_address) {
	*(virtual_address) = (void*)(((unsigned int)*(virtual_address)) & MMU_SMALL_PAGE_MASK);
	mmu_table_t second_table = _mmu_get_second_level_table(table, *(virtual_address));
	if (second_table.address == NULL) {
		return 0;
	}

	unsigned int pte = ((unsigned int)physical_address & MMU_SMALL_PAGE_MASK) | MMU_SECOND_LEVEL_NOT_GLOBAL | MMU_SECOND_LEVEL_AP | MMU_SMALL_PAGE_DESCRIPTOR; /* TODO: set execute never bit */
	unsigned int* pte_address = _mmu_get_second_level_descriptor_address(&second_table, *(virtual_address));
	*(pte_address) = pte;

	return MMU_SMALL_PAGE_SIZE;
}

unsigned int mmu_map(mmu_table_t* table, void* virtual_address, void* physical_address) {
	if (table->kernel_table) {
		_mmu_map_section(table, virtual_address, physical_address);
		return MMU_SECTION_SIZE;
	} else {
		return _mmu_map_small_page(table, &virtual_address, physical_address);
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

static unsigned int _mmu_reserve_small_page(mmu_table_t* table, void** virtual_address) {
	void* physical_address = ram_manager_reserve_aligned(MMU_SMALL_PAGE_SIZE, MMU_SMALL_PAGE_ALIGNMENT);
	if (physical_address == NULL || !_mmu_map_small_page(table, virtual_address, physical_address)) {
		/* TODO: free reserved ram */
		return 0;
	}

	//FIXME: loader_load(virtual_address, MMU_SMALL_PAGE_SIZE);


	return MMU_SMALL_PAGE_SIZE;
}

void mmu_activate_process(Process_t* process) {
	mmu_table_t* table = process->page_table;

	if (table == NULL) {
		return;
	}

	if (table->kernel_table) {
		/* the kernel is always activated (TTBR1) */
		return;
	}

	unsigned int ttbr = (unsigned int)table->address;
	unsigned int contextidr = (unsigned int)process->pid; /* TODO: ASID are the lower 8 bit. it must be ensured that in the TLB the ASID is unique to the new process */
	mmu_ttbr_set0(ttbr, contextidr);
}

unsigned int _mmu_handle_abort(unsigned int status, void** virtual) {
	status &=  MMU_ABORT_MASK;

	void* virtual_address = *(virtual);

	if (status == MMU_ABORT_DEBUG) {
		logger_debug("abort was a debug event.");
		return 0;
	} else {
		mmu_table_t* table;
		if ((unsigned int)virtual_address < 0x10000000U) {
			table = process_manager_current_process->page_table;
		} else {
			/* TODO: check if SPSR says that the abort wasn't in user mode */
			table = kernel_master_table;
		}
		if (table != NULL) {
			switch (status) {
			case MMU_ABORT_TRANSLATION_FAULT_SECTION:
				if (table->kernel_table) {
					_mmu_load_section(table, virtual_address); /* TODO: handle to less memory */
					logger_debug("section for address 0x%08X loaded for process %s", virtual_address, process_manager_current_process->name);
					return; /* FIXME: return mapped size */
				}
			case MMU_ABORT_TRANSLATION_FAULT_PAGE: /* when a user process has a fault for a section, it's a hidden fault for a page because the mmu can't know it should be a page fault */
				return _mmu_reserve_small_page(table, virtual); /* TODO: else handle to less memory */
			}
		}
	}

	logger_error("Process %s should be killed because of access to 0x%08X, status 0x%08X.", process_manager_current_process->name, virtual_address, status);
	_enable_interrupts();
	while (1) ;
	/* TODO: kill process, clean SP of abort mode */

	return 0;
}

/*#pragma INTERRUPT(pabt_handler, PABT);
interrupt void pabt_handler() {
	logger_warn("KERNEL INFO: Prefetch abort.");

	unsigned int ifsr = mmu_get_ifsr();
	void* ifar = mmu_get_ifar();

	_mmu_handle_abort(ifsr, ifar);
}

#pragma INTERRUPT(dabt_handler, DABT);
interrupt void dabt_handler() {
	logger_warn("KERNEL INFO: data abort");

	unsigned int dfsr = mmu_get_dfsr();
	void* dfar = mmu_get_dfar();

	_mmu_handle_abort(dfsr, dfar);
}*/
