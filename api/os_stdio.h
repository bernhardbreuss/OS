/*
 * osstd.h
 *
 *  Created on: Jun 12, 2013
 *      Author: Bernhard
 */

#ifndef OS_STDIO_H_
#define OS_STDIO_H_

#include "device.h"
#include "driver.h"
#include <stdlib.h>
#include "process.h"

typedef struct {
	ProcessId_t driver;
	void* handle;
} handle_t;

int os_ioctl(Device_t device, driver_msg_t* buf, size_t size);
int os_open(Device_t device, driver_msg_t* buf, size_t size, handle_t* handle, driver_mode_t mode);
int os_close(handle_t* handle);
int os_write(handle_t* handle, driver_msg_t* buf, size_t size);
int os_read(handle_t* handle, driver_msg_t* buf, size_t size);

#endif /* OS_STDIO_H_ */
