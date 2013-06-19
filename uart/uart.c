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
#include <mem_io.h>

static uart_t uart;
static uart_protocol_format_t protocol;
static uart_owner_t owner = {0x0, 0x1};

static int uart_get(int uart_nr);
static void uart_init(uart_protocol_format_t protocol);
static void uart_software_reset();
static void uart_read_character(char* buffer);
static void uart_write_character(char* buffer);
static void uart_write_uint8_t(uint8_t* buffer);
static int uart_is_empty_read_queue();
static int uart_is_empty_write_queue();

static void* uart_open(driver_msg_t* buf, size_t size, driver_mode_t mode) {
	if(size < sizeof(unsigned int))
		return NULL;

	if(!owner.owned){
		protocol.baudrate = buf->data[0];
		protocol.stopbit = buf->data[1];
		protocol.datalen = buf->data[2];
		protocol.use_parity = buf->data[3];
		uart_init(protocol);

		owner.owned = 0x1;
		return (void*) owner.owned_id;
	}
	return NULL;
}

static int uart_close(void* handle) {
	owner.owned = 0x0;
	owner.owned_id +=1;
	return 1;
}

static int uart_write(void* handle, driver_msg_t* buf, size_t size) {
	if(owner.owned_id == (unsigned int) handle) {
		int i;
		for(i = 0; i < size; i++) {
			while(!uart_is_empty_write_queue(uart));
				uart_write_character(&(buf->buffer[i]));
		}
	}
	return 1;
}

static int uart_read(void* handle, driver_msg_t* buf, size_t size) {
	return 1;
}

Driver_t driver = {
		NULL,
		uart_open,
		uart_close,
		uart_read,
		uart_write
};

int main(int argc, char* argv[]) {

	int exit_code = -1;

	if(argc == 2) {
		int device_number = strtol(argv[1], NULL, 10);
		exit_code = uart_get(device_number);
	}

	if(exit_code != 0) {
		while(1);
	} else {
		driver_init();
	}

	return exit_code;
}

static int uart_get(int uart_nr) {
	unsigned int uart_base_address;

	switch(uart_nr) {
		case 21 : uart_base_address = ((unsigned int)  0x4806A000); break;
		case 22 : uart_base_address = ((unsigned int) 0x4806C000); break;
		case 23 : uart_base_address = ((unsigned int) 0x49020000); break;
		default:
			return -1;
	}

	uart.DLL_REG = uart_base_address + UART_DLL_REG_OFFSET;
	uart.RHR_REG = uart_base_address + UART_RHR_REG_OFFSET;
	uart.THR_REG = uart_base_address + UART_THR_REG_OFFSET;
	uart.DLH_REG = uart_base_address + UART_DLH_REG_OFFSET;
	uart.IER_REG = uart_base_address + UART_IER_REG_OFFSET;
	uart.IIR_REG = uart_base_address + UART_IIR_REG_OFFSET;
	uart.FCR_REG = uart_base_address + UART_FCR_REG_OFFSET;
	uart.EFR_REG = uart_base_address + UART_EFR_REG_OFFSET;
	uart.LCR_REG = uart_base_address + UART_LCR_REG_OFFSET;
	uart.MCR_REG = uart_base_address + UART_MCR_REG_OFFSET;
	uart.XON1_ADDR1_REG = uart_base_address + UART_XON1_ADDR1_REG_OFFSET;
	uart.LSR_REG = uart_base_address + UART_LSR_REG_OFFSET;
	uart.XON2_ADDR2_REG = uart_base_address + UART_XON2_ADDR2_REG_OFFSET;
	uart.MSR_REG = uart_base_address + UART_MSR_REG_OFFSET;
	uart.TCR_REG = uart_base_address + UART_TCR_REG_OFFSET;
	uart.XOFF1_REG = uart_base_address + UART_XOFF1_REG_OFFSET;
	uart.SPR_REG = uart_base_address + UART_SPR_REG_OFFSET;
	uart.TLR_REG = uart_base_address + UART_TLR_REG_OFFSET;
	uart.XOFF2_REG = uart_base_address + UART_XOFF2_REG_OFFSET;
	uart.MDR1_REG = uart_base_address + UART_MDR1_REG_OFFSET;
	uart.MDR2_REG = uart_base_address + UART_MDR2_REG_OFFSET;
	uart.SFLSR_REG = uart_base_address + UART_SFLSR_REG_OFFSET;
	uart.TXFLL_REG = uart_base_address + UART_TXFLL_REG_OFFSET;
	uart.RESUME_REG = uart_base_address + UART_RESUME_REG_OFFSET;
	uart.TXFLH_REG = uart_base_address + UART_TXFLH_REG_OFFSET;
	uart.SFREGL_REG = uart_base_address + UART_SFREGL_REG_OFFSET;
	uart.RXFLL_REG = uart_base_address + UART_RXFLL_REG_OFFSET;
	uart.SFREGH_REG = uart_base_address + UART_SFREGH_REG_OFFSET;
	uart.RXFLH_REG = uart_base_address + UART_RXFLH_REG_OFFSET;
	uart.UASR_REG = uart_base_address + UART_UASR_REG_OFFSET;
	uart.BLR_REG = uart_base_address + UART_BLR_REG_OFFSET;
	uart.ACREG_REG = uart_base_address + UART_ACREG_REG_OFFSET;
	uart.SCR_REG = uart_base_address + UART_SCR_REG_OFFSET;
	uart.SSR_REG = uart_base_address + UART_SSR_REG_OFFSET;
	uart.EBLR_REG = uart_base_address + UART_EBLR_REG_OFFSET;
	uart.MVR_REG = uart_base_address + UART_MVR_REG_OFFSET;
	uart.SYSC_REG = uart_base_address + UART_SYSC_REG_OFFSET;
	uart.SYSS_REG = uart_base_address + UART_SYSS_REG_OFFSET;
	uart.WER_REG = uart_base_address + UART_WER_REG_OFFSET;
	uart.CFPS_REG = uart_base_address + UART_CFPS_REG_OFFSET;

	return 0;
}

