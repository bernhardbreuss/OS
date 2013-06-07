/*
 * fs.c
 *
 *  Created on: 10.05.2013
 *      Author: lumannnn
 */

#include "fs.h"
#include "../../../platform/omap3530/mmchs.h"
#include "../../../bit.h"
#include "../../generic/fs/fs.h"
#include "../../../service/logger/logger.h"

#define MAX_TRIES (100*5)

// private API

/**
 * @param instance 1..3
 */
static void fs_device_init_mmchs(int instance_nr, MMCHS_t* instance);

static void fs_device_enable_interface_and_functional_clock();

static void fs_device_soft_reset(MMCHS_t* handle);

static void fs_device_set_default_capabilities(MMCHS_t* handle);
static void fs_device_wake_up_config(MMCHS_t* handle);
static RESPONSE_t fs_device_mmc_host_and_bus_config(MMCHS_t* handle);

static RESPONSE_t fs_device_identify_card(FileHandle_t* handle);

// helper functions

static int fs_device_check_mmchs_stat_cc(MMCHS_t* instance);
static int fs_device_check_mmchs_stat_cto(MMCHS_t* instance);
static void fs_device_set_mmchs_sysctl_src_and_wait_until_reset(MMCHS_t* instance);
static void fs_device_set_mmchs_sysctl_srd_and_wait_until_reset(MMCHS_t* instance);

static RESPONSE_t fs_device_finish_card_identification(FileHandle_t* handle);

static void fs_device_update_clock_frequency(MMCHS_t* instance, unsigned int frequency);

typedef enum {
	READ=0,
	WRITE
} MMCHS_OPERATION_t;

typedef struct _CMD {
	unsigned int mmchs_con;
	unsigned int mmchs_ie;
	unsigned int mmchs_ise;
	unsigned int mmchs_cmd;
	unsigned int mmchs_arg;
} CMD_t;

#define UNDEFINED_ARG (0x0)

static RESPONSE_t fs_device_send_cmd(MMCHS_t* instance, CMD_t* cmd);

static RESPONSE_t fs_device_send_cmd0(MMCHS_t* instance);
//static RESPONSE_t fs_device_send_cmd1(MMCHS_t* instance);
static RESPONSE_t fs_device_send_cmd2(MMCHS_t* instance);
static RESPONSE_t fs_device_send_cmd3(MMCHS_t* instance);
static RESPONSE_t fs_device_send_cmd5(MMCHS_t* instance);
static RESPONSE_t fs_device_send_cmd7(MMCHS_t* instance);
static RESPONSE_t fs_device_send_cmd8(MMCHS_t* instance);
//static RESPONSE_t fs_device_send_cmd9(MMCHS_t* instance);
//static RESPONSE_t fs_device_send_cmd12(MMCHS_t* instance);
//static RESPONSE_t fs_device_send_cmd52(MMCHS_t* instance);
static RESPONSE_t fs_device_send_cmd55(MMCHS_t* instance);
//static RESPONSE_t fs_device_set_bus_width_with_cmd6(MMCHS_t* instance);
//static RESPONSE_t fs_device_enable_high_speed_feature_with_cmd6(MMCHS_t* instance);
static RESPONSE_t fs_device_send_acmd6(MMCHS_t* instance);
static RESPONSE_t fs_device_send_acmd41(MMCHS_t* instance);

static RESPONSE_t fs_device_read_write(FileHandle_t* handle, MMCHS_OPERATION_t operation);
/*
static RESPONSE_t fs_device_read_write_dma_polling(FileHandle_t* handle, MMCHS_OPERATION_t* operation);
static RESPONSE_t fs_device_configure_and_enable_DMA(FileHandle_t* handle);
static RESPONSE_t fs_device_disable_DMA(FileHandle_t* handle);
*/

static void fs_device_logger_debug(char* message) {
	logger_debug(message);
}
static void fs_device_logger_error(char* message) {
	logger_error(message);
}


// * *********************** *
// * 	API
// * *********************** *

RESPONSE_t fs_init(FileHandle_t* handle) {
	RESPONSE_t status = SUCCESS;
	// see page 3160:
	// first step:
	// configure interface and functional clocks.
	// -> YES
	// software reset of the MMC/SD/SDIO host controller
	// ->
	// second step:
	// set module's hardware capabilities
	// ->
	// customize module's idle and wake-up modes
	// ->
	// end
	fs_device_init_mmchs(1, handle->instance);

	fs_device_enable_interface_and_functional_clock(handle);
	fs_device_soft_reset(handle->instance);
	fs_device_set_default_capabilities(handle->instance);
	fs_device_wake_up_config(handle->instance);

	status = fs_device_mmc_host_and_bus_config(handle->instance);
	if (status == ERROR) {
		return ERROR;
	}

	return fs_device_identify_card(handle);
}

RESPONSE_t fs_read(FileHandle_t* handle) {
	RESPONSE_t status = ERROR;
 	status = fs_device_read_write(handle, READ);

	return status;
}

RESPONSE_t fs_write(FileHandle_t* handle) {
	RESPONSE_t status = ERROR;
 	status = fs_device_read_write(handle, WRITE);

	return status;
}

// private API

