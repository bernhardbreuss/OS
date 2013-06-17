/*
 * osstd.h
 *
 *  Created on: Jun 12, 2013
 *      Author: Bernhard
 */

#ifndef OSSTD_H_
#define OSSTD_H_

#include "device.h"
#include "driver.h"
#include <stdlib.h>
#include "process.h"

typedef struct {
	ProcessId_t driver;
	void* handle;
} handle_t;

int ioctl(Device_t device, driver_msg_t* buf, size_t size);
int open(Device_t device, driver_msg_t* buf, size_t size, handle_t* handle, driver_mode_t mode);
int close(handle_t* handle);
int write(handle_t* handle, driver_msg_t* buf, size_t size);
int read(handle_t* handle, driver_msg_t* buf, size_t size);

#endif /* OSSTD_H_ */