static void uart_init(uart_protocol_format_t protocol) {
	int old_lcr_reg, old_enhanced_en, old_tcr_tlr, mr;

	unsigned int values[4];
	unsigned int addresses[4];

	//17.5.1.1.1 UART Software Reset
	uart_software_reset(uart);

	//perform 1) , 2) and 3)
	values[0] = uart.LCR_REG;
	addresses[0] = uart.LCR_REG;
	memory_mapped_read(values, 1);

	//switch to mode b
	old_lcr_reg = values[0]; 					//save old value
	values[0] = 0x00BF;							//mode b
	memory_mapped_write(values, addresses, 1);

	values[0] = uart.EFR_REG;
	values[1] = uart.LCR_REG;
	addresses[0] = uart.EFR_REG;
	addresses[1] = uart.LCR_REG;

	mr = memory_mapped_read(values, 2);

	old_enhanced_en = ((values[0] >> 4) & BIT0);
	values[0] |= BIT4;
	values[1] = 0x0080; 						//mode a, last write

	memory_mapped_write(values, addresses, 2);

	//perform 4), 5) and 6)
	values[0] = uart.MCR_REG;
	values[1] = uart.FCR_REG;
	values[2] = uart.LCR_REG;
	addresses[0] = uart.MCR_REG;
	addresses[1] = uart.FCR_REG;
	addresses[2] = uart.LCR_REG;

	mr = memory_mapped_read(values, 3);

	old_tcr_tlr = ((values[0] >> 6) & BIT0);
	values[0] |= BIT6;
	values[1] |= (BIT3 | BIT0);
	values[2] = 0x00BF; 				//mode b

	memory_mapped_write(values, addresses, 3);

	//perform 7), 8), 9) and 10)
	values[0] = uart.SCR_REG;
	values[1] = uart.EFR_REG;
	values[2] = uart.LCR_REG;
	addresses[0] = uart.SCR_REG;
	addresses[1] = uart.EFR_REG;
	addresses[2] = uart.LCR_REG;

	mr = memory_mapped_read(values, 3);

	values[0] |= (BIT2 | BIT6 | BIT7);
	values[1] |= (old_enhanced_en << 4);
	values[2] = 0x0080;							//mode a

	memory_mapped_write(values, addresses, 3);

	// perform 11), 12)

	values[0] = uart.MCR_REG;
	values[1] = uart.LCR_REG;
	addresses[0] = uart.MCR_REG;
	addresses[1] = uart.LCR_REG;

	mr = memory_mapped_read(values, 2);
	values[0] |= (old_tcr_tlr << 6);
	values[1] = old_lcr_reg;

	memory_mapped_write(values, addresses, 2);

	//perform 1) and 2)

	values[0] = uart.MDR1_REG;
	values[1] = uart.LCR_REG;
	addresses[0] = uart.MDR1_REG;
	addresses[1] = uart.LCR_REG;

	mr = memory_mapped_read(values, 2);

	values[0] = (BIT2 | BIT1 | BIT0);
	values[1] = 0x00BF; 						//mode b

	memory_mapped_write(values, addresses, 2);

	//perform 3), 4), 5) and 6)
	values[0] = uart.EFR_REG;
	values[1] = uart.LCR_REG;
	values[2] = uart.IER_REG;
	values[3] = uart.LCR_REG;
	addresses[0] = uart.EFR_REG;
	addresses[1] = uart.LCR_REG;
	addresses[2] = uart.IER_REG;
	addresses[3] = uart.LCR_REG;

	mr = memory_mapped_read(values, 4);

	old_enhanced_en = ((values[0] >> 4) & BIT0);
	values[0] |= BIT4;
	values[1] = 0x0;
	values[2] = 0x0;
	values[3] = 0x00BF;

	memory_mapped_write(values, addresses, 4);

	//perform 7) and 8)
	addresses[0] = uart.DLL_REG;
	addresses[1] = uart.DLH_REG;
	addresses[2] = uart.LCR_REG;

	values[0] = (protocol.baudrate & 0xFF);
	values[1] = ((protocol.baudrate >> 8) & 0x3F);
	values[2] = 0x0;

	memory_mapped_write(values, addresses, 3);

	//perform 9) and 10)

	values[0] = uart.IER_REG;
	addresses[0] = uart.IER_REG;
	addresses[1] = uart.LCR_REG;

	mr = memory_mapped_read(values, 1);
	values[0] |= BIT0;
	values[1] = 0x00BF; 					//mode b

	memory_mapped_write(values, addresses, 2);

	//perform 11) and 12 a)
	values[0] = uart.EFR_REG;
	values[1] = uart.LCR_REG;
	addresses[0] = uart.EFR_REG;
	addresses[1] = uart.LCR_REG;

	mr = memory_mapped_read(values, 2);

	values[0] |= (old_enhanced_en << 4);
	values[1] &= 0x00111111;

	memory_mapped_write(values, addresses, 2);

	//perform 12 b) and 13)

	values[0] = uart.LCR_REG;
	addresses[0] = uart.LCR_REG;
	addresses[1] = uart.MDR1_REG;

	mr = memory_mapped_read(values, 1);

	values[0] |= protocol.datalen;
	values[0] |= (protocol.stopbit << 2);
	values[0] |= (protocol.use_parity << 3);
	values[1] = 0x00;							//UART mode

	memory_mapped_write(values, addresses, 2);
}

