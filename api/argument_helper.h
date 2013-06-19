/*
 * argumeht_helper.h
 *
 *  Created on: Jun 18, 2013
 *      Author: Patrick
 */

#ifndef ARGUMEHT_HELPER_H_
#define ARGUMEHT_HELPER_H_

#define ARGUMENTS_MAX_LENGTH 500 /* 512 - 3*4 */

extern void* ARGS_ADDR;
extern void* ARGV_ADDR;

int argument_helper_parse(void);

#endif /* ARGUMEHT_HELPER_H_ */
