/*
 * osx.c
 *
 *  Created on: Jun 15, 2013
 *      Author: Stephan
 */

#include "osx.h"
#include <inttypes.h>

binary_t* osx_init(void* ident, binary_read read_function) {
	binary_t* binary = malloc(sizeof(binary_t));
	if (binary == NULL) {
		return NULL;
	}

	linked_list_init(&binary->sections);
	binary->ident = ident;
	binary->read_function = read_function;

	if (!read_function(ident, &binary->entry_point, 0, 4)) {
		osx_close(binary);
		return NULL;
	}

	uint8_t num_sections;
	if (!read_function(ident, &num_sections, 4, 1)) {
		osx_close(binary);
		return NULL;
	}

	int i;
	size_t offset = 5;
	for (i = 0; i < num_sections; i++) {
		binary_section_t* section = malloc(sizeof(binary_section_t));
		if (section == NULL) {
			osx_close(binary);
			return NULL;
		}

		if (!read_function(ident, section, offset, sizeof(binary_section_t))) {
			osx_close(binary);
			return NULL;
		}


		/* insert sorted */

		linked_list_node_t* node = binary->sections.head;
		if (node == NULL) {
			node = linked_list_add(&binary->sections, section);
		} else if (((binary_section_t*)node->value)->mem_address > section->mem_address) {
			node = linked_list_insert_begin(&binary->sections, section);
		} else {
			while (node->next != NULL) {
				if (((binary_section_t*)node->next->value)->mem_address > section->mem_address) {
					break;
				}
				node = node->next;
			}
			node = linked_list_insert(&binary->sections, node, section);
		}
		if (node == NULL) {
			osx_close(binary);
			return NULL;
		}

		offset += 17;
	}

	return binary;
}

void osx_close(binary_t* binary) {
	linked_list_clear(&binary->sections);
	free(binary);
}