static void fs_device_init_mmchs(int instance_nr, MMCHS_t* instance) {
	unsigned int* base = 0x0;
	switch (instance_nr) {
	case 1:
		base = MMCHS1_BASE;
		break;
	case 2:
		base = MMCHS2_BASE;
		break;
	case 3:
		base = MMCHS3_BASE;
		break;
	default:
		fs_device_logger_error("unsupported instance requested.\n");
	}

	instance->SYSCONFIG = base + MMCHS_SYSCONFIG_OFFSET;
	instance->SYSSTATUS = base + MMCHS_SYSSTATUS_OFFSET;
	instance->CSRE = base + MMCHS_CSRE_OFFSET;
	instance->CON = base + MMCHS_CON_OFFSET;
	instance->BLK = base + MMCHS_BLK_OFFSET;
	instance->ARG = base + MMCHS_ARG_OFFSET;
	instance->CMD = base + MMCHS_CMD_OFFSET;
	instance->RSP10 = base + MMCHS_RSP10_OFFSET;
	instance->RSP32 = base + MMCHS_RSP32_OFFSET;
	instance->RSP54 = base + MMCHS_RSP54_OFFSET;
	instance->RSP76 = base + MMCHS_RSP76_OFFSET;
	instance->DATA = base + MMCHS_DATA_OFSET;
	instance->PSTATE = base + MMCHS_PSTATE_OFFSET;
	instance->HCTL = base + MMCHS_HCTL_OFFSET;
	instance->SYSCTL = base + MMCHS_SYSCTL_OFFSET;
	instance->STAT = base + MMCHS_STAT_OFFSET;
	instance->IE = base + MMCHS_IE_OFFSET;
	instance->ISE = base + MMCHS_ISE_OFFSET;
	instance->AC12 = base + MMCHS_AC12_OFFSET;
	instance->CAPA = base + MMCHS_CAPA_OFFSET;
}

static void fs_device_enable_interface_and_functional_clock() {
	// Prior to any MMCHS register access one must enable MMCHS interface clock and functional clock in
	// PRCM module registers PRCM.CM_ICLKEN1_CORE and PRCM.CM_FCLKEN1_CORE. Please refer to
	// Chapter 4, Power, Reset, and Clock Management.
	*(CM_FCLKEN1_CORE) |= (CM_CLKEN1_CORE_EN_MMC1 | CM_CLKEN1_CORE_EN_MMC2 | CM_CLKEN1_CORE_EN_MMC3);
	*(CM_ICLKEN1_CORE) |= (CM_CLKEN1_CORE_EN_MMC1 | CM_CLKEN1_CORE_EN_MMC2 | CM_CLKEN1_CORE_EN_MMC3);
}

static void fs_device_soft_reset(MMCHS_t* instance) {
	// see page 3161
	// set the MMCi.MMCHS_SYSCONFIG[1] SOFTRESET bit to 0x1
	*(instance->SYSCONFIG) |= MMCHS_SYSCONFIG_SOFTRESET;
	// Read the MMCi.MMCHS_SYSSTATUS[0] RESETDONE bit
	// RESETDONE = 0x1? --> no? -> wait.
	while ((*(instance->SYSSTATUS)) & MMCHS_SYSSTATUS_RESETDONE != MMCHS_SYSSTATUS_RESETDONE)
		;
}

static void fs_device_set_default_capabilities(MMCHS_t* instance) {
	/*
	 * Software must read capabilities (in boot ROM for instance) and is allowed to set (write)
	 * MMCi.MMCHS_CAPA[26:24] and MMCi.MMCHS_CUR_CAPA[23:0] registers before the MMC/SD/SDIO
	 * host driver is started.
	 */
	// activate VS18 and VS30
	*(instance->CAPA) |= MMCHS_CAPA_VS18 | MMCHS_CAPA_VS30;
}

static void fs_device_wake_up_config(MMCHS_t* instance) {
	// page 3162
	// set the MMCi.MMCHS_SYSCONFIG[2] ENAWAKEUP bit to 0x1 if required
	*(instance->SYSCONFIG) |= MMCHS_SYSCONFIG_ENAWAKEUP;
	// set the MMCi.MMCHS_HCTL[24] IWE bit to 0x1 to enable the wake-up event on SD card interrupt
	*(instance->HCTL) |= MMCHS_HCTL_IWE;
	// set the MMCi.MMCHS[8] CIRQ_ENABLE bit enable the card interrupt (for SDIO card only)
	*(instance->IE) |= MMCHS_IE_CIRQ_ENABLE;
}

