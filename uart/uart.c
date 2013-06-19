/*
 * uart.c
 *
 *  Created on: 17.06.2013
 *      Author: edi
 */

#include "uart.h"
#include <driver.h>
#include <inttypes.h>
#include <device.h>
#include <process.h>
#include <bit.h>

static uart_t* uart = NULL;
static uart_protocol_format_t protocol;
static uart_owner_t owner = {0x0, 0x1};

static void uart_get(int uart_nr, uart_t* uart);
static void uart_init(uart_t* const uart, uart_protocol_format_t protocol);
static void uart_software_reset(uart_t* const uart);
static int uart_switch_to_config_mode_b(uart_t* const uart);
static int uart_switch_to_config_mode_a(uart_t* const uart);
static void uart_switch_to_operational_mode(uart_t* const uart);
static void uart_set_baudrate(uart_t* const uart, uart_protocol_format_t* const protocol);
static void uart_set_protocol_format(uart_t* const uart, uart_protocol_format_t* const protocol);
static void uart_set_mode(uart_t* const uart, int uart_mode);
static void uart_read_character(uart_t* const uart, char* buffer);
static void uart_write_character(uart_t* const uart, char* buffer);
static void uart_write_uint8_t(uart_t* const uart, uint8_t* buffer);
static int uart_is_empty_read_queue(uart_t* const uart);
static int uart_is_empty_write_queue(uart_t* const uart);

static int uart_ioctl(driver_msg_t* buf, size_t size) {
	if(size < sizeof(unsigned int))
		return NULL;

	//TODO: delete if process start with arguments possible

	if(!owner.owned){
		unsigned int device_number = buf->data[0];
		uart_get(device_number, uart);
	}
	return 1;
}

static void* uart_open(driver_msg_t* buf, size_t size, driver_mode_t mode) {
	if(size < sizeof(unsigned int))
		return NULL;

	if(!owner.owned){
		protocol.baudrate = buf->data[0];
		protocol.stopbit = buf->data[1];
		protocol.datalen = buf->data[2];
		protocol.use_parity = buf->data[3];
		uart_init(uart, protocol);
		if(uart != NULL) {
			owner.owned = 0x1;
			return (void*) owner.owned_id;
		}
	}
	return NULL;
}

static int uart_close(void* handle) {
	owner.owned = 0x0;
	owner.owned_id +=1;
	return 1;
}

static int uart_write(void* handle, driver_msg_t* buf, size_t size) {
	if(owner.owned_id == (unsigned int) &handle) {
		int i;
		for(i = 0; i < size; i++) {
			while(!uart_is_empty_write_queue(uart));
				uart_write_character(uart, &(buf->buffer[i]));
		}
	}
	return 1;
}

static int uart_read(void* handle, driver_msg_t* buf, size_t size) {
	return 1;
}

Driver_t driver = {
		uart_ioctl,
		uart_open,
		uart_close,
		uart_read,
		uart_write
};

void main(void) {
	driver_init();
}

