/*
 * process_context.h
 *
 *  Created on: May 10, 2013
 *      Author: Bernhard
 */

#ifndef PROCESS_CONTEXT_H_
#define PROCESS_CONTEXT_H_

#include "../../../kernel/process.h"
#include "../../platform.h"

void process_context_save();
void process_context_load();

uint8_t process_context_init(Process_t* process);

#endif /* PROCESS_CONTEXT_H_ */
