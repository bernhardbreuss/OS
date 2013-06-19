/*
 * osx.h
 *
 *  Created on: Jun 15, 2013
 *      Author: Stephan
 */

#ifndef OSX_H_
#define OSX_H_

#include "binary.h"

binary_t* osx_init(void* ident, binary_read read_function);
void osx_close(binary_t* binary);

#endif /* OSX_H_ */
