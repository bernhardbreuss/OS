/*
 * prcm.h
 *
 * power resource clock management
 *
 *  Created on: 07.05.2013
 *      Author: edi
 */

#ifndef PRCM
#define PRCM

//clock selection per domain (gptimer 2 to 9)
//OMAP35x.pdf page 503
//see also 16.2.1 page 2611
#define PRCM_CM_CLKSEL_PER 				((unsigned int*) 0x48005040)

#endif /* POWER_RESOURCE_CLOCK_MANAGEMENT_H_ */
