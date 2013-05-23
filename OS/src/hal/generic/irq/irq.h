/*
 * irq.h
 *
 *  Created on: 13.05.2013
 *      Author: edi
 */

#ifndef IRQ_H_
#define IRQ_H_

#include "../../omap3530/mpu_subsystem/intcps.h"

void irq_add_handler(intcps_mapping_id_t id, void(*irq_handle_func)(void));

#endif /* IRQ_H_ */
