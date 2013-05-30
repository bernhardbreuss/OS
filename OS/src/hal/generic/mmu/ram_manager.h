/*
 * ram_manager.h
 *
 *  Created on: May 26, 2013
 *      Author: Bernhard
 */

#ifndef GENERIC_RAM_MANAGER_H_
#define GENERIC_RAM_MANAGER_H_

#include <inttypes.h>

uint32_t ram_manager_align_index(unsigned int alignment, unsigned int* index, uint32_t* bit);
void* ram_manager_get_address(uint32_t index, uint32_t bit);

#endif /* GENERIC_RAM_MANAGER_H_ */
