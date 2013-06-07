/*
 * linked_list.h
 *
 *  Created on: May 9, 2013
 *      Author: Bernhard
 */

#ifndef LINKEDLIST_H_
#define LINKEDLIST_H_

typedef struct _linked_list_node_t linked_list_node_t;

typedef struct _linked_list_t {
	linked_list_node_t* head;
	linked_list_node_t* tail;
} linked_list_t;

struct _linked_list_node_t {
	void* value;
	linked_list_node_t* next;
	linked_list_node_t* prev;
};


void linked_list_init(linked_list_t* list);
linked_list_node_t* linked_list_add(linked_list_t* list, void* value);
linked_list_node_t* linked_list_add_node(linked_list_t* list, linked_list_node_t* new_node);
linked_list_node_t* linked_list_insert_begin(linked_list_t* list, void* value);
linked_list_node_t* linked_list_insert(linked_list_t* list, linked_list_node_t* node, void* value);
linked_list_node_t* linked_list_pop_head(linked_list_t* list);
linked_list_node_t* linked_list_pop_tail(linked_list_t* list);
void* linked_list_remove(linked_list_t* list, linked_list_node_t* node);
void linked_list_clear(linked_list_t* list);

#endif /* LINKEDLIST_H_ */
