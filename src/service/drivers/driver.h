/*
 * drivers.h
 *
 * Defines driver type implicit definition of syscalls open/close/read/write
 *
 *  Created on: 02.05.2013
 *      Author: Stephan
 */

#ifndef DRIVERS_H_
#define DRIVERS_H_

/*
 * struct for typedef driver
 */

typedef struct driver_t{
	uint32_t (*ioctl) (); //device depentend controll
	uint32_t (*open) (); //open driver file
	uint32_t (*close) (); //close driver file
	uint32_t (*read) (); //read from driver file
	uint32_t (*write) (); //write to driver file
	uint32_t (*create) (); //inits the driver server
}driver_t;

#endif /* DRIVERS_H_ */