static RESPONSE_t fs_device_mmc_host_and_bus_config(MMCHS_t* instance) {
	// page 3163
	// write MMCi.MMCHS_CON register (OD, DW8, CEATA) to configure specific data and command transfer
	*(instance->CON) |= MMCHS_CON_OD
			& ~MMCHS_CON_DW8
			& ~MMCHS_CON_CEATA;

	// write MMCi.MMCHS_HCTL register (SDVS, SDBP, DTW) to configure the card voltage value and power mode and data bus width
	/* write MMCi.MMCHS_HCTL register */
	*(instance->HCTL) &= ~MMCHS_HCTL_SVDS_RESET;

	if (instance->ARG != MMCHS1_BASE + MMCHS_ARG_OFFSET) {
		/*
		 * MMCHS2: This field must be set to 0x5.
		 * MMCHS3: This field must be set to 0x5.
		 */
		*(instance->HCTL) |= MMCHS_HCTL_SDVS_1V8;
	} else {
		// TODO what about instance 1?
		*(instance->HCTL) |= MMCHS_HCTL_SDVS_3V3;
	}

	*(instance->HCTL) |= MMCHS_HCTL_SDBP | MMCHS_HCTL_DTW;
	// TODO ... i dont't know.
	// For SD/SDIO cards:
	// following a SET_BUS_WIDTH command (ACMD6) with a value written in bit 1 of the argument.
	// Prior to this command, the SD card configuration register (SCR) must be verified for the supported bus width by the SD card.

	// \
	//  \
	//   |
	//   V
	// Send ACMD6 (application specific commands must be prefixed with CMD55)
	RESPONSE_t status = SUCCESS;
	status = fs_device_send_cmd55(instance);
	if (status == ERROR) {
		fs_device_logger_error("fs_device_mmc_host_and_bus_config: cmd55 failed.\n");
//		return ERROR;
	}
	status = fs_device_send_acmd6(instance);
	if (status == ERROR) {
		fs_device_logger_error("fs_device_mmc_host_and_bus_config: acmd6 failed.\n");
//		return ERROR;
	}

	/* end of: write MMCi.MMCHS_HCTL register */

	// Read back the MMCi.MMCHS_HCTL[8] SDBP bit
	// SDBP = 0x1? --> no? -> wait.
	while ((*(instance->HCTL)) & MMCHS_HCTL_SDBP != MMCHS_HCTL_SDBP)
		;

	// set the MMCi.MMCHS_SYSCTL[0] ICE bit to 0x1 to enable the internal clock
	*(instance->SYSCTL) |= MMCHS_SYSCTL_ICE;

	// XXX: WHAT???? what you want from me? leave me alone with your horseshit! TODO -.-
	// set the CONTROL.CONTROL_PADCONF_<module>_<signal>[x] INPUTENABLE bit to 0x1
	// `-> Reason: Enable the input buffer of the clock output. As a result, there is a loopback through the output and input buffers. The goal is to support the synchronization of the mmci_clk.
	fs_device_update_clock_frequency(instance, MMCHS_SYSCTL_CLKD_80KHZ);

	// write the MMCi.MMCHS_SYSCONFIG CLOCKACTIVITY, SIDLEMODE and AUTOIDLE field to configure the behavior of the module in idle mode.
	*(instance->SYSCONFIG) |= MMCHS_SYSCOFNIG_AUTOIDLE | MMCHS_SYSCOFNIG_CLOCKACTIVITY_MAINTAIN;
	*(instance->SYSCONFIG) &= ~MMCHS_SYSCONFIG_SIDLEMODE_RESET;
	*(instance->SYSCONFIG) |= MMCHS_SYSCONFIG_SIDLEMODE_IGNORE_IDLE_REQUEST;

	return SUCCESS;
}

