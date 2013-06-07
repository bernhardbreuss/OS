/*
 * kernel.h
 *
 *  Created on: Jun 4, 2013
 *      Author: Bernhard
 */

#ifndef KERNEL_H_
#define KERNEL_H_

#define KERNEL_START_PROCESS 0
#define KERNEL_START_PROCESS_BYFUNC 0
#define KERNEL_START_PROCESS_BYPATH 1
#define MEM_IO_READ 16
#define MEM_IO_READ 17

#define KERNEL_ERROR 0
#define KERNEL_OK 1

void kernel_main_loop(void);

#endif /* KERNEL_H_ */
