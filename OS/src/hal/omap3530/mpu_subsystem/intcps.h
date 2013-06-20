/*
 * intcps.h
 *
 *  interrupt control processing system
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
//This register supplies the currently active IRQ interrupt number. page 1082
#define INTCPS_SIR_FIQ 					0x48200044
//This register contains the interrupt mask. Omap3530x.pdf page 1086
//disabled interrupt lines are masked, which means they are set to 1
#define INTCPS_MIRn						0x48200084
//This register is used to set the interrupt mask bits. page 1087
#define INTCPS_MIR_SETn					0x4820008C
//This register is used to set the interrupt agreement bits.
#define INTCPS_CONTROL					0x48200048


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

#define UART1_INTCPS_MAPPING_ID				72
#define UART2_INTCPS_MAPPING_ID				73
#define UART3_INTCPS_MAPPING_ID				74


/*
 * Call this to activate the interrupt line for this interrupt line
 * associated with this inctps_mapping_id
 */
void intcps_active_interrupt_line(int interrupt_line_id);

/*
 * Call this to enable generation of new interrupt requests, after an interrupt has been handled.
 * Omap3530x.pdf page 1083
 */
void intcps_enable_new_irq_generation(void);

#endif /* INTCPS_H_ */