// see page 3164:
static RESPONSE_t fs_device_identify_card(FileHandle_t* handle) {
	// (Start)

	// do module initialization before this
	RESPONSE_t status = ERROR;

	// set MMCi.MMCHS_CON[1] INT bit to 0x1 to send an initialization stream
	*(handle->instance->CON) |= MMCHS_CON_INT;

	// write 0x00000000 in the MMCi.MMCHS_CMD register
	*(handle->instance->CMD) = 0x00000000;

	// (wait 1 ms)
	// FIXME
	volatile int i = 0;
	while (++i < 200000);

	// set MMCi.MMCHS_STAT[0] CC bit to 0x1 to clear the flag
	*(handle->instance->STAT) |= MMCHS_STAT_CC;

	// set MMCi.MMCHS_CON[1] INT bit to 0x0 to end the initialization sequence
	*(handle->instance->CON) &= ~MMCHS_CON_INT;

	// clear MMCHS_STAT register (write 0xFFFF FFFF)
	*(handle->instance->STAT) = 0xFFFFFFFF;

	// change clock frequency to fit protocol
	fs_device_update_clock_frequency(handle->instance, MMCHS_SYSCTL_CLKD_400KHZ);

	// send CMD0 command
	status = fs_device_send_cmd0(handle->instance);
	if (status == ERROR) {
		fs_device_logger_error("fs_device_identify_card: cmd0 failed.\n");
//		return ERROR;
	}
	// (A)
	// send a CMD5 command
	status = fs_device_send_cmd5(handle->instance);
	if (status == ERROR) {
		fs_device_logger_error("fs_device_identify_card: cmd5 failed.\n");
//		return ERROR;
	}

	// read the MMCi.MMCHS_STAT register
	while (1) {
		// CC = 0x1
		if (fs_device_check_mmchs_stat_cc(handle->instance) == 1) {
			// it is an SDIO card
			handle->storage_type = SDIO;
			fs_device_logger_debug("SDIO card detected\n");
			fs_device_logger_error("SDIO cards are not supported.\n");

			// See the SDIO Standard Specification to identify the card type:
			// Memory only, I/O only, Combo

			// goto (End)
			return ERROR;
		}
		// CTO = 0x1
		if (fs_device_check_mmchs_stat_cto(handle->instance) == 1) {
			break;
		}
	}

	fs_device_set_mmchs_sysctl_src_and_wait_until_reset(handle->instance);
	status = fs_device_send_cmd8(handle->instance);
	if (status == ERROR) {
		fs_device_logger_error("fs_device_identify_card: cmd8 failed.\n");
//		return ERROR;
	}

	// read the MMCi.MMCHS_STAT register
	while (1) {
		// CC = 0x1
		if (fs_device_check_mmchs_stat_cc(handle->instance) == 1) {
			// yes? (it is an SD card compliant with standard 2.0 or later)
			handle->storage_type = SD_2;
			fs_device_logger_debug("SD card compliant with standard 2.0 or later detected.\n");

			// see the SD Standard Specification version 2.0 or later
			// to identify the card type: High Capacity; Standard Capacity

			// debug
			unsigned int response = *(handle->instance->RSP10);
			fs_device_logger_debug("CMD8 success.\n");
			unsigned int CMD8_ARG = (0x0UL << 12 | (1 << 8) | 0xCEUL << 0);
			if (response != CMD8_ARG) {
//				return ERROR;
			}

			handle->storage_type = SD_2_HC;

			// goto (End)
			return SUCCESS;
		}
		// CTO = 0x1
		if (fs_device_check_mmchs_stat_cto(handle->instance) == 1) {
			break;
		}
	}

	fs_device_set_mmchs_sysctl_src_and_wait_until_reset(handle->instance);

	int abort = 0;
	while (abort == 0) {
		// see page 3165.
		// marker:
		// ---
		// |A|
		//  V

		status = fs_device_send_cmd55(handle->instance);
		if (status == ERROR) {
			fs_device_logger_error("fs_device_identify_card: cmd55 failed.\n");
			return ERROR;
		}
		status = fs_device_send_acmd41(handle->instance);
		if (status == ERROR) {
			fs_device_logger_error("fs_device_identify_card: acmd41 failed.\n");
			return ERROR;
		}

		// read the MMCi.MMCHS_STAT register
		while (1) {
			// CC = 0x1
			if (fs_device_check_mmchs_stat_cc(handle->instance) == 1) {
				// (it is a SD card compliant with standard 1.x)
				handle->storage_type = SD_1x;
				fs_device_logger_debug("SD card compliant with standard 1.x or later detected.\n");

				// verify the card is busy:
				// read the MMCi.MMCHS_RSP10[31] bit
				// equal to 0x1?
				unsigned int* mmchs_rsp10 = handle->instance->RSP10;
				if (*(mmchs_rsp10) == (*(mmchs_rsp10)) | BIT31) {
					// --> yes? -> the card is not busy
					// 			goto (B)
					return fs_device_finish_card_identification(handle);
				} else {
					// --> no? -> the card is busy
					//			goto ---
					//				 |A|
					//				  V
					break;
				}
			}
			// CTO = 0x1
			if (fs_device_check_mmchs_stat_cto(handle->instance) == 1) {
				// it is a MMC card
				abort = 1;
				break;
			}
		}
	}


	// (it is a MMC card)
	handle->storage_type = MMC;
	fs_device_set_mmchs_sysctl_src_and_wait_until_reset(handle->instance);
	// FIX ME:
	// As of now, we should not get to here. We don't have any MMC card so we do not support it.
	// If you want to support MMC cards: uncomment following code block; test it.
	return ERROR;
	/*

	while (1) {
		// - ---------- -
		// - (marker_1) -
		// - ---------- -

		// TO DO what's this OCR 0?
		// send a CMD1 command* (*With OCR 0. In case of a CMD1 with OCR=0, a second CMD1 must be sent to the card with the "negociated" voltage.
		status = fs_device_send_cmd1(handle->instance);
		if (status == ERROR) {
			fs_device_logger_error("fs_device_identify_card: cmd1 failed.\n");
			return ERROR;
		}

		// read the MMCi.MMCHS_STAT register
		while (1) {
			// CTO = 0x1
			if (fs_device_check_mmchs_stat_cto(handle->instance) == 1) {
				// unknown type of card
				handle->storage_type = UNKNOWN;
				fs_device_logger_debug("unknown card type.\n");
				// goto (end)
				return ERROR;
			}

			// CC = 0x1
			if (fs_device_check_mmchs_stat_cc(handle->instance) == 1) {
				break;
			}
		}

		// it is a MMC card
		handle->storage_type = MMC;
		fs_device_logger_debug("MMC card detected.\n");

		// Verify the card is busy:
		// read the MMCi.MMCHS_RSP19[31] bit --> typo in OMPA35x.pdf: should be MMCHS_RSP10
		// equal to 0x1?
		unsigned int* mmchs_rsp10 = handle->instance->RSP10;
		if (*(mmchs_rsp10) == (*(mmchs_rsp10)) | BIT31) {
			// --> yes? -> the card is not busy
			// 			goto (B)
			return fs_device_finish_card_identification(handle);
		} else {
			// --> no? (the card is busy)
			// 			-> goto (marker_1)
			fs_device_logger_debug("card is busy...\n");
		}
	}
	*/

	// (end)
}

/**
 * (B)
 */
static RESPONSE_t fs_device_finish_card_identification(FileHandle_t* handle) {
	// (B)
	int status = SUCCESS;

	// TODO what about the information?
	// send a CMD2 command to get information on how to access the card content
	status = fs_device_send_cmd2(handle->instance);
	if (status == ERROR) {
		fs_device_logger_error("fs_device_identify_card: fs_device_finish_card_identification: cmd2 failed.\n");
		return ERROR;
	}
	status = fs_device_send_cmd3(handle->instance);
	if (status == ERROR) {
		fs_device_logger_error("fs_device_identify_card: fs_device_finish_card_identification: cmd3 failed.\n");
		return ERROR;
	}

	// card type?
	if (handle->storage_type == MMC) {
		// --> MMC card? -> Is there more than one MMC connected to the same bus, and are they all identified?
		//			--> yes -> goto (B)
		//			--> no -> (continue)

		// nothing to do here...
		// we do not support MMC cards yet.
		fs_device_logger_error("MMC cards are not supported.\n");
		status = ERROR;
	}

	// (continue)

	status = fs_device_send_cmd7(handle->instance);
	if (status == ERROR) {
		fs_device_logger_error("fs_device_identify_card: fs_device_finish_card_identification: cmd7 failed.\n");
		return ERROR;
	}
	return status;
}

//static void fs_set_direction_read(unsigned int* MMCHS_instance) {
//	*(MMCHS_instance + MMCHS_CMD_OFFSET) |= MMCHS_CMD_DDIR;
//}
//
//static void fs_set_direction_write(unsigned int* MMCHS_instance) {
//	*(MMCHS_instance + MMCHS_CMD_OFFSET) &= ~MMCHS_CMD_DDIR;
//}


// helper functions

static int fs_device_check_mmchs_stat_cc(MMCHS_t* instance) {
	if (*(instance->STAT) == (*(instance->STAT) | MMCHS_STAT_CC)) {
		return 1;
	}

	return 0;
}

