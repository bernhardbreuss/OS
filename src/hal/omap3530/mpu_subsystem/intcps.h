/*
 * intcps.h
 *
 *  Created on: 03.05.2013
 *      Author: edi
 */

#ifndef OMAP3530_INTCPS_H_
#define OMAP3530_INTCPS_H_

// OMAP3530x.pdf page 1079 - register overview
#define INTCPS_REGISTER_N_BIT_SIZE		0x20					//32 bits

//This register is used to clear the interrupt mask bits. page 1087
#define INTCPS_MIR_CLEARn				0x48200088
//his register supplies the currently active IRQ interrupt number. page 1081
#define INTCPS_SIR_IRQ					0x48200040
//This register supplies the currently active FIQ interrupt number. page 1082
#define NTCPS_SIR_FIQ 					0x48200044
//This register contains the interrupt mask. page 1086
#define INTCPS_MIRn						0x48200084
//This register is used to set the interrupt mask bits. page 1087
#define INTCPS_MIR_SETn					0x4820008C

//OMAP3530x.pdf page 1063
#define GPTIMER1_INTCPS_MAPPING_ID 			37
#define GPTIMER2_INTCPS_MAPPING_ID 			38
#define GPTIMER3_INTCPS_MAPPING_ID 			39
#define GPTIMER4_INTCPS_MAPPING_ID 			40
#define GPTIMER5_INTCPS_MAPPING_ID 			41
#define GPTIMER6_INTCPS_MAPPING_ID 			42
#define GPTIMER7_INTCPS_MAPPING_ID 			43
#define GPTIMER8_INTCPS_MAPPING_ID 			44
#define GPTIMER9_INTCPS_MAPPING_ID 			45
#define GPTIMER10_INTCPS_MAPPING_ID 		46
#define GPTIMER11_INTCPS_MAPPING_ID 		47

/**
 * Interrupt Mapping to the MPU System
 * Omap3530x.pdf page 1063
 */
typedef short intcps_gptimer_mapping_t;


#endif /* INTCPS_H_ */
