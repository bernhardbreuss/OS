/*
 * uart.h
 *
 *  Created on: 17.06.2013
 *      Author: edi
 */

#ifndef UART_H_
#define UART_H_

typedef struct _uart_t {
	unsigned int DLL_REG;
	unsigned int RHR_REG;
	unsigned int THR_REG;
	unsigned int DLH_REG;
	unsigned int IER_REG;
	unsigned int IIR_REG;
	unsigned int FCR_REG;
	unsigned int EFR_REG;
	unsigned int LCR_REG;
	unsigned int MCR_REG;
	unsigned int XON1_ADDR1_REG;
	unsigned int LSR_REG;
	unsigned int XON2_ADDR2_REG;
	unsigned int MSR_REG;
	unsigned int TCR_REG;
	unsigned int XOFF1_REG;
	unsigned int SPR_REG;
	unsigned int TLR_REG;
	unsigned int XOFF2_REG;
	unsigned int MDR1_REG;
	unsigned int MDR2_REG;
	unsigned int SFLSR_REG;
	unsigned int TXFLL_REG;
	unsigned int RESUME_REG;
	unsigned int TXFLH_REG;
	unsigned int SFREGL_REG;
	unsigned int RXFLL_REG;
	unsigned int SFREGH_REG;
	unsigned int RXFLH_REG;
	unsigned int UASR_REG;
	unsigned int BLR_REG;
	unsigned int ACREG_REG;
	unsigned int SCR_REG;
	unsigned int SSR_REG;
	unsigned int EBLR_REG;
	unsigned int MVR_REG;
	unsigned int SYSC_REG;
	unsigned int SYSS_REG;
	unsigned int WER_REG;
	unsigned int CFPS_REG;
} uart_t;

/*
 * UART protocol format
 */
typedef struct _uart_protocol_format_t {
    unsigned int baudrate;
    unsigned short datalen;
    unsigned short stopbit;
    unsigned int use_parity;
} uart_protocol_format_t;

typedef struct _uart_owner_t {
	unsigned int owned 		: 1;
	unsigned int owned_id	: 31;
} uart_owner_t;


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

#endif /* UART_H_ */
