/*
 * irq.h
 *
 *  Created on: May 2, 2013
 *      Author: Bernhard
 */

#ifndef OMAP3530_IRQ_H_
#define OMAP3530_IRQ_H_

#include "../mpu_subsystem/intcps.h"

#pragma INTERRUPT(irq_handler, IRQ);
#pragma TASK(irq_handler)
interrupt void irq_handler();

void irq_init_handles(void);
void irq_handle_interrupt(void);

#endif /* OMAP3530_IRQ_H_ */
