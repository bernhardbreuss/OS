/*
 * binary.h
 *
 *  Created on: May 18, 2013
 *      Author: Bernhard
 */

#ifndef BINARY_H_
#define BINARY_H_

#include <inttypes.h>
#include <linked_list.h>
#include <stdlib.h>

#define PROTECTION_FLAG_X	0x1
#define PROTECTION_FLAG_W	0x2
#define PROTECTION_FLAG_R	0x4

typedef uint32_t (*binary_read)(void* ident, void* dst, uint32_t offset, size_t length); /* TODO: offset and length should be the same type as used in the file lib */

typedef struct {
	void* ident;
	binary_read read_function;
	void* entry_point;
	linked_list_t sections;
} binary_t;

typedef struct {
	uint32_t file_offset;
	uint32_t file_length;
	void* mem_address;
	size_t mem_length;
	uint8_t protection_flags;
} binary_section_t;

#endif /* BINARY_H_ */
