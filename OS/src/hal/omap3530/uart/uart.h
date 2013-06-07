/*
 * uart.h
 *
 *  Created on: 24.05.2013
 *      Author: füssi CoOp edi
 */

#ifndef UART_OMAP3530_H_
#define UART_OMAP3530_H_

#include "../../../service/logger/logger.h"
#include "../../generic/uart/uart.h"
#include "../../../bit.h"
#include <stdlib.h>

#define UART1 		((unsigned int) 0x4806A000)
#define UART_NR_1		1
#define UART2 		((unsigned int) 0x4806C000)
#define UART_NR_2		2
#define UART3 		((unsigned int) 0x49020000)
#define UART_NR_3		3

#define UART_DLL_REG_OFFSET  			0x0
#define UART_RHR_REG_OFFSET  			0x0
#define UART_THR_REG_OFFSET  			0x0
#define UART_DLH_REG_OFFSET  			0x004
#define UART_IER_REG_OFFSET  			0x004
#define UART_IIR_REG_OFFSET  			0x008
#define UART_FCR_REG_OFFSET  			0x008
#define UART_EFR_REG_OFFSET  			0x008
#define UART_LCR_REG_OFFSET  			0x00C
#define UART_MCR_REG_OFFSET  			0x010
#define UART_XON1_ADDR1_REG_OFFSET  	0x010
#define UART_LSR_REG_OFFSET  			0x014
#define UART_XON2_ADDR2_REG_OFFSET  	0x014
#define UART_MSR_REG_OFFSET  			0x018
#define UART_TCR_REG_OFFSET  			0x018
#define UART_XOFF1_REG_OFFSET  			0x018
#define UART_SPR_REG_OFFSET  			0x01C
#define UART_TLR_REG_OFFSET  			0x01C
#define UART_XOFF2_REG_OFFSET  			0x01C
#define UART_MDR1_REG_OFFSET  			0x020
#define UART_MDR2_REG_OFFSET  			0x024
#define UART_SFLSR_REG_OFFSET  			0x028
#define UART_TXFLL_REG_OFFSET  			0x028
#define UART_RESUME_REG_OFFSET  		0x02C
#define UART_TXFLH_REG_OFFSET  			0x02C
#define UART_SFREGL_REG_OFFSET  		0x030
#define UART_RXFLL_REG_OFFSET  			0x030
#define UART_SFREGH_REG_OFFSET  		0x034
#define UART_RXFLH_REG_OFFSET  			0x034
#define UART_UASR_REG_OFFSET  			0x038
#define UART_BLR_REG_OFFSET  			0x038
#define UART_ACREG_REG_OFFSET  			0x03C
#define UART_SCR_REG_OFFSET  			0x040
#define UART_SSR_REG_OFFSET  			0x044
#define UART_EBLR_REG_OFFSET  			0x048
#define UART_MVR_REG_OFFSET  			0x050
#define UART_SYSC_REG_OFFSET  			0x054 /* System configuration REG_OFFSETister. */
#define UART_SYSS_REG_OFFSET  			0x058 /* System status REG_OFFSETister */
#define UART_WER_REG_OFFSET  			0x05C
#define UART_CFPS_REG_OFFSET  			0x060

#endif /* UART_OMAP3530_H_ */