static int fs_device_check_mmchs_stat_cto(MMCHS_t* instance) {
	if (*(instance->STAT) == (*(instance->STAT) | MMCHS_STAT_CTO)) {
		return 1;
	}

	return 0;
}

static void fs_device_set_mmchs_sysctl_src_and_wait_until_reset(MMCHS_t* instance) {
	// set MMCi.MMCHS_SYSCTL[25] SRC bit to 0x1
	*(instance->SYSCTL) |= MMCHS_SYSCTL_SRC;
	// and wait until it returns to 0x0
	while ((*(instance->SYSCTL)) & MMCHS_SYSCTL_SRC != ~MMCHS_SYSCTL_SRC)
			;
}

static void fs_device_set_mmchs_sysctl_srd_and_wait_until_reset(MMCHS_t* instance) {
	// set MMCi.MMCHS_SYSCTL[26] SRD bit to 0x1
	*(instance->SYSCTL) |= MMCHS_SYSCTL_SRD;
	// and wait until it returns to 0x0
	while ((*(instance->SYSCTL)) & MMCHS_SYSCTL_SRD != ~MMCHS_SYSCTL_SRD)
			;
}

static void fs_device_update_clock_frequency(MMCHS_t* instance, unsigned int frequency) {
	// set clock enable to 0x0 to not provide the clock to the card
	*(instance->SYSCTL) &= ~MMCHS_SYSCTL_CEN;

	// set new clock frequency
	*(instance->SYSCTL) &= ~MMCHS_SYSCTL_CLKD_RESET;
	*(instance->SYSCTL) |= (frequency << 6);

	// poll until internal clock is stable
	while ( (*(instance->SYSCTL)) & MMCHS_SYSCTL_ICS != MMCHS_SYSCTL_ICS )
		;

	// set clock enable to 0x1 to provide the clock to the card
	*(instance->SYSCTL) |= MMCHS_SYSCTL_CEN;
}

/*
 * implement the "send_cmd_xyz()" according to page 3172.
 */

