/*
 * synctimer.h
 *
 *  Created on: 05.05.2013
 *      Author: Patrick
 *
 *
 *      Register Name 			Type 	Register Width (Bits) 	Offset Address 		32-kHz-Sync Timer Physical Address
 *		REG_32KSYNCNT_REV 		R 		32 						0x0000 				0x4832 0000
 *		REG_32KSYNCNT_SYSCONFIG R/W 	32 						0x0004 				0x4832 0004
 *		REG_32KSYNCNT_CR 		R 		32 						0x0010 				0x4832 0010
 */

#ifndef SYNCTIMER_H_
#define SYNCTIMER_H_

#define REG_32KSYNCNT_REV 			((unsigned int) 0x4832 0000)
#define REG_32KSYNCNT_SYSCONFIG 	((unsigned int) 0x4832 0004)
#define REG_32KSYNCNT_CR 			((unsigned int) 0x4832 0010)

#endif /* SYNCTIMER_H_ */
