/*
 * driver.h
 *
 *  Created on: May 9, 2013
 *      Author: Bernhard
 */

#ifndef DRIVER_H_
#define DRIVER_H_

#include <inttypes.h>
#include <stdlib.h>
#include "message.h"
#include "device.h"

#define DRIVER_BUFFER_SIZE (MESSAGE_BUFFER_SIZE - (sizeof(unsigned int) * 3))
#define DRIVER_DATA_SIZE (MESSAGE_DATA_SIZE - 3)


typedef enum {
	DRIVER_MODE_READ = 0x1,
	DRIVER_MODE_WRITE = 0x2,
	DRIVER_MODE_READ_WRITE = 0x3
} driver_mode_t;

typedef union {
	char buffer[DRIVER_BUFFER_SIZE];
	unsigned int data[DRIVER_DATA_SIZE];
} driver_msg_t;

#define DRIVER_IOCTL 0
#define DRIVER_OPEN 1
#define DRIVER_CLOSE 2
#define DRIVER_READ 3
#define DRIVER_WRITE 4

typedef struct _Driver_t {
	int (*ioctl) (driver_msg_t* buf, size_t size); //device dependent control
	void* (*open) (driver_msg_t* buf, size_t size, driver_mode_t mode); //open driver file
	int (*close) (void* handle); //close driver file
	int (*read) (void* handle, driver_msg_t* buf, size_t size); //read from driver file
	int (*write) (void* handle, driver_msg_t* buf, size_t size); //write to driver file
} Driver_t;

extern Driver_t driver;

void driver_init(void (*async_loop)(void));

#endif /* DRIVER_H_ */