static RESPONSE_t fs_device_send_cmd0(MMCHS_t* instance) {
	CMD_t cmd;
	cmd.mmchs_con = 0x00000001;
	cmd.mmchs_ie = 0x00040001;
	cmd.mmchs_ise = 0x00040001;
	cmd.mmchs_cmd = 0x00000000;
	cmd.mmchs_arg =  UNDEFINED_ARG;
	return fs_device_send_cmd(instance, &cmd);
}
/*
static RESPONSE_t fs_device_send_cmd1(MMCHS_t* instance) {
	CMD_t cmd;
	cmd.mmchs_con = 0x00000001;
	cmd.mmchs_ie = 0x00050001;
	cmd.mmchs_ise = 0x00050001;
	cmd.mmchs_cmd = 0x01020000;
	cmd.mmchs_arg =  UNDEFINED_ARG;
	return fs_device_send_cmd(instance, &cmd);
}
*/
static RESPONSE_t fs_device_send_cmd2(MMCHS_t* instance) {
	CMD_t cmd;
	cmd.mmchs_con =  0x00000001;
	cmd.mmchs_ie =  0x00070001;
	cmd.mmchs_ise =  0x00070001;
	cmd.mmchs_cmd =  0x02090000;
	cmd.mmchs_arg =  UNDEFINED_ARG;
	return fs_device_send_cmd(instance, &cmd);
}
static RESPONSE_t fs_device_send_cmd3(MMCHS_t* instance) {
	CMD_t cmd;
	cmd.mmchs_con =  0x00000001;
	cmd.mmchs_ie =  0x100f0001;
	cmd.mmchs_ise =  0x100f0001;
	cmd.mmchs_cmd =  0x031a0000;
	cmd.mmchs_arg =  0x00010000;
	return fs_device_send_cmd(instance, &cmd);
}
static RESPONSE_t fs_device_send_cmd5(MMCHS_t* instance) {
	CMD_t cmd;
	cmd.mmchs_con =  0x00000001;
	cmd.mmchs_ie =  0x00050001;
	cmd.mmchs_ise =  0x00050001;
	cmd.mmchs_cmd =  0x05020000;
	cmd.mmchs_arg =  UNDEFINED_ARG;
	return fs_device_send_cmd(instance, &cmd);
}
static RESPONSE_t fs_device_send_cmd7(MMCHS_t* instance) {
	CMD_t cmd;
	cmd.mmchs_con =  0x00000000;
	cmd.mmchs_ie =  0x100f0001;
	cmd.mmchs_ise =  0x100f0001;
	cmd.mmchs_cmd =  0x071a0000;
	cmd.mmchs_arg =  0x00010000;
	return fs_device_send_cmd(instance, &cmd);
}
static RESPONSE_t fs_device_send_cmd8(MMCHS_t* instance) {
	unsigned int cmd8_arg = (0x0UL << 12 | (1 << 8) | 0xCEUL << 0);
	CMD_t cmd;
	cmd.mmchs_con =  0x00000001;
	cmd.mmchs_ie =  0x100f0001;
	cmd.mmchs_ise =  0x100f0001;
	cmd.mmchs_cmd =  0x81a0000;
	cmd.mmchs_arg =  cmd8_arg; // UNDEFINED_ARG;
	return fs_device_send_cmd(instance, &cmd);
}
/*
static RESPONSE_t fs_device_send_cmd9(MMCHS_t* instance) {
	CMD_t cmd;
	cmd.mmchs_con =  0x00000000;
	cmd.mmchs_ie =  0x00070001;
	cmd.mmchs_ise =  0x00070001;
	cmd.mmchs_cmd =  0x09090000;
	cmd.mmchs_arg =  0x00010000;
	return fs_device_send_cmd(instance, &cmd);
}
// TODO
static RESPONSE_t fs_device_send_cmd12(MMCHS_t* instance) {
	CMD_t cmd;
//	cmd.mmchs_con =  0x00000001;
//	cmd.mmchs_ie =  0x100f0001;
//	cmd.mmchs_ise =  0x100f0001;
//	cmd.mmchs_cmd =  0x371a0000;
//	cmd.mmchs_arg =  UNDEFINED_ARG;
	return fs_device_send_cmd(instance, &cmd);
}
// TODO
static RESPONSE_t fs_device_send_cmd52(MMCHS_t* instance) {
	CMD_t cmd;
//	cmd.mmchs_con =  0x00000001;
//	cmd.mmchs_ie =  0x100f0001;
//	cmd.mmchs_ise =  0x100f0001;
//	cmd.mmchs_cmd =  0x371a0000;
//	cmd.mmchs_arg =  UNDEFINED_ARG;
	return fs_device_send_cmd(instance, &cmd);
}
*/
static RESPONSE_t fs_device_send_cmd55(MMCHS_t* instance) {
	CMD_t cmd;
	cmd.mmchs_con =  0x00000001;
	cmd.mmchs_ie =  0x100f0001;
	cmd.mmchs_ise =  0x100f0001;
	cmd.mmchs_cmd =  0x371a0000;
	cmd.mmchs_arg =  UNDEFINED_ARG;
	return fs_device_send_cmd(instance, &cmd);
}
/*
static RESPONSE_t fs_device_set_bus_width_with_cmd6(MMCHS_t* instance) {
	CMD_t cmd;
	cmd.mmchs_con =  0x00000000;
	cmd.mmchs_ie =  0x100f0001;
	cmd.mmchs_ise =  0x100f0001;
	cmd.mmchs_cmd =  0x061b0000;
	cmd.mmchs_arg =  0x03b70200;
	return fs_device_send_cmd(instance, &cmd);
}
static RESPONSE_t fs_device_enable_high_speed_feature_with_cmd6(MMCHS_t* instance) {
	CMD_t cmd;
	cmd.mmchs_con =  0x00000020;
	cmd.mmchs_ie =  0x100f0001;
	cmd.mmchs_ise =  0x100f0001;
	cmd.mmchs_cmd =  0x061b0000;
	cmd.mmchs_arg =  0x03b90100;
	return fs_device_send_cmd(instance, &cmd);
}
*/
// TODO ... what is this sorcery?
static RESPONSE_t fs_device_send_acmd6(MMCHS_t* instance) {
	CMD_t cmd;
	cmd.mmchs_con =  0x00000001;
	cmd.mmchs_ie =  ((1 << 28) | (1 << 19) | (1 << 17) | (1 << 0) | (1 << 18) | (1 << 16));
	cmd.mmchs_ise =  0x100f0001;
	cmd.mmchs_cmd =  (((6 & 0x3F) << 24) | (0x2UL << 16));
	cmd.mmchs_arg =  0x2;
	return fs_device_send_cmd(instance, &cmd);
}
// TODO ... what is this sorcery?
static RESPONSE_t fs_device_send_acmd41(MMCHS_t* instance) {
	CMD_t cmd;
	cmd.mmchs_con =  0x00000001;
	cmd.mmchs_ie =  ((1 << 28) | (1 << 19) | (1 << 17) | (1 << 0) | (1 << 18) | (1 << 16));
	cmd.mmchs_ise =  0x100f0001;
	cmd.mmchs_cmd =  (((41 & 0x3F) << 24) | (0x2UL << 16));
	cmd.mmchs_arg =  UNDEFINED_ARG;
	return fs_device_send_cmd(instance, &cmd);
}

/*
 * see page 3172
 */
