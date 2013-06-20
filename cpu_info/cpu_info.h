/*
 * cpu_info.h
 *
 *  Created on: 20.06.2013
 *      Author: lumannnn
 */

#ifndef CPU_INFO_H_
#define CPU_INFO_H_

#define PRM_CLKSEL				((unsigned int) 0x48306D40)
#define CM_CLKSEL1_PLL_MPU		((unsigned int) 0x48004940)	// mpu
#define CM_CLKSEL1_PLL			((unsigned int) 0x48004D40)	// core

/*
 * Calculates the currently set CORE_DPLL_CLK
 * See OMAP35x.pdf, page 312.
 */
int cpu_info_calk_clkoutX2();

#endif /* CPU_INFO_H_ */
