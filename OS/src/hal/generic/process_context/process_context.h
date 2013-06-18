/*
 * process_context.h
 *
 *  Created on: May 10, 2013
 *      Author: Bernhard
 */

#ifndef PROCESS_CONTEXT_H_
#define PROCESS_CONTEXT_H_

#include "../../../kernel/process/process.h"

void process_context_save();
void process_context_load();

void process_context_init_byfunc(Process_t* process, process_func_t func, uint8_t privileged);
void process_context_init_bybinary(Process_t* process, binary_t* binary);

#endif /* PROCESS_CONTEXT_H_ */