static RESPONSE_t fs_device_send_cmd(MMCHS_t* instance, CMD_t* cmd) {
	// (start)

	// Read the MMCHi.MMCHS_PSTATE[0] CMDI bit
	// --> CMDI = 0x0?
	// -> no? wait... command line is in use; issuing a command is not allowed.
	// -> yes? command line is not in use; issuing a command is allowed.
	while ((*(instance->PSTATE)) & MMCHS_PSTATE_CMDI != ~MMCHS_PSTATE_CMDI)
		;

	// Write MMCHi.MMCHS_CON register; set MIT, STR bit fields
	// (Bitfields in MMCHS_CON should be configured according to command features (stream or multiblock, with or without timeout))
	*(instance->CON) = cmd->mmchs_con;

	// TODO or maybe not?
	// Write MMCHi.MMCHS_CSRE register, if response type permits (see standard specifications)

	// Write MMCHi.MMCHS_BLK
	// Write MMCHi.MMCHS_SYSCTL to set DTO bitfield.
	// (MMCHS_BLK must be configured with block size and number of blocks if data are present.)
	*(instance->BLK) = MMCHS_BLEN_TRANSFER_512_BYTES;
	*(instance->SYSCTL) &= ~MMCHS_SYSCTL_DTO_RESET;
	*(instance->SYSCTL) |= MMCHS_SYSCTL_DTO_TCF_2pow27;

	// Write MMCHi.MMCHS_ARG register
	if (cmd->mmchs_arg != UNDEFINED_ARG) {
		*(instance->ARG) = cmd->mmchs_arg;
	}

	// Write MMCHi.MMCHS_IE and MMCi.MMCHS_ISE registers to enable required interrupts
	// (In order to use interrupts MMCHS_ISE must be configured. If polling is used configuring MMCHS_IE is enough).
	*(instance->IE) = cmd->mmchs_ie;

	// Write MMCi.MMCHS_CMD register
	*(instance->CMD) = cmd->mmchs_cmd;

	// - ---------- -
	// - (marker_1) -
	// - ---------- -

	unsigned int mmchs_stat_value = 0;

	int try = -1;
	while (++try < MAX_TRIES) {
		// Read the MMCi.MMCHS_STAT register
		mmchs_stat_value = *(instance->STAT);
			// CTO = 0x1 and CCRC = 0x1?
		if ( ((mmchs_stat_value) & (MMCHS_STAT_CTO | MMCHS_STAT_CCRC)) == (MMCHS_STAT_CTO | MMCHS_STAT_CCRC) ) {
			// --> yes?
			// This is a particular case that occurs when there is a conflict on the mmci_cmd line.
			// Set MMCi.MMCHS_SYSCTL[25] SRC bit to 0x1 and wait until it returns to 0x0
			*(instance->SYSCTL) |= MMCHS_SYSCTL_SRC;
			while ( (*(instance->SYSCTL) & MMCHS_SYSCTL_SRC) != ~MMCHS_SYSCTL_SRC )
				;

			// got to (end)
			return ERROR;
		} else {
			// --> no?
			// CTO = 0x1 and CCRC = 0x0?
			if ( ((mmchs_stat_value) & (MMCHS_STAT_CTO | MMCHS_STAT_CCRC)) == (MMCHS_STAT_CTO & ~MMCHS_STAT_CCRC) ) {
				// --> yes? Set the MMCI.MMCHS_SYSCTL[25] SRC bit to 0x1 and wait until it returns to 0x0
				*(instance->SYSCTL) |= MMCHS_SYSCTL_SRC;
				while ( (*(instance->SYSCTL) & MMCHS_SYSCTL_SRC) != ~MMCHS_SYSCTL_SRC )
					;

				// go to (end)
				return ERROR;
			} else {
				// --> no?
				// CC = 0x1?
				if ( (mmchs_stat_value & MMCHS_STAT_CC) == MMCHS_STAT_CC ) {
					// --> yes? Read the MMCi.MMCHS_CMD[17:16] RESP_TYPE
					// RESP_TYPE = 0x0?
					if ((*(instance->CMD) & (BIT17 | BIT16)) == 0x0) {
						// --> yes? No response -> go to (end)
						return SUCCESS;
					} else {
						// --> no? A response is waiting
						// Read the MMCi.MMCHS_RSP register
						// Verify the command errors by reading MMCi.MMCHS_STAT register (CIE, CEB, CCRC, and CERR bits)
						// TODO correct?
						volatile int response = *(instance->RSP10);
						if ( 0x0 != (mmchs_stat_value & (MMCHS_STAT_CIE | MMCHS_STAT_CEB | MMCHS_STAT_CCRC | MMCHS_STAT_CERR)) ) {
							return ERROR;
						}

						// go to (end)
						return SUCCESS;
					}

				} else {
					// --> no? got to (marker_1)
					continue; // unnecessary 'continue', i know.. but yeah. it's the way it is. and also it comes in quite handy if one wants to compare the algorithm with the specifications.
				}
			}
		}
	}

	// MAX_TRIES reached.
	return ERROR;
}

// without DMA, with polling.
static RESPONSE_t fs_device_read_write(FileHandle_t* handle, MMCHS_OPERATION_t operation) {
	// (start)

	// Read the MMCi.MMCHS_PSTATE[1] DATI bit
	// DATI = 0x0?
	// --> no? -> the data lines are in use, wait
	// --> yes? -> data lines are not in use
	while ((*(handle->instance->PSTATE)) & MMCHS_PSTATE_DATI != ~MMCHS_PSTATE_DATI)
		;

	// TODO
	// Send a data command (see command transfer flow)
	// Is there any error?
	// yes -> go to (end)
	int error = SUCCESS;
	if (error == ERROR) {
		return ERROR;
	}
	// no -> go to (A)

	// (A)

	int i = 0;
	// TODO check this stuff...
	int stop_condition = (((*(handle->instance->BLK)) & MMCHS_BLEN_TRANSFER_512_BYTES) + 3) / 4;
	while (i < stop_condition) {
		// Read the MMCi.MMCHS_STAT
		// *(handle->instance->STAT)
		// test if any error occurred
		if ((*(handle->instance->STAT)) & MMCHS_STAT_ERRI == MMCHS_STAT_ERRI) {
			return ERROR;
		}

		// Poll BWR if a write command
		// Poll BRR if a read command
		// --> BWR or BRR is set?
		// ----> no -> go to (A)
		// ----> yes -> continue
		// Write or Read 4 bytes into the MMCi.MMCHS_DATA register
		// (Repeat MMCHS_DATA access (BLEN + 3)/4 times -> go to (A))

		if (operation == READ) {
			if ((*(handle->instance->STAT)) & MMCHS_STAT_BRR != MMCHS_STAT_BRR) {
				// TODO read
				++i;
			}
		} else {
			if ((*(handle->instance->STAT)) & MMCHS_STAT_BWR == MMCHS_STAT_BWR) {
				// TODO write
				++i;
			}
		}
	}

	// - ---------- -
	// - (marker_1) -
	// - ---------- -
	while (1) {
		// Read the MMCi.MMCHS_STAT
		// TC interrupt occurred?
		unsigned int status_reg = *(handle->instance->STAT);
		if ((status_reg & MMCHS_STAT_TC) != MMCHS_STAT_TC) {
			// --> no
			// ----> DEB or DCRC or DTO interrupt occurred?
			if (((status_reg & MMCHS_STAT_DEB) == MMCHS_STAT_DEB) ||
					((status_reg & MMCHS_STAT_DCRC) == MMCHS_STAT_DCRC) ||
					((status_reg & MMCHS_STAT_DTO) == MMCHS_STAT_DTO)) {
				// -------> Yes -> There was an error during the data transfer
				// ---------> Set MMCi.MMCHS_SYSCTL[26] SRD bit to 0x1
				// 				and wait until it returns to 0x0
				fs_device_set_mmchs_sysctl_srd_and_wait_until_reset(handle->instance);
				// ---------> go to (end)
				return ERROR;
			} else {
				// -------> No -> No event rises in MMCHS_STAT (go to (marker_1))
				continue;
			}
		} else {
			// --> yes -> Transfer complete
			break;
		}
	}

	// Transfer type
	if (handle->protocol_type->type == BLOCK) {
		// --> Finite -> go to (end)
		return SUCCESS;
	} else {
		// TODO ? shall we support STREAM?

		// --> Infinite
		// ----> For MMC and SD only: Is Auto-CMD12 enabled?
		int is_auto_cmd12_enabled = 0;
		if (is_auto_cmd12_enabled) {
			// -------> Yes -> Read the MMCi.MMCHS_STAT[24] ACE bit and verify if there are errors by reading the MMCi.MMCHS_AC12 register
		} else {
			// -------> No -> Send CMD12 for MMC and SD cards / CMD52 for SDIO cards
		}

		return SUCCESS;
	}

	// (end)
}

