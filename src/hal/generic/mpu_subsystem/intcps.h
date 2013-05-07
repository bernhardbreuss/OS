/*
 * intcps.h
 *
 *  Created on: 03.05.2013
 *      Author: edi
 */

#ifndef INTCPS_H_
#define INTCPS_H_

#include "../../omap3530/timer/gptimer.h"
#include "../../omap3530/mpu_subsystem/intcps.h"

/**
 * Call this to activate the interrupts for this timer in the mpu subsystem.
 */
void intcps_activate_gptimer(const gptimer_t* const timer);

#endif /* INTCPS_H_ */
