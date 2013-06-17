/*
 * loader.c
 *
 *  Created on: May 19, 2013
 *      Author: Bernhard
 */

#include "loader.h"
#include <string.h>
#include "../process/process_manager.h"
#include <linked_list.h>
#include "../../hal/generic/mmu/mmu.h"

/**
 * Loads part of a binary into a given memory space. If the binary doesn't
 * contain information to load, these memory parts will stay unchanged.
 * Returns the number of used sections of the binary.
 */
static uint32_t _loader_load(binary_t* binary, void* virtual_address, void* physical_address, size_t length) {
	if (binary == NULL) {
		return 0;
	}

	binary_section_t* section;
	linked_list_node_t* node = binary->sections.head;
	uint32_t sections_loaded = 0;

	while (length > 0) {
		/* find corresponding section in binary*/
		while (node != NULL) {
			section = node->value;
			if ((unsigned int)section->mem_address <= ((unsigned int)virtual_address + length) && ((unsigned int)section->mem_address + section->mem_length) > (unsigned int)virtual_address) {
				/* section found */
				break;
			} else if (((uint8_t*)section->mem_address + length) > (uint8_t*)virtual_address) {
				/* section not found */
				node = NULL;
			} else {
				node = node->next;
			}
		}

		if (node != NULL) {
			if (section->mem_address > virtual_address) {
				/* there is an empty junk, which will be not initialized. skip it */
				length -= ((unsigned int)section->mem_address - (unsigned int)virtual_address);
				physical_address = ((uint8_t*)physical_address + ((uint8_t*)section->mem_address - (uint8_t*)virtual_address));
				virtual_address = section->mem_address;
			}

			size_t loaded = ((uint32_t)virtual_address - (uint32_t)section->mem_address);
			uint32_t file_offset = section->file_offset + loaded; /* TODO: file_offset should be the same type as used in the file sizes in the file lib */

			size_t missing;
			if ((loaded + length) > section->mem_length) {
				missing = (section->mem_length - loaded);
			} else {
				missing = length;
			}

			size_t to_load;
			if ((file_offset + missing) > (section->file_offset + section->file_length)) {
				/* it's required to load more than in the file */

				size_t file_loaded = (file_offset - section->file_offset);
				if (section->file_length > file_loaded) {
					/* there is still a little bit to load from the file */
					to_load = section->file_length - file_loaded;
				} else {
					/* there is nothing left to load from the file */
					to_load = 0;
				}

				if (to_load < missing) {
					/* set memory to 0 for the missing parts */
					memset(((uint8_t*)physical_address + to_load), 0, (missing - to_load));
				}
			} else {
				/* we can load everything from file */
				to_load = missing;
			}

			if (to_load > 0 && !binary->read_function(binary->ident, physical_address, file_offset, to_load)) {
				/* when reading failed make a "hard" error */
				return 0;
			}

			node = node->next;
			length -= missing;
			virtual_address = ((uint8_t*)virtual_address + missing);
			physical_address = ((uint8_t*)physical_address + missing);
			sections_loaded++;
		} else {
			break; /* there is no section from where we could load anything */
		}
	}

	return sections_loaded;
}

static linked_list_t list;
static Process_t* loader_process;

static uint8_t loading;

int loader_main(int argc, char* argv[]) {
	loader_process = process_manager_current_process;

	_disable_interrupts();
	linked_list_init(&list);
	while (1) {
		loading = 1;
		linked_list_node_t* node;
		while ((node = linked_list_pop_head(&list)) != NULL) {
			_enable_interrupts();

			loader_load_t* load = node->value;
			mmu_map(process_manager_current_process->page_table, load->physical_address, load->physical_address);
			_loader_load(load->process->binary, load->virtual_address, load->physical_address, load->length);

			_disable_interrupts();

			process_manager_set_process_ready(load->process);
			free(load);
			free(node);
		}

		loading = 0;
		process_manager_block_current_process();
		process_manager_run_process(NULL);
	}
}

void loader_addload(void* virtual_address, void* physical_address, size_t length) {
	loader_load_t* load = malloc(sizeof(loader_load_t));
	if (load == NULL) {
		/* TODO: kill process */
		while (1) ;
	}

	load->process = process_manager_current_process;
	load->virtual_address = virtual_address;
	load->physical_address = physical_address;
	load->length = length;
	linked_list_add(&list, load);

	process_manager_block_current_process(); /* <-- block user process which has a page fault */
	if (!loading) {
		process_manager_set_process_ready(loader_process); /* <-- start loader process */
	}
}
