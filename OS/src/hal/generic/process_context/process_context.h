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

uint8_t process_context_init(Process_t* process);

void process_context_init_byfunc(Process_t* process, process_func func);
void process_context_init_bybinary(Process_t* process, binary_t* binary);

#endif /* PROCESS_CONTEXT_H_ */
