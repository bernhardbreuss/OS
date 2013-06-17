/*
 * processs.h
 *
 *  Created on: 05.06.2013
 *      Author: Bernhard
 */

#ifndef API_PROCESS_H_
#define API_PROCESS_H_

#include <inttypes.h>

#define PROCESS_ANY -2
#define INVALID_PROCESS_ID -1 /* TODO: rename with prefix PROCESS ... */
#define PROCESS_KERNEL 0
#define PROCESS_DRIVER_MANAGER 3

#define PROCESS_DRIVER_MANAGER_NAME "Driver manager"

#define PROCESS_MAX_NAME_LENGTH 64

typedef int32_t ProcessId_t;
typedef char* process_name_t;
typedef int (*process_func_t)(int argc, char* argv[]);

ProcessId_t process_find(process_name_t name);

#endif /* API_PROCESS_H_ */
