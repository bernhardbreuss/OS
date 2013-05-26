/*
 * uart_new.c
 *
 *  Created on: 24.05.2013
 *      Author: füssi CoOp edi
 */

#include "uart_new.h"

void uart_new_get(int uart_nr, uart_t* uart){
	unsigned int uart_base_address;

	switch(uart_nr) {
		case 1 : uart_base_address = UART1_NEW; break;
		case 2 : uart_base_address = UART2_NEW; break;
		case 3 : uart_base_address = UART3_NEW; break;
		default:
			logger_error("UART number %d in  not supported!", uart_nr);
			uart_t fault = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
					NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };
			*uart = fault;
			return;
	}

	uart->DLL_REG = (unsigned int*) (uart_base_address + UART_DLL_REG_OFFSET);
	uart->RHR_REG = (unsigned int*) (uart_base_address + UART_RHR_REG_OFFSET);
	uart->THR_REG = (unsigned int*) (uart_base_address + UART_THR_REG_OFFSET);
	uart->DLH_REG = (unsigned int*) (uart_base_address + UART_DLH_REG_OFFSET);
	uart->IER_REG = (unsigned int*) (uart_base_address + UART_IER_REG_OFFSET);
	uart->IIR_REG = (unsigned int*) (uart_base_address + UART_IIR_REG_OFFSET);
	uart->FCR_REG = (unsigned int*) (uart_base_address + UART_FCR_REG_OFFSET);
	uart->EFR_REG = (unsigned int*) (uart_base_address + UART_EFR_REG_OFFSET);
	uart->LCR_REG = (unsigned int*) (uart_base_address + UART_LCR_REG_OFFSET);
	uart->MCR_REG = (unsigned int*) (uart_base_address + UART_MCR_REG_OFFSET);
	uart->XON1_ADDR1_REG = (unsigned int*) (uart_base_address + UART_XON1_ADDR1_REG_OFFSET);
	uart->LSR_REG = (unsigned int*) (uart_base_address + UART_LSR_REG_OFFSET);
	uart->XON2_ADDR2_REG = (unsigned int*) (uart_base_address + UART_XON2_ADDR2_REG_OFFSET);
	uart->MSR_REG = (unsigned int*) (uart_base_address + UART_MSR_REG_OFFSET);
	uart->TCR_REG = (unsigned int*) (uart_base_address + UART_TCR_REG_OFFSET);
	uart->XOFF1_REG = (unsigned int*) (uart_base_address + UART_XOFF1_REG_OFFSET);
	uart->SPR_REG = (unsigned int*) (uart_base_address + UART_SPR_REG_OFFSET);
	uart->TLR_REG = (unsigned int*) (uart_base_address + UART_TLR_REG_OFFSET);
	uart->XOFF2_REG = (unsigned int*) (uart_base_address + UART_XOFF2_REG_OFFSET);
	uart->MDR1_REG = (unsigned int*) (uart_base_address + UART_MDR1_REG_OFFSET);
	uart->MDR2_REG = (unsigned int*) (uart_base_address + UART_MDR2_REG_OFFSET);
	uart->SFLSR_REG = (unsigned int*) (uart_base_address + UART_SFLSR_REG_OFFSET);
	uart->TXFLL_REG = (unsigned int*) (uart_base_address + UART_TXFLL_REG_OFFSET);
	uart->RESUME_REG = (unsigned int*) (uart_base_address + UART_RESUME_REG_OFFSET);
	uart->TXFLH_REG = (unsigned int*) (uart_base_address + UART_TXFLH_REG_OFFSET);
	uart->SFREGL_REG = (unsigned int*) (uart_base_address + UART_SFREGL_REG_OFFSET);
	uart->RXFLL_REG = (unsigned int*) (uart_base_address + UART_RXFLL_REG_OFFSET);
	uart->SFREGH_REG = (unsigned int*) (uart_base_address + UART_SFREGH_REG_OFFSET);
	uart->RXFLH_REG = (unsigned int*) (uart_base_address + UART_RXFLH_REG_OFFSET);
	uart->UASR_REG = (unsigned int*) (uart_base_address + UART_UASR_REG_OFFSET);
	uart->BLR_REG = (unsigned int*) (uart_base_address + UART_BLR_REG_OFFSET);
	uart->ACREG_REG = (unsigned int*) (uart_base_address + UART_ACREG_REG_OFFSET);
	uart->SCR_REG = (unsigned int*) (uart_base_address + UART_SCR_REG_OFFSET);
	uart->SSR_REG = (unsigned int*) (uart_base_address + UART_SSR_REG_OFFSET);
	uart->EBLR_REG = (unsigned int*) (uart_base_address + UART_EBLR_REG_OFFSET);
	uart->MVR_REG = (unsigned int*) (uart_base_address + UART_MVR_REG_OFFSET);
	uart->SYSC_REG = (unsigned int*) (uart_base_address + UART_SYSC_REG_OFFSET);
	uart->SYSS_REG = (unsigned int*) (uart_base_address + UART_SYSS_REG_OFFSET);
	uart->WER_REG = (unsigned int*) (uart_base_address + UART_WER_REG_OFFSET);
	uart->CFPS_REG = (unsigned int*) (uart_base_address + UART_CFPS_REG_OFFSET);
}

/*
 * Clear the UART registers of the given uart_base_addr
 */
void uart_new_software_reset(uart_t* uart) {

  /*
   * When the software reset bit is set high SYSC_REG[1], it causes a full device reset.
   * This bit is automatically reset by the hardware. Read returns 0.
   */
  *(uart->SYSC_REG) |= BIT1;

  /*
   * Wait for the end of the reset operation.
   * Poll the UARTi.SYSS_REG[0] RESETDONE bit until it equals 1.
   */
  while (!(*uart->SYSS_REG && BIT1)) {}
}



