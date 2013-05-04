/*
 * irq.h
 *
 *  Created on: May 2, 2013
 *      Author: Bernhard
 */

#ifndef OMAP3530_IRQ_H_
#define OMAP3530_IRQ_H_

#include "../process.h"

extern void* irq_saved_context[PROCESS_CONTEXT_SIZE];

#pragma INTERRUPT(irq_handler, IRQ);
#pragma TASK(irq_handler)
interrupt void irq_handler();

#endif /* OMAP3530_IRQ_H_ */