/*
 * Clear the UART registers of the given UART
 */
static void uart_software_reset() {

	unsigned int values[1];
	unsigned int addresses[1];
	addresses[0] = uart.SYSC_REG;
	values[0] = uart.SYSC_REG;

  /*
   * When the software reset bit is set high SYSC_REG[1], it causes a full device reset.
   * This bit is automatically reset by the hardware. Read returns 0.
   */

  int mem_read_ret_val = memory_mapped_read(values, 1);
  values[0] = ( values[0] | BIT1 );
  memory_mapped_write(values, addresses, 1);

  /*
   * Wait for the end of the reset operation.
   * Poll the UARTi.SYSS_REG[0] RESETDONE bit until it equals 1.
   */

  int i;
  do {
	  for(i = 0; i < 20000; i++);
	  values[0] = uart.SYSS_REG;
	  memory_mapped_read(values, 1);
	  unsigned int test = (values[0] & BIT0);
  } while(!(values[0] & BIT0));

}

static void uart_read_character(char* buffer) {
//	*buffer = *(uart->RHR_REG);
}
static void uart_write_character(char* buffer) {
	unsigned int value[1];
	value[0] = *buffer;
	unsigned int address[1];
	address[0] = uart.THR_REG;
	memory_mapped_write(value, address,  1);
}
static void uart_write_uint8_t(uint8_t* buffer) {
	unsigned int value[1];
	value[0] = (((unsigned int) *buffer) & 0x000F);
	unsigned int address[1];
	address[0] = uart.THR_REG;
	memory_mapped_write(value, address, 1);
}
static int uart_is_empty_read_queue() {
	unsigned int value[1];
	value[0] = uart.LSR_REG;
	int ret = memory_mapped_read(value, 1);
	if((value[0] & BIT0) && ret == 0)
		return 0;
	return 1;
}
static int uart_is_empty_write_queue(void) {
	unsigned int value[1];
	value[0] = uart.LSR_REG;
	int ret = memory_mapped_read(value, 1);
	if((value[0] & BIT5) && ret == 0)
		return 1;
	return 0;
}
