/*
 * ram_manager.h
 *
 *  Created on: May 24, 2013
 *      Author: Bernhard
 */

#ifndef RAM_MANAGER_H_
#define RAM_MANAGER_H_

#include <stdlib.h>

void* ram_manager_reserve(size_t size);
void* ram_manager_reserve_aligned(size_t size, unsigned int alignment);
void ram_manager_init(void);

#endif /* RAM_MANAGER_H_ */
