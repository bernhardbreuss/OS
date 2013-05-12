/*
 * driver_manager.h
 *
 *  Created on: May 9, 2013
 *      Author: Bernhard
 */

#ifndef DRIVER_MANAGER_H_
#define DRIVER_MANAGER_H_

#include "driver.h"

void driver_manager_init(void);
void driver_manager_add_driver(Device_t device, Driver_t* driver, ProcessId_t (*start_process)(Device_t device));
ProcessId_t driver_manager_get_process(Device_t device);




#endif /* DRIVER_MANAGER_H_ */
