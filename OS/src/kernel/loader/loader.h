/*
 * loader.h
 *
 *  Created on: May 19, 2013
 *      Author: Bernhard
 */

#ifndef LOADER_H_
#define LOADER_H_

#include <stdlib.h>
#include <inttypes.h>
#include "binary.h"
#include "../process/process.h"

typedef struct {
	Process_t* process;
	void* virtual_address;
	void* physical_address;
	size_t length;
} loader_load_t;

void loader_addload(void* virtual_address, void* physical_address, size_t length);
int loader_main(int argc, char* argv[]);

#endif /* LOADER_H_ */