static void uart_get(int uart_nr, uart_t* uart) {
	unsigned int uart_base_address;

	switch(uart_nr) {
		case 21 : uart_base_address = ((unsigned int)  0x4806A000); break;
		case 22 : uart_base_address = ((unsigned int) 0x4806C000); break;
		case 23 : uart_base_address = ((unsigned int) 0x49020000); break;
		default:
//			uart_t fault = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
//					NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };
//			*uart = fault;
			uart = NULL;
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

static void uart_init(uart_t* const uart, uart_protocol_format_t protocol) {
	int old_lcr_reg, old_enhanced_en, old_tcr_tlr;

	//17.5.1.1.1 UART Software Reset
	uart_software_reset(uart);

	//enable FIFO - 17.5.1.1.2
	//1)
	old_lcr_reg = uart_switch_to_config_mode_b(uart);

	//2)
	old_enhanced_en = ((*(uart->EFR_REG) >> 4) & BIT0);
	*(uart->EFR_REG) |= BIT4;

	//3)
	uart_switch_to_config_mode_a(uart);

	//4)
	old_tcr_tlr = ((*(uart->MCR_REG) >> 6) & BIT0);
	*(uart->MCR_REG) |= BIT6;

	//5)
	//after UART software reset both trigger levels
	//(receive FCR_REG[7:6] and transmit FCR_REG[5:4])
	//are set to 8 characters (1 byte)
	*(uart->FCR_REG) |= BIT3; 			//DMA mode 1
	*(uart->FCR_REG) |= BIT0;			//enable FIFO

	//6)
	uart_switch_to_config_mode_b(uart);

	//7 - setting RX_FIFO_TRIG_DMA, TX_FIFO_TRIG_DMA
	//take care if using DMA

	//8
	*(uart->SCR_REG) |= BIT7;		//enable the granularity on TRIGGER RX level
	*(uart->SCR_REG) |= BIT6;		//enable the granularity on TRIGGER TX level
	*(uart->SCR_REG) |= BIT2;		//specify -> DMA mode 1 (UARTi_DMA_TX, UARTi_DMA_RX)
	//*(uart->SCR_REG) |= 			//set the bit to 1 or 0 to specify which register we used to set the DMA mode, we used both ...

	//9
	*(uart->EFR_REG) |= (old_enhanced_en << 4);

	//10
	uart_switch_to_config_mode_a(uart);

	//11
	*(uart->MCR_REG) |= (old_tcr_tlr << 6);

	//12
	*(uart->LCR_REG) = old_lcr_reg;

	//Protocol, Baud Rate, and Interrupt Settings - 17.5.1.1.3

	//1)
	*(uart->MDR1_REG) |= (BIT2 | BIT1 | BIT0);

	//2)
	uart_switch_to_config_mode_b(uart);

	//3
	old_enhanced_en = ((*(uart->EFR_REG) >> 4) & BIT0);
	*(uart->EFR_REG) |= BIT4;

	//4
	*(uart->LCR_REG) = 0x0000;

	//5
	*(uart->IER_REG) = 0x0000;

	//6
	uart_switch_to_config_mode_b(uart);

	//7
	uart_set_baudrate(uart, &protocol);

	//8
	*(uart->LCR_REG) = 0x0000;

	//9 - load the new interrupt configuration
	*(uart->IER_REG) |= BIT0;

	//10
	uart_switch_to_config_mode_b(uart);

	//11
	*(uart->EFR_REG) |= (old_enhanced_en << 4);

	//12
	uart_switch_to_operational_mode(uart);
	uart_set_protocol_format(uart, &protocol);

	//13
	uart_set_mode(uart, 0x00);
}

/*
 * Clear the UART registers of the given UART
 */
static void uart_software_reset(uart_t* const uart) {

  /*
   * When the software reset bit is set high SYSC_REG[1], it causes a full device reset.
   * This bit is automatically reset by the hardware. Read returns 0.
   */
  *(uart->SYSC_REG) |= BIT1;

  /*
   * Wait for the end of the reset operation.
   * Poll the UARTi.SYSS_REG[0] RESETDONE bit until it equals 1.
   */
  while (!(*uart->SYSS_REG & BIT0)) {}
}

/*
 * Switch to UART configuration b mode.
 * @return the old value of the LCR (line control register)
 */
static int uart_switch_to_config_mode_b(uart_t* const uart) {
	int old_lcr_reg = *(uart->LCR_REG);
	*(uart->LCR_REG) = 0x00BF;
	return old_lcr_reg;
}
/*
 * Switch to UART configuration a mode.
 * @return the old value of the LCR (line control register)
 */
static int uart_switch_to_config_mode_a(uart_t* const uart) {
	int old_lcr_reg = *(uart->LCR_REG);
	*(uart->LCR_REG) = 0x0080;
	return old_lcr_reg;
}

static void uart_switch_to_operational_mode(uart_t* const uart) {
	*(uart->LCR_REG) &= 0x00111111;
}

static void uart_set_baudrate(uart_t* const uart,  uart_protocol_format_t* const protocol) {
	*(uart->DLL_REG) = protocol->baudrate & 0xFF;				/* set least signification bits */
	*(uart->DLH_REG) = (protocol->baudrate >> 8) & 0x3F;		/* set most signification bits */
}
static void uart_set_protocol_format(uart_t* const uart, uart_protocol_format_t* const protocol) {
	*(uart->LCR_REG) |= protocol->datalen;
	*(uart->LCR_REG) |= (protocol->stopbit << 2);
	*(uart->LCR_REG) |= (protocol->use_parity << 3);
}
static void uart_set_mode(uart_t* const uart, int uart_mode) {
	*(uart->MDR1_REG) = uart_mode;
}
static void uart_read_character(uart_t* const uart, char* buffer) {
	*buffer = *(uart->RHR_REG);
}
static void uart_write_character(uart_t* const uart, char* buffer) {
	*(uart->THR_REG) = *buffer;
}
static void uart_write_uint8_t(uart_t* const uart, uint8_t* buffer) {
	*(uart->THR_REG) = *buffer;
}
static int uart_is_empty_read_queue(uart_t* const uart) {
	if(*(uart->LSR_REG) & BIT0)
		return 0;
	return 1;
}
static int uart_is_empty_write_queue(uart_t* const uart) {
	if(*(uart->LSR_REG) & BIT5)
		return 1;
	return 0;
}
