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
#include <message.h>

typedef struct _Driver_t {
	const char* id;
	uint32_t (*ioctl) (message_t* msg); //device dependent control
	uint32_t (*open) (message_t* msg); //open driver file
	uint32_t (*close) (message_t* msg); //close driver file
	uint32_t (*read) (message_t* msg); //read from driver file
	uint32_t (*write) (message_t* msg); //write to driver file
} Driver_t;

#endif /* DRIVER_H_ */
