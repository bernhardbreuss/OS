/*
 * linked_list.c
 *
 *  Created on: May 9, 2013
 *      Author: Bernhard
 */

#include "linked_list.h"
#include <stdlib.h>

static linked_list_node_t* get_node(void* value) {
	linked_list_node_t* node = malloc(sizeof(linked_list_node_t));

	if (node == NULL) {
		return NULL;
	}

	node->value = value;

	return node;
}

void linked_list_init(linked_list_t* list) {
	list->head = NULL;
	list->tail = NULL;
}

linked_list_node_t* linked_list_add(linked_list_t* list, void* value) {
	linked_list_node_t* new_node = get_node(value);
	if (new_node == NULL) {
		return NULL;
	}

	return linked_list_add_node(list, new_node);
}

linked_list_node_t* linked_list_add_node(linked_list_t* list, linked_list_node_t* new_node) {
	new_node->next = NULL;
	new_node->prev = list->tail;
	if (list->head == NULL) {
		/* empty list */
		list->head = new_node;
	} else {
		list->tail->next = new_node;
	}
	list->tail = new_node;

	return new_node;
}

linked_list_node_t* linked_list_insert_begin(linked_list_t* list, void* value) {
	linked_list_node_t* new_node = get_node(value);
	if (new_node == NULL) {
		return NULL;
	}

	new_node->next = list->head;
	new_node->prev = NULL;
	if (list->head == NULL) {
		/* empty list */
		list->tail = new_node;
		new_node->next = NULL;
	} else {
		list->head->prev = new_node;
	}
	list->head = new_node;

	return new_node;
}

linked_list_node_t* linked_list_insert(linked_list_t* list, linked_list_node_t* node, void* value) {
	linked_list_node_t* new_node = get_node(value);
	if (new_node == NULL) {
		return NULL;
	}

	new_node->next = node->next;
	new_node->prev = node;
	node->next = new_node;

	if (list->tail == node) {
		list->tail = new_node;
	} else {
		new_node->next->prev = new_node;
	}

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
	list->tail = NULL;
}

linked_list_node_t* linked_list_pop_head(linked_list_t* list) {
	if (list->head == NULL) {
		return NULL;
	}

	linked_list_node_t* node = list->head;
	list->head = node->next;
	if (list->head == NULL) {
		/* list is now empty */
		list->tail = NULL;
	} else {
		list->head->prev = NULL;
	}

	return node;
}

linked_list_node_t* linked_list_pop_tail(linked_list_t* list) {
	if (list->tail == NULL) {
		return NULL;
	}

	linked_list_node_t* node = list->tail;
	list->tail = node->prev;
	if (list->tail == NULL) {
		/* list is now empty */
		list->head = NULL;
	} else {
		list->tail->next = NULL;
	}

	return node;
}

void* linked_list_remove(linked_list_t* list, linked_list_node_t* node) {
	if (list->head == node) {
		/* first element */
		list->head = node->next;
	} else {
		node->prev->next = node->next;
	}
	if (list->tail == node) {
		/* last element */
		list->tail = node->prev;
	} else {
		node->next->prev = node->prev;
	}

	void* value = node->value;
	free(node);
	return value;
}
