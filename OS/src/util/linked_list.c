/*
 * linked_list.c
 *
 *  Created on: May 9, 2013
 *      Author: Bernhard
 */

#include "linked_list.h"
#include <stdlib.h>

linked_list_node_t* get_node(void* value) {
	linked_list_node_t* node = malloc(sizeof(linked_list_node_t));

	if (node == NULL) {
		return NULL;
	}

	node->next = NULL;
	node->value = value;

	return node;
}

void linked_list_init(linked_list_t* list) {
	list->head = NULL;
}

linked_list_node_t* linked_list_add(linked_list_t* list, void* value) {
	linked_list_node_t* new_node = get_node(value);
	if (new_node == NULL) {
		return NULL;
	}

	if (list->head == NULL) { /* empty list */
		list->head = new_node;
	} else {
		linked_list_node_t* node = list->head;
		while (node->next != NULL) {
			node = node->next;
		}
		node->next = new_node;
	}

	return new_node;
}

linked_list_node_t* linked_list_insert(linked_list_node_t* node, void* value) {
	linked_list_node_t* new_node = get_node(value);
	if (new_node == NULL) {
		return NULL;
	}

	new_node->next = node->next;
	node->next = new_node;

	return new_node;
}

void linked_list_clear(linked_list_t* list) {
	linked_list_node_t* node = list->head;
	linked_list_node_t* next_node;

	while (node != NULL) {
		free(node->value);
		next_node = node->next;
		free(node);
		node = next_node;
	}

	list->head = NULL;
}
