/*
 * binary_map.c
 *
 *  Created on: 19.06.2013
 *      Author: lumannnn
 */


#include "kernel/loader/binary.h"
#include "binary_map.h"
#include <linked_list.h>
#include <stdlib.h>
#include <string.h>

int binary_map_init(binary_map_t* map) {
	linked_list_t* _map = malloc(sizeof(linked_list_t));
	if (_map == NULL) {
		return ERROR;
	}

	linked_list_init(_map);
	map->map = _map;

	return SUCCESS;
}

int binary_map_add(binary_map_t* map, char* name, binary_t* binary) {
	if (map == NULL) {
		return ERROR;
	}

	binary_map_entry_t* map_entry = malloc(sizeof(binary_map_entry_t));
	if (map_entry == NULL) {
		return ERROR;
	}

	map_entry->name = name; 	// should we copy it?
	map_entry->binary = binary;

	linked_list_node_t* result = linked_list_add(map->map, map_entry);
	if (result == NULL) {
		return ERROR;
	}

	return SUCCESS;
}

binary_t* binary_map_get_binary(binary_map_t* map, char* name) {
	linked_list_t* _map = map->map;

	linked_list_node_t* node = _map->head;
	while (node != NULL) {
		binary_map_entry_t* map_entry = node->value;
		if (strcmp(map_entry->name, name) == 0) {
			return map_entry->binary;
		}

		node = node->next;
	}

	return NULL;
}
