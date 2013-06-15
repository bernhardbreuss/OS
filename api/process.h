/*
 * processs.h
 *
 *  Created on: 05.06.2013
 *      Author: Bernhard
 */

#ifndef API_PROCESS_H_
#define API_PROCESS_H_


#define PROCESS_ANY -2
#define INVALID_PROCESS_ID -1 /* TODO: rename with prefix PROCESS ... */
#define PROCESS_KERNEL 0

typedef int32_t ProcessId_t;
typedef int (*process_func_t)(int argc, char* argv[]);

#endif /* API_PROCESS_H_ */
