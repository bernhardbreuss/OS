/*
 * mmchs.h
 *
 *  Created on: 10.05.2013
 *      Author: lumannnn
 */

#ifndef OMAP3530_MMCHS_H_
#define OMAP3530_MMCHS_H_

typedef enum {
	UNKNOWN=0,
	MMC,
	SD_1x,
	SD_2,
	SD_2_HC,
	SDIO
} StorageType_t;

typedef enum {
	STREAM=0,
	BLOCK
} ProtocolType_t;

typedef struct _Protocol_t {
	ProtocolType_t type;
	int block_size;
} Protocol_t;

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

	StorageType_t storage_type;
	Protocol_t protocol_type;
} MMCHS_t;

#endif
