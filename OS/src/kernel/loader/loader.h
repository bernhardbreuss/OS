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

/**
 * Loads part of a binary into a given memory space. If the binary doesn't
 * contain information to load, these memory parts will stay unchanged.
 * Returns the number of used sections of the binary.
 */
uint32_t loader_load(binary_t* binary, void* address, size_t length);

#endif /* LOADER_H_ */
