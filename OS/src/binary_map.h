/*
 * binary_mapping.h
 *
 *  Created on: 19.06.2013
 *      Author: lumannnn
 */

#ifndef BINARY_MAPPING_H_
#define BINARY_MAPPING_H_

#include "kernel/loader/binary.h"
#include <linked_list.h>

#define ERROR 	(-1)
#define SUCCESS	(0)

typedef struct _binary_map_entry {
	char* name;
	binary_t* binary;
} binary_map_entry_t;

typedef struct _binary_map {
	linked_list_t* map;
} binary_map_t;


int binary_map_init(binary_map_t* map);
/**
 * map <code>binary</code> to name <code>name</code>.
 */
int binary_map_add(binary_map_t* map, char* name, binary_t* binary);

/**
 * get mapped binary for name <code>name</code>.
 */
binary_t* binary_map_get_binary(binary_map_t*, char* name);

#endif /* BINARY_MAPPING_H_ */
