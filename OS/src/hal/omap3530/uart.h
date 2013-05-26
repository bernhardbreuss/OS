/*
 * uart.h
 *
 *  Created on: 24.05.2013
 *      Author: edi
 */

#ifndef OMAP3530_UART_PLATFORM_H_
#define OMAP3530_UART_PLATFORM_H_

typedef struct _uart_t {
	volatile unsigned int* DLL_REG;
	volatile unsigned int* RHR_REG;
	volatile unsigned int* THR_REG;
	volatile unsigned int* DLH_REG;
	volatile unsigned int* IER_REG;
	volatile unsigned int* IIR_REG;
	volatile unsigned int* FCR_REG;
	volatile unsigned int* EFR_REG;
	volatile unsigned int* LCR_REG;
	volatile unsigned int* MCR_REG;
	volatile unsigned int* XON1_ADDR1_REG;
	volatile unsigned int* LSR_REG;
	volatile unsigned int* XON2_ADDR2_REG;
	volatile unsigned int* MSR_REG;
	volatile unsigned int* TCR_REG;
	volatile unsigned int* XOFF1_REG;
	volatile unsigned int* SPR_REG;
	volatile unsigned int* TLR_REG;
	volatile unsigned int* XOFF2_REG;
	volatile unsigned int* MDR1_REG;
	volatile unsigned int* MDR2_REG;
	volatile unsigned int* SFLSR_REG;
	volatile unsigned int* TXFLL_REG;
	volatile unsigned int* RESUME_REG;
	volatile unsigned int* TXFLH_REG;
	volatile unsigned int* SFREGL_REG;
	volatile unsigned int* RXFLL_REG;
	volatile unsigned int* SFREGH_REG;
	volatile unsigned int* RXFLH_REG;
	volatile unsigned int* UASR_REG;
	volatile unsigned int* BLR_REG;
	volatile unsigned int* ACREG_REG;
	volatile unsigned int* SCR_REG;
	volatile unsigned int* SSR_REG;
	volatile unsigned int* EBLR_REG;
	volatile unsigned int* MVR_REG;
	volatile unsigned int* SYSC_REG; /* System configuration REGister. */
	volatile unsigned int* SYSS_REG;  /* System status REGister */
	volatile unsigned int* WER_REG;
	volatile unsigned int* CFPS_REG;
} uart_t;

#endif /* UART_H_ */
