/*
 * loader.c
 *
 *  Created on: May 19, 2013
 *      Author: Bernhard
 */

#include "loader.h"
#include <string.h>

uint32_t loader_load(binary_t* binary, void* address, size_t length) {
	binary_section_t* section;
	linked_list_node_t* node = binary->sections.head;
	uint32_t sections_loaded = 0;

	while (length > 0) {
		/* find corresponding section in binary*/
		while (node != NULL) {
			section = node->value;
			if ((unsigned int)section->mem_address <= ((unsigned int)address + length) && ((unsigned int)section->mem_address + section->mem_length) > (unsigned int)address) {
				/* section found */
				break;
			} else if (((uint8_t*)section->mem_address + length) > (uint8_t*)address) {
				/* section not found */
				node = NULL;
			} else {
				node = node->next;
			}
		}

		if (node != NULL) {
			if (section->mem_address > address) {
				/* there is an empty junk, which will be not initialized */
				length -= ((unsigned int)section->mem_address - (unsigned int)address);
				address = section->mem_address;
			}

			size_t loaded = ((uint32_t)address - (uint32_t)section->mem_address);
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
					memset(((uint8_t*)address + to_load), 0, (missing - to_load));
				}
			} else {
				/* we can load everything from file */
				to_load = missing;
			}

			if (to_load > 0 && !binary->read_function(binary->ident, address, file_offset, to_load)) {
				/* when reading failed make a "hard" error */
				return 0;
			}

			node = node->next;
			length -= missing;
			address = ((uint8_t*)address + missing);
			sections_loaded++;
		} else {
			break; /* there is no section from where we could load anything */
		}
	}

	return sections_loaded;
}