/*
static RESPONSE_t fs_device_read_write_dma_polling(FileHandle_t* handle, MMCHS_OPERATION_t* operation) {
	// (start)

	// Read the MMCi.MMCHS_PSTATE[1] DATI bit
	// DATI = 0x0?
	// --> no? -> the data lines are in use, wait
	// --> yes? -> data lines are not in use
	while ((*(handle->instance->PSTATE)) & MMCHS_PSTATE_DATI != ~MMCHS_PSTATE_DATI)
		;

	// Configure and Enable the DMA channel (see the DMA chapter)
	fs_device_configure_and_enable_DMA(handle);

	// Send a data command (see command transfer flow)
	// is there any error?
	if (ERROR == fs_device_send_cmd0(handle->instance)) {
		// TO DO is this the right command?
		// --> yes? Disable the DMA channel (see the DMA chapter)
		fs_device_disable_DMA(handle);
	}

	// Do you want to interrupt the transfer?
	// --> yes?	(we don't want to interrupt the transfer)
	//		--> send CMD12 for MMC and SD cards; CMD52 for SDIO cards
	// --> no?
	//		--> got to (marker_1)

	// - ---------- -
	// - (marker_1) -
	// - ---------- -
	while (1) {
		unsigned int mmchs_stat = *(handle->instance->STAT);
		// Read the MMCi.MMCHS_STAT register
		// TC = 0x1?
		if ((mmchs_stat & MMCHS_STAT_TC) != MMCHS_STAT_TC) {
			// --> no?
			// (DEB or DCRC or DTO) = 0x1?
			if (mmchs_stat & (MMCHS_STAT_DEB) != MMCHS_STAT_DEB) {
				// --> no? -> go to marker_1
				continue;
			}
			if (mmchs_stat & (MMCHS_STAT_DCRC) != MMCHS_STAT_DCRC) {
				// --> no? -> go to marker_1
				continue;
			}
			if (mmchs_stat & (MMCHS_STAT_DTO) != MMCHS_STAT_DTO) {
				// --> no? -> go to marker_1
				continue;
			}
			// --> yes? an error occurred during the transfer
			//	-> Disable the DMA channel (see the DMA chapter)
			fs_device_disable_DMA(handle);

			//	-> Set MMCi.MMCHS_SYSCTL[26] SRD bits to 0x1 and wait until it returns to 0x0
			fs_device_set_mmchs_sysctl_srd_and_wait_until_reset(handle->instance);
			//	-> (end)
			return ERROR;
		} else {
			// --> yes?
			break;
		}
	}

	int success = SUCCESS;

	// TO DO
	//	-> For MMC and SD only: is Auto-CMD12 enabled?
	int auto_cmd12_enabled = 1;
	if (!auto_cmd12_enabled || handle->storage_type == SDIO) {
		// --> no? (or for SDIO)
		//	-> Send CMD12 for MMC and SD cards / CMD52 for SDIO cards
		if (handle->storage_type == MMC || handle->storage_type == SD_2 || handle->storage_type == SD_1x) {
			fs_device_send_cmd12(handle->instance);
		}
		if (handle->storage_type == SDIO) {
			fs_device_send_cmd52(handle->instance);
		}
	} else {
		// --> yes? the STOP command is automatically sent to the card
		//	-> Read the MMCi.MMCHS_STAT[24] ACE bit and verify if there are errors by reading the MMCi.MMCHS_AC12 register
		if (0x0 != (*(handle->instance->STAT) & MMCHS_STAT_ACE)) {
			success = ERROR;
		}
		if (0x0 != (*(handle->instance->AC12) & (MMCHS_AC12_ACCE | MMCHS_AC12_ACEB | MMCHS_AC12_ACIE | MMCHS_AC12_ACNE | MMCHS_AC12_ACTO | MMCHS_AC12_CNI))) {
			success = ERROR;
		}
	}

	// Disable the DMA channel (see the DMA chapter)
	fs_device_disable_DMA(handle);

	// (end)
	return success;
}

// TO DO
static RESPONSE_t fs_device_configure_and_enable_DMA(FileHandle_t* handle) {

}

// TO DO
static RESPONSE_t fs_device_disable_DMA(FileHandle_t* handle) {

}
*/
