/*
 * elf.c
 *
 *  Created on: May 18, 2013
 *      Author: Bernhard
 */

#include "elf.h"
#include "binary.h"

binary_t* elf_init(void* ident, binary_read read_function) {
	binary_t* binary = malloc(sizeof(binary_t));
	if (binary == NULL) {
		return NULL;
	}

	linked_list_init(&binary->sections);
	binary->ident = ident;
	binary->read_function = read_function;

	Elf_Ehdr header;
	if (!binary->read_function(binary->ident, &header, 0, sizeof(Elf_Ehdr))) {
		elf_close(binary);
		return NULL;
	}

	/* check for ELF magic header, 32-bit, MSB/LSB, executable, program header table */
	if (header.e_ident[EI_MAG0] != 0x7f ||
			header.e_ident[EI_MAG1] != 'E' ||
			header.e_ident[EI_MAG2] != 'L' ||
			header.e_ident[EI_MAG3] != 'F' ||
			header.e_ident[EI_CLASS] != ELFCLASS ||
			header.e_ident[EI_DATA] != ELFDATA ||
			header.e_type != ET_EXEC ||
			header.e_machine != ELFMACHINE ||
			header.e_phoff == 0) {

		elf_close(binary);
		return NULL;
	}

	Elf32_Phdr programHeader;
	int i;
	for (i = 0; i < header.e_phnum; i++) {
		if (!binary->read_function(binary->ident, &programHeader, (header.e_phoff + (i * header.e_phentsize)), sizeof(Elf32_Phdr))) {
			elf_close(binary);
			return NULL;
		}

		if (programHeader.p_type == PT_LOAD) {
			binary_section_t* section = malloc(sizeof(binary_section_t));
			if (section == NULL) {
				elf_close(binary);
				return NULL;
			}

			section->file_offset = programHeader.p_offset;
			section->file_length = programHeader.p_filesz;
			section->mem_address = programHeader.p_vaddr;
			section->mem_length = programHeader.p_memsz;

			section->protection_flags = 0;
			if (programHeader.p_flags & PF_R) {
				section->protection_flags |= PROTECTION_FLAG_R;
			}
			if (programHeader.p_flags & PF_W) {
				section->protection_flags |= PROTECTION_FLAG_W;
			}
			if (programHeader.p_flags & PF_X) {
				section->protection_flags |= PROTECTION_FLAG_X;
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
				node = linked_list_insert(node, section);
			}
			if (node == NULL) {
				elf_close(binary);
				return NULL;
			}
		}
	}

	binary->entry_point = header.e_entry;

	return binary;
}

void elf_close(binary_t* binary) {
	linked_list_clear(&binary->sections);
	free(binary);
}
