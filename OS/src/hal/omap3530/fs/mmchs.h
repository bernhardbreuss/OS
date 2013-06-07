/*
 * mmchs.h
 *
 *  Created on: 10.05.2013
 *      Author: lumannnn
 */

#ifndef OMAP3530_MMCHS_H_
#define OMAP3530_MMCHS_H_

typedef struct _MMCHS_t {
	unsigned int* volatile SYSCONFIG;
	unsigned int* volatile SYSSTATUS;
	unsigned int* volatile CSRE;
	unsigned int* volatile CON;
	unsigned int* volatile BLK;
	unsigned int* volatile ARG;
	unsigned int* volatile CMD;
	unsigned int* volatile RSP10;
	unsigned int* volatile RSP32;
	unsigned int* volatile RSP54;
	unsigned int* volatile RSP76;
	unsigned int* volatile DATA;
	unsigned int* volatile PSTATE;
	unsigned int* volatile HCTL;
	unsigned int* volatile SYSCTL;
	unsigned int* volatile STAT;
	unsigned int* volatile IE;
	unsigned int* volatile ISE;
	unsigned int* volatile AC12;
	unsigned int* volatile CAPA;
} MMCHS_t;

#endif
