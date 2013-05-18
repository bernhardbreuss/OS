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
} linked_list_t;

struct _linked_list_node_t {
	void* value;
	linked_list_node_t* next;
};


void linked_list_init(linked_list_t* list);
linked_list_node_t* linked_list_add(linked_list_t* list, void* value);
linked_list_node_t* linked_list_insert(linked_list_node_t* node, void* value);
void linked_list_clear(linked_list_t* list);

#endif /* LINKEDLIST_H_ */
