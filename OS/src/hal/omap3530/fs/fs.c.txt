/*
 * fs.c
 *
 *  Created on: 10.05.2013
 *      Author: lumannnn
 */

#include "stdlib.h"
#include "../../../bit.h"
#include "../../../platform/omap3530/mmchs.h"
#include "../../generic/fs/fs.h"
#include "fs.h"
#include "../../../service/logger/logger.h"

#define MAX_TRIES (100*5)

#define SIGNATURE_16(a, b) ((a) | (b << 8))
/**
 * Return a 32 bit signature built from four ASCII characters
 */
#define SIGNATURE_32(a, b, c, d) (SIGNATURE_16(a, b) | (SIGNATURE_16(c, d) << 16))

typedef RESPONSE_t (*FS_DEVICE_READ) 	(MMCHS_t* instance);
typedef RESPONSE_t (*FS_DEVICE_WRITE) 	(MMCHS_t* instance);

typedef struct _FS_DEVICE_t {
	FS_DEVICE_READ read;
	FS_DEVICE_WRITE write;
} FS_DEVICE_t;

FS_DEVICE_t* fs_device;

RESPONSE_t fs_device_read(MMCHS_t* instance);
RESPONSE_t fs_device_write(MMCHS_t* instance);

// private API

typedef enum {
	READ=0,
	WRITE
} MMCHS_OPERATION_t;

static RESPONSE_t fs_device_read_write(MMCHS_t* instance, MMCHS_OPERATION_t operation);
static RESPONSE_t fs_device_transfer_block(MMCHS_t* instance, MMCHS_OPERATION_t operation);

/**
 * @param instance 1..3
 */
static void fs_device_init_mmchs(int instance_nr, MMCHS_t* instance);
// static void fs_device_setup_dma(MMCHS_t* instance); TODO ?
static void fs_device_enable_interface_and_functional_clock(MMCHS_t* instance);
static void fs_device_soft_reset(MMCHS_t* instnace);
static void fs_device_set_default_capabilities(MMCHS_t* instance);
static void fs_device_wake_up_config(MMCHS_t* instance);
static void fs_device_init_procedure_start(MMCHS_t* instance);
static void fs_device_pre_card_identification(MMCHS_t* instance);
static RESPONSE_t fs_device_mmc_host_and_bus_config(MMCHS_t* instance);

static RESPONSE_t fs_device_identify_card(MMCHS_t* instance);

// helper functions

static int fs_device_check_mmchs_stat_cc(MMCHS_t* instance);
static int fs_device_check_mmchs_stat_cto(MMCHS_t* instance);
static void fs_device_set_mmchs_sysctl_src_and_wait_until_reset(MMCHS_t* instance);
static void fs_device_set_mmchs_sysctl_srd_and_wait_until_reset(MMCHS_t* instance);

static RESPONSE_t fs_device_finish_card_identification(MMCHS_t* instance);

static void fs_device_update_clock_frequency(MMCHS_t* instance, unsigned int frequency);

static void fs_device_parse_card_cid(MMCHS_t* instance);
static RESPONSE_t fs_device_obtain_card_specific_data(MMCHS_t* instance);
static void fs_device_card_config_data(MMCHS_t* instance);
static void fs_device_block_information(MMCHS_t* instance);
static void fs_device_calc_card_CLKD(MMCHS_t* instance);

typedef struct _CMD {
	unsigned int mmchs_con;
	unsigned int mmchs_ie;
	unsigned int mmchs_ise;
	unsigned int mmchs_cmd;
	unsigned int mmchs_arg;
	unsigned int mmchs_blk;
} CMD_t;

#define UNDEFINED_ARG (0x0)
#define UNDEFINED_BLK (0x0)

static RESPONSE_t fs_device_send_cmd(MMCHS_t* instance, CMD_t* cmd);

static RESPONSE_t fs_device_send_cmd0(MMCHS_t* instance);
static RESPONSE_t fs_device_send_cmd1(MMCHS_t* instance);
static RESPONSE_t fs_device_send_cmd2(MMCHS_t* instance);
static RESPONSE_t fs_device_send_cmd3(MMCHS_t* instance);
static RESPONSE_t fs_device_send_cmd5(MMCHS_t* instance);
static RESPONSE_t fs_device_send_cmd7(MMCHS_t* instance, unsigned int mmchs_cmd);
static RESPONSE_t fs_device_send_cmd8(MMCHS_t* instance);
static RESPONSE_t fs_device_send_cmd9(MMCHS_t* instance, unsigned int mmchs_cmd);
//static RESPONSE_t fs_device_send_cmd12(MMCHS_t* instance);
//static RESPONSE_t fs_device_send_cmd52(MMCHS_t* instance);
static RESPONSE_t fs_device_send_cmd55(MMCHS_t* instance);
//static RESPONSE_t fs_device_set_bus_width_with_cmd6(MMCHS_t* instance);
//static RESPONSE_t fs_device_enable_high_speed_feature_with_cmd6(MMCHS_t* instance);
static RESPONSE_t fs_device_send_acmd6(MMCHS_t* instance);
static RESPONSE_t fs_device_send_acmd41(MMCHS_t* instance, unsigned int mmchs_cmd);

static RESPONSE_t fs_device_send_cmd16(MMCHS_t* instance, unsigned int mmchs_cmd);
static RESPONSE_t fs_device_send_cmd23(MMCHS_t* instance);
static RESPONSE_t fs_device_send_cmd25(MMCHS_t* instance);
static RESPONSE_t fs_device_send_cmd18(MMCHS_t* instance, unsigned int mmchs_cmd);

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

RESPONSE_t fs_init() {
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
	instance = malloc(sizeof(MMCHS_t));
	fs_device = malloc(sizeof(FS_DEVICE_t));

	fs_device_init_mmchs(1, instance);
	MMCHS_BLOCK_t block = {
		SIGNATURE_32('s', 'd', 'i', 'o'),
		TRUE,	// not implemented
		FALSE,
		FALSE,	// not implemented
		FALSE,	// not implemented
		512,
		4,		// alignment for any buffer that read/writes blocks
		0		// the logical last block.
	};
	instance->block = block;

	fs_device_enable_interface_and_functional_clock(instance);
	fs_device_soft_reset(instance);
	fs_device_set_default_capabilities(instance);
	fs_device_init_procedure_start(instance);
	fs_device_pre_card_identification(instance);
	fs_device_wake_up_config(instance);
	fs_device_mmc_host_and_bus_config(instance);

	RESPONSE_t status = SUCCESS;
	status = fs_device_identify_card(instance);

	fs_device->read = &fs_device_read;
	fs_device->write = &fs_device_write;

	fl_init();
	fl_attach_media(&fs_read, &fs_write);

	return status;
}

int fs_read(uint32 sector, uint8 *buffer, uint32 sector_count) {
	if (sector > instance->block.lba) {
		return ERROR;
	}
	instance->lba = sector;
	instance->buffer_size = sector_count * 512; // TODO #define ... 512
	instance->expected_block_count = sector_count;
	instance->buffer = buffer;
	return fs_device->read(instance);
}

int fs_write(uint32 sector, uint8 *buffer, uint32 sector_count) {
	if (sector > instance->block.lba) {
		return ERROR;
	}

	instance->lba = sector;
	instance->buffer_size = sector_count * 512; // TODO #define ... 512
	instance->expected_block_count = sector_count;
	instance->buffer = buffer;
	return fs_device->write(instance);
}

RESPONSE_t fs_device_read(MMCHS_t* instance) {
	return fs_device_read_write(instance, READ);
}

RESPONSE_t fs_device_write(MMCHS_t* instance) {
 	return fs_device_read_write(instance, WRITE);
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
		fs_device_logger_error("unsupported instance requested.");
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

	instance->buffer = malloc(sizeof(unsigned int*) * 512);
}

static void fs_device_enable_interface_and_functional_clock(MMCHS_t* instance) {
	// Prior to any MMCHS register access one must enable MMCHS interface clock and functional clock in
	// PRCM module registers PRCM.CM_ICLKEN1_CORE and PRCM.CM_FCLKEN1_CORE. Please refer to
	// Chapter 4, Power, Reset, and Clock Management.
	*(CM_FCLKEN1_CORE) |= (CM_CLKEN1_CORE_EN_MMC1 | CM_CLKEN1_CORE_EN_MMC2 | CM_CLKEN1_CORE_EN_MMC3);
	*(CM_ICLKEN1_CORE) |= (CM_CLKEN1_CORE_EN_MMC1 | CM_CLKEN1_CORE_EN_MMC2 | CM_CLKEN1_CORE_EN_MMC3);
//	*(instance->SYSCONFIG) |= MMCHS_SYSCOFNIG_AUTOIDLE;
//	*(CM_AUTOIDLE1_CORE) |= (CM_AUTOIDLE1_AUTO_MMC1 | CM_AUTOIDLE1_AUTO_MMC2 | CM_AUTOIDLE1_AUTO_MMC3);
}

static void fs_device_soft_reset(MMCHS_t* instance) {
	// see page 3161
	// set the MMCi.MMCHS_SYSCONFIG[1] SOFTRESET bit to 0x1
	*(instance->SYSCONFIG) = MMCHS_SYSCONFIG_SOFTRESET;
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
//	*(instance->IE) |= MMCHS_IE_CIRQ_ENABLE;

	*(instance->IE) |= (
			MMCHS_IE_CC_ENABLE
			|  MMCHS_IE_TC_ENABLE
			|  MMCHS_IE_BGE_ENABLE
			|  MMCHS_IE_BWR_ENABLE
			|  MMCHS_IE_BRR_ENABLE
			|  MMCHS_IE_CIRQ_ENABLE
			|  MMCHS_IE_OBI_ENABLE
			|  MMCHS_IE_CTO_ENABLE
			|  MMCHS_IE_CCRC_ENABLE
			|  MMCHS_IE_CEB_ENABLE
			|  MMCHS_IE_CIE_ENABLE
			|  MMCHS_IE_DTO_ENABLE
			|  MMCHS_IE_DCRC_ENABLE
			|  MMCHS_IE_DEB_ENABLE
			|  MMCHS_IE_ACE_ENABLE
			|  MMCHS_IE_CERR_ENABLE
			|  MMCHS_IE_BADA_ENABLE);
}

static void fs_device_init_procedure_start(MMCHS_t* instance) {
	*(instance->CON) |= MMCHS_CON_INIT;
	// send dummy command
	*(instance->CMD) = 0x00000000;
	*(CONTROL_PADCONF_MMC1_CLK) = 0x100;
}

static void fs_device_pre_card_identification(MMCHS_t* instance) {
	*(instance->HCTL) = 0x00000b00;
	*(instance->SYSCTL) = 0x00003C07;
	*(instance->CON) = 0x00000001;
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
		// default is value: 3.0V - we leave it that way.
		*(instance->HCTL) |= MMCHS_HCTL_SDVS_3V0;
	}

	*(instance->HCTL) |= MMCHS_HCTL_SDBP;
	*(instance->HCTL) &= ~MMCHS_HCTL_DTW;

	// For SD/SDIO cards:
	// following a SET_BUS_WIDTH command (ACMD6) with a value written in bit 1 of the argument.
	// Prior to this command, the SD card configuration register (SCR) must be verified for the supported bus width by the SD card.
	// \
	//  \
	//   |
	//   V
	// Send ACMD6 (application specific commands must be prefixed with CMD55)
	// if no SD 1.x card is inserted, the following commands will fail.
	RESPONSE_t status = SUCCESS;
	status = fs_device_send_cmd55(instance);
	if (status == ERROR) {
		fs_device_logger_debug("fs_device_mmc_host_and_bus_config: cmd55 failed.");
//		return ERROR;
	}
	status = fs_device_send_acmd6(instance);
	if (status == ERROR) {
		fs_device_logger_debug("fs_device_mmc_host_and_bus_config: acmd6 failed.");
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
static RESPONSE_t fs_device_identify_card(MMCHS_t* instance) {
	// (Start)

	// do module initialization before this
	RESPONSE_t status = ERROR;
	unsigned int response = 0x0;
	int cmd8_supported = TRUE;
	unsigned int mmchs_cmd = 0;

	// set MMCi.MMCHS_CON[1] INT bit to 0x1 to send an initialization stream
	*(instance->CON) |= MMCHS_CON_INIT;

	// write 0x00000000 in the MMCi.MMCHS_CMD register
	*(instance->CMD) = 0x00000000;

	// (wait 1 ms)
	// FIXME
	volatile int i = 0;
	while (++i < 200000);

	// set MMCi.MMCHS_STAT[0] CC bit to 0x1 to clear the flag
	*(instance->STAT) |= MMCHS_STAT_CC;

	// set MMCi.MMCHS_CON[1] INT bit to 0x0 to end the initialization sequence
	*(instance->CON) &= ~MMCHS_CON_INIT;

	// clear MMCHS_STAT register (write 0xFFFF FFFF)
	*(instance->STAT) = 0xFFFFFFFF;

	// change clock frequency to fit protocol
	fs_device_update_clock_frequency(instance, MMCHS_SYSCTL_CLKD_400KHZ);

	// send CMD0 command
	fs_device_send_cmd0(instance);

	// (A)
	// send a CMD5 command
	// The command will fail if there is no SDIO card.
	// In case of success: response in MMCHS_RSP10 register
	status = fs_device_send_cmd5(instance);
	if (status == ERROR) {
		fs_device_logger_debug("fs_device_identify_card: cmd5 failed.");
	} else {
		response = *(instance->RSP10);
	}

	// read the MMCi.MMCHS_STAT register
	int try = -1;
	while (++try < MAX_TRIES) {
		// CC = 0x1
		if (fs_device_check_mmchs_stat_cc(instance) == 1) {
			// it is an SDIO card
			instance->storage_type = SDIO;
			fs_device_logger_debug("SDIO card detected");
			fs_device_logger_error("SDIO cards are not supported.");

			// See the SDIO Standard Specification to identify the card type:
			// Memory only, I/O only, Combo

			// goto (End)
			return ERROR;
		}
		// CTO = 0x1
		if (fs_device_check_mmchs_stat_cto(instance) == 1) {
			break;
		}
	}

	fs_device_set_mmchs_sysctl_src_and_wait_until_reset(instance);
	// The command will fail if there is no SD 2.x card.
	// In case of success: response in MMCHS_RSP10
	status = fs_device_send_cmd8(instance);
	if (status == ERROR) {
		fs_device_logger_debug("fs_device_identify_card: cmd8 failed.");
		cmd8_supported = 0; // false;
	} else {
		response = *(instance->RSP10);
	}

	// read the MMCi.MMCHS_STAT register
	try = -1;
	while (++try < MAX_TRIES) {
		// CC = 0x1
		if (fs_device_check_mmchs_stat_cc(instance) == 1) {
			// yes? (it is an SD card compliant with standard 2.0 or later)
			instance->storage_type = SD_2;
			fs_device_logger_debug("SD card compliant with standard 2.0 or later detected.");

			// see the SD Standard Specification version 2.0 or later
			// to identify the card type: High Capacity; Standard Capacity

			// debug
			unsigned int response = *(instance->RSP10);
			fs_device_logger_debug("CMD8 success.");
			unsigned int CMD8_ARG = (0x0UL << 12 | (1 << 8) | 0xCEUL << 0);
			if (response != CMD8_ARG) {
//				return ERROR;
			}

			instance->storage_type = SD_2_HC;

			fs_device_finish_card_identification(instance);

			// goto (End)
			return SUCCESS;
		}
		// CTO = 0x1
		if (fs_device_check_mmchs_stat_cto(instance) == 1) {
			break;
		}
	}

	fs_device_set_mmchs_sysctl_src_and_wait_until_reset(instance);

	int abort = 0;
	while (abort == 0) {
		// see page 3165.
		// marker:
		// ---
		// |A|
		//  V

		// send a CMD5 command
		// The command will fail if there is no SDIO card.
		// In case of success: response in MMCHS_RSP10 register
		status = fs_device_send_cmd55(instance);
		if (status == ERROR) {
			fs_device_logger_debug("fs_device_identify_card: cmd55 failed.");
		}

		response = *(instance->RSP10);
		((unsigned int*) &(instance->OCR))[0] = response;
		unsigned int mmchs_cmd = response;
		if (cmd8_supported) {
			mmchs_cmd |=  (1 << 30);
		}
		status = fs_device_send_acmd41(instance, mmchs_cmd);
		if (status == ERROR) {
			fs_device_logger_debug("fs_device_identify_card: acmd41 failed.");
		}

		// read the MMCi.MMCHS_STAT register
		while (1) {
			// CC = 0x1
			if (fs_device_check_mmchs_stat_cc(instance) == 1) {
				// (it is a SD card compliant with standard 1.x)
				instance->storage_type = SD_1x;
				fs_device_logger_debug("SD card compliant with standard 1.x or later detected.");

				// verify the card is busy:
				// read the MMCi.MMCHS_RSP10[31] bit
				// equal to 0x1?
				response = *(instance->RSP10);
				((unsigned int*) &(instance->OCR))[0] = response;
				if (response == (response) | BIT31) {
					// --> yes? -> the card is not busy
					// 			goto (B)
					return fs_device_finish_card_identification(instance);
				} else {
					// --> no? -> the card is busy
					//			goto ---
					//				 |A|
					//				  V
					break;
				}
			}
			// CTO = 0x1
			if (fs_device_check_mmchs_stat_cto(instance) == 1) {
				// it is a MMC card
				abort = 1;
				break;
			}
		}
	}


	// (it is a MMC card)
	instance->storage_type = MMC;
	fs_device_set_mmchs_sysctl_src_and_wait_until_reset(instance);
	// FIX ME:
	// As of now, we should not get to here. We don't have any MMC card so we do not support it.
	// If you want to support MMC cards: uncomment following code block; test it.
//	return ERROR;

	while (1) {
		// - ---------- -
		// - (marker_1) -
		// - ---------- -

		// TO DO what's this OCR 0?
		// send a CMD1 command* (*With OCR 0. In case of a CMD1 with OCR=0, a second CMD1 must be sent to the card with the "negociated" voltage.
		status = fs_device_send_cmd1(instance);
		if (status == ERROR) {
			fs_device_logger_debug("fs_device_identify_card: cmd1 failed.");
//			return ERROR;
		}

		// read the MMCi.MMCHS_STAT register
		while (1) {
			// CTO = 0x1
			if (fs_device_check_mmchs_stat_cto(instance) == 1) {
				// unknown type of card
				instance->storage_type = UNKNOWN;
				fs_device_logger_debug("unknown card type.");
				// goto (end)
				return ERROR;
			}

			// CC = 0x1
			if (fs_device_check_mmchs_stat_cc(instance) == 1) {
				break;
			}
		}

		// it is a MMC card
		instance->storage_type = MMC;
		fs_device_logger_debug("MMC card detected.");

		// Verify the card is busy:
		// read the MMCi.MMCHS_RSP19[31] bit --> typo in OMPA35x.pdf: should be MMCHS_RSP10
		// equal to 0x1?
		unsigned int* mmchs_rsp10 = instance->RSP10;
		if (*(mmchs_rsp10) == (*(mmchs_rsp10)) | BIT31) {
			// --> yes? -> the card is not busy
			// 			goto (B)
			return fs_device_finish_card_identification(instance);
		} else {
			// --> no? (the card is busy)
			// 			-> goto (marker_1)
			fs_device_logger_debug("card is busy...");
		}
	}

	// (end)
}

/**
 * (B)
 */
static RESPONSE_t fs_device_finish_card_identification(MMCHS_t* instance) {
	// (B)
	int status = SUCCESS;

	// TODO what about the information?
	// send a CMD2 command to get information on how to access the card content
	status = fs_device_send_cmd2(instance);
	if (status == ERROR) {
		fs_device_logger_error("fs_device_identify_card: fs_device_finish_card_identification: cmd2 failed.");
	}
	fs_device_parse_card_cid(instance);

	status = fs_device_send_cmd3(instance);
	instance->RCA = (*(instance->RSP10) >> 16);
	if (status == ERROR) {
		fs_device_logger_error("fs_device_identify_card: fs_device_finish_card_identification: cmd3 failed.");
//		return ERROR;
	}

	// card type?
	if (instance->storage_type == MMC) {
		// --> MMC card? -> Is there more than one MMC connected to the same bus, and are they all identified?
		//			--> yes -> goto (B)
		//			--> no -> (continue)

		// nothing to do here...
		// we do not support MMC cards yet.
		fs_device_logger_error("MMC cards are not supported.");
		status = ERROR;
	}

	// (continue)

	status = fs_device_send_cmd7(instance, (instance->RCA << 16));
	if (status == ERROR) {
		fs_device_logger_error("fs_device_identify_card: fs_device_finish_card_identification: cmd7 failed.");
	}

	status = fs_device_obtain_card_specific_data(instance);

	instance->block.lba = (instance->num_blocks - 1);
	instance->block.block_size = instance->block_size;
	instance->block.read_only = FALSE;

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
	while (((*(instance->SYSCTL)) & MMCHS_SYSCTL_SRC) == MMCHS_SYSCTL_SRC)
			;
}

static void fs_device_set_mmchs_sysctl_srd_and_wait_until_reset(MMCHS_t* instance) {
	// set MMCi.MMCHS_SYSCTL[26] SRD bit to 0x1
	*(instance->SYSCTL) |= MMCHS_SYSCTL_SRD;
	// and wait until it returns to 0x0
	while (((*(instance->SYSCTL)) & MMCHS_SYSCTL_SRD) == MMCHS_SYSCTL_SRD)
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

static void fs_device_parse_card_cid(MMCHS_t* instance) {
	instance->CID.MDT = 	((*(instance->RSP10) >> 8) & 0xFFF);
	instance->CID.PSN = 	(((*(instance->RSP10) >> 24) & 0xFF) | (((*(instance->RSP32)) & 0xFFFFFF) << 8));
	instance->CID.PRV = 	((*(instance->RSP32) >> 24) & 0xFF);
	instance->CID.PNM[4] = 	(*(instance->RSP54) & 0xFF);
	instance->CID.PNM[3] = 	((*(instance->RSP54) >> 8) & 0xFF);
	instance->CID.PNM[2] = 	((*(instance->RSP54) >> 16) & 0xFF);
	instance->CID.PNM[1] = 	((*(instance->RSP54) >> 24) & 0xFF);
	instance->CID.PNM[0] = 	(*(instance->RSP76) & 0xFF);
	instance->CID.OID = 	((*(instance->RSP76) >> 8) & 0xFFFF);
	instance->CID.MID = 	((*(instance->RSP76) >> 24) & 0xFF);
}

static RESPONSE_t fs_device_obtain_card_specific_data(MMCHS_t* instance) {
	RESPONSE_t status;
	status = fs_device_send_cmd9(instance, instance->RCA << 16);
	if (status == ERROR) {
		return status;
	}

 //Populate 128-bit CSD register data.
  ((unsigned int*) &(instance->CSD))[0] = *(instance->RSP10);
  ((unsigned int*) &(instance->CSD))[1] = *(instance->RSP32);
  ((unsigned int*) &(instance->CSD))[2] = *(instance->RSP54);
  ((unsigned int*) &(instance->CSD))[3] = *(instance->RSP76);

  //Calculate total number of blocks and max. data transfer rate supported by the detected card.
  fs_device_card_config_data(instance);

  return status;
}

static void fs_device_card_config_data(MMCHS_t* instance) {
	fs_device_block_information(instance);
	fs_device_calc_card_CLKD(instance);
}

static void fs_device_block_information(MMCHS_t* instance) {
	MMCHS_CSD_SD2_t* csd_sd2;
	unsigned int card_size;

	if (instance->storage_type == SD_2_HC) {
		csd_sd2 = (MMCHS_CSD_SD2_t *) &(instance->CSD);

		// Populate block_size.
		instance->block_size = (0x1UL << csd_sd2->READ_BL_LEN);

		// Calculate Total number of blocks.
		card_size = csd_sd2->C_SIZELow16 | (csd_sd2->C_SIZEHigh6 << 2);
		instance->num_blocks = ((card_size + 1) * 1024);
	} else {
		// Populate block_size.
		instance->block_size = (0x1UL << instance->CSD.READ_BL_LEN);

		// Calculate Total number of blocks.
		card_size = (instance->CSD.C_SIZELow2 | instance->CSD.C_SIZEHigh10 << 2);
		instance->num_blocks = (card_size + 1) * (1 << (instance->CSD.C_SIZE_MULT + 2));
	}

	// For >=2G card, block_size may be 1K, but the transfer size is 512 bytes.
	if (instance->block_size > 512) {
		instance->block_size = 512;
	}
}

static void fs_device_calc_card_CLKD(MMCHS_t* instance) {
	unsigned char max_data_transfer_rate = instance->CSD.TRAN_SPEED;
	unsigned int transfer_rate_value = 0;
	unsigned int time_value = 0;
	unsigned int frequency = 0;

	// For SD Cards  we would need to send CMD6 to set
	// speeds above 25MHz. High Speed mode 50 MHz and up

	//Calculate Transfer rate unit (Bits 2:0 of TRAN_SPEED)
	switch (max_data_transfer_rate & 0x7) {
	case 0:
	  transfer_rate_value = 100 * 1000;
	  break;

	case 1:
	  transfer_rate_value = 1 * 1000 * 1000;
	  break;

	case 2:
	  transfer_rate_value = 10 * 1000 * 1000;
	  break;

	case 3:
	  transfer_rate_value = 100 * 1000 * 1000;
	  break;

	default:
	  fs_device_logger_error("fs_device_calc_card_CLKD: Invalid rate unit parameter.");
	}

	//Calculate Time value (Bits 6:3 of TRAN_SPEED)
	switch ((max_data_transfer_rate >> 3) & 0xF) {
	case 1:
	  time_value = 10;
	  break;

	case 2:
	  time_value = 12;
	  break;

	case 3:
	  time_value = 13;
	  break;

	case 4:
	  time_value = 15;
	  break;

	case 5:
	  time_value = 20;
	  break;

	case 6:
	  time_value = 25;
	  break;

	case 7:
	  time_value = 30;
	  break;

	case 8:
	  time_value = 35;
	  break;

	case 9:
	  time_value = 40;
	  break;

	case 10:
	  time_value = 45;
	  break;

	case 11:
	  time_value = 50;
	  break;

	case 12:
	  time_value = 55;
	  break;

	case 13:
	  time_value = 60;
	  break;

	case 14:
	  time_value = 70;
	  break;

	case 15:
	  time_value = 80;
	  break;

	default:
		fs_device_logger_error("fs_device_calc_card_CLKD: Invalid transfer speed parameter.\n");
	}

	frequency = transfer_rate_value * time_value / 10;

	// Calculate Clock divider value to program in MMCHS_SYSCTL[CLKD] field.
	instance->clk_frq_select = ((MMCHS_REF_CLK / frequency) + 1);
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
	cmd.mmchs_blk =  UNDEFINED_BLK;
	return fs_device_send_cmd(instance, &cmd);
}
static RESPONSE_t fs_device_send_cmd1(MMCHS_t* instance) {
	CMD_t cmd;
	cmd.mmchs_con = 0x00000001;
	cmd.mmchs_ie = 0x00050001;
	cmd.mmchs_ise = 0x00050001;
	cmd.mmchs_cmd = 0x01020000;
	cmd.mmchs_arg =  UNDEFINED_ARG;
	cmd.mmchs_blk =  UNDEFINED_BLK;
	return fs_device_send_cmd(instance, &cmd);
}
static RESPONSE_t fs_device_send_cmd2(MMCHS_t* instance) {
	CMD_t cmd;
	cmd.mmchs_con =  0x00000001;
	cmd.mmchs_ie =  0x00070001;
	cmd.mmchs_ise =  0x00070001;
	cmd.mmchs_cmd =  0; //0x02090000;
	cmd.mmchs_arg =  UNDEFINED_ARG;
	cmd.mmchs_blk =  UNDEFINED_BLK;
	return fs_device_send_cmd(instance, &cmd);
}
static RESPONSE_t fs_device_send_cmd3(MMCHS_t* instance) {
	CMD_t cmd;
	cmd.mmchs_con =  0x00000001;
	cmd.mmchs_ie =  0x100f0001;
	cmd.mmchs_ise =  0x100f0001;
	cmd.mmchs_cmd =  0; // TODO set this to 0 and test (finish_card_ident...) 0x031a0000;
	cmd.mmchs_arg =  0x00010000;
	cmd.mmchs_blk =  UNDEFINED_BLK;
	return fs_device_send_cmd(instance, &cmd);
}
static RESPONSE_t fs_device_send_cmd5(MMCHS_t* instance) {
	CMD_t cmd;
	cmd.mmchs_con =  0x00000001;
	cmd.mmchs_ie =  0x00050001;
	cmd.mmchs_ise =  0x00050001;
	cmd.mmchs_cmd =  0x05020000;
	cmd.mmchs_arg =  UNDEFINED_ARG;
	cmd.mmchs_blk =  UNDEFINED_BLK;
	return fs_device_send_cmd(instance, &cmd);
}
static RESPONSE_t fs_device_send_cmd7(MMCHS_t* instance, unsigned int mmchs_cmd) {
	CMD_t cmd;
	cmd.mmchs_con =  0x00000000;
	cmd.mmchs_ie =  0x100f0001;
	cmd.mmchs_ise =  0x100f0001;
	cmd.mmchs_cmd =  mmchs_cmd; // 0x071a0000;
	cmd.mmchs_arg =  0x00010000;
	cmd.mmchs_blk =  UNDEFINED_BLK;
	return fs_device_send_cmd(instance, &cmd);
}
static RESPONSE_t fs_device_send_cmd8(MMCHS_t* instance) {
	CMD_t cmd;
	cmd.mmchs_con =  0x00000001;
	cmd.mmchs_ie =  0x100f0001;
	cmd.mmchs_ise =  0x100f0001;
	cmd.mmchs_cmd =  0x81a0000;
	cmd.mmchs_arg =  (0x0UL << 12 | BIT8 | 0xCEUL << 0); //UNDEFINED_ARG;
	cmd.mmchs_blk =  UNDEFINED_BLK;
	return fs_device_send_cmd(instance, &cmd);
}
static RESPONSE_t fs_device_send_cmd9(MMCHS_t* instance, unsigned int mmchs_cmd) {
	CMD_t cmd;
	cmd.mmchs_con =  0x00000000;
	cmd.mmchs_ie =  0x00070001;
	cmd.mmchs_ise =  0x00070001;
	cmd.mmchs_cmd =  mmchs_cmd; //0x09090000;
	cmd.mmchs_arg =  0x00010000;
	cmd.mmchs_blk =  UNDEFINED_BLK;
	return fs_device_send_cmd(instance, &cmd);
}
/*
// TODO
static RESPONSE_t fs_device_send_cmd12(MMCHS_t* instance) {
	CMD_t cmd;
//	cmd.mmchs_con =  0x00000001;
//	cmd.mmchs_ie =  0x100f0001;
//	cmd.mmchs_ise =  0x100f0001;
//	cmd.mmchs_cmd =  0x371a0000;
//	cmd.mmchs_arg =  UNDEFINED_ARG;
//	cmd.mmchs_blk =  UNDEFINED_BLK;
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
//	cmd.mmchs_blk =  UNDEFINED_BLK;
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
	cmd.mmchs_blk =  UNDEFINED_BLK;
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
	cmd.mmchs_blk =  UNDEFINED_BLK;
	return fs_device_send_cmd(instance, &cmd);
}
static RESPONSE_t fs_device_enable_high_speed_feature_with_cmd6(MMCHS_t* instance) {
	CMD_t cmd;
	cmd.mmchs_con =  0x00000020;
	cmd.mmchs_ie =  0x100f0001;
	cmd.mmchs_ise =  0x100f0001;
	cmd.mmchs_cmd =  0x061b0000;
	cmd.mmchs_arg =  0x03b90100;
	cmd.mmchs_blk =  UNDEFINED_BLK;
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
	cmd.mmchs_blk =  UNDEFINED_BLK;
	return fs_device_send_cmd(instance, &cmd);
}
// TODO ... what is this sorcery?
static RESPONSE_t fs_device_send_acmd41(MMCHS_t* instance, unsigned int mmchs_cmd) {
	CMD_t cmd;
	cmd.mmchs_con =  0x00000001;
	cmd.mmchs_ie =  ((1 << 28) | (1 << 19) | (1 << 17) | (1 << 0) | (1 << 18) | (1 << 16));
	cmd.mmchs_ise =  0x100f0001;
	cmd.mmchs_cmd =  mmchs_cmd;
	cmd.mmchs_arg =  UNDEFINED_ARG;
	cmd.mmchs_blk =  UNDEFINED_BLK;
	return fs_device_send_cmd(instance, &cmd);
}

// TODO
static RESPONSE_t fs_device_send_cmd16(MMCHS_t* instance, unsigned int mmchs_cmd) {
	CMD_t cmd;
	cmd.mmchs_con =  0x00000020;
	if (instance->storage_type != MMC) {
		cmd.mmchs_con &= ~MMCHS_CON_DW8;
	}
	cmd.mmchs_ie =  0x100f0001;
	cmd.mmchs_ise =  0x100f0001;
	cmd.mmchs_cmd =	mmchs_cmd; // 0x101a0000;
	cmd.mmchs_arg =  0x00000200;
	cmd.mmchs_blk =  UNDEFINED_BLK;
	return fs_device_send_cmd(instance, &cmd);
}
static RESPONSE_t fs_device_send_cmd23(MMCHS_t* instance) {
	CMD_t cmd;
	cmd.mmchs_con =  0x00000020;
	if (instance->storage_type != MMC) {
		cmd.mmchs_con &= ~MMCHS_CON_DW8;
	}
	cmd.mmchs_ie =	0x100f0001;
	cmd.mmchs_ise =  0x100f0001;
	cmd.mmchs_cmd =	0x171a0000;
	cmd.mmchs_arg =  instance->expected_block_count; //0x00000008;		// nr of blocks to be expected
	cmd.mmchs_blk =  UNDEFINED_BLK;
	return fs_device_send_cmd(instance, &cmd);
}
static RESPONSE_t fs_device_send_cmd25(MMCHS_t* instance) {
	CMD_t cmd;
	cmd.mmchs_con =  0x00000020;
	if (instance->storage_type != MMC) {
		cmd.mmchs_con &= ~MMCHS_CON_DW8;
	}
	cmd.mmchs_ie =	0x107f0013;
	cmd.mmchs_ise =  0x107f0013;
	cmd.mmchs_cmd =	0x193a0023;
	cmd.mmchs_arg =  0x00000000;
	cmd.mmchs_blk =  0x00080200;
	return fs_device_send_cmd(instance, &cmd);
}
static RESPONSE_t fs_device_send_cmd18(MMCHS_t* instance, unsigned int mmchs_cmd) {
	CMD_t cmd;
	cmd.mmchs_con =  0x00000020;
	if (instance->storage_type != MMC) {
		cmd.mmchs_con &= ~MMCHS_CON_DW8;
	}
	cmd.mmchs_ie =  0x107f0023;
	cmd.mmchs_ise =  0x107f0023;
	cmd.mmchs_cmd =  mmchs_cmd; // 0x123a0033;
	cmd.mmchs_arg =  0x00000000;
					// (number_blocks << 16 | block_length)
	cmd.mmchs_blk = ((instance->expected_block_count << 16) | instance->block.block_size);// 0x00080200;
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
	while (~((*(instance->PSTATE)) & MMCHS_PSTATE_CMDI) == ~MMCHS_PSTATE_CMDI)
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

	if (cmd->mmchs_blk != UNDEFINED_BLK) {
		*(instance->BLK) = cmd->mmchs_blk;
	}

	// clear the status register.
	*(instance->STAT) = 0xFFFFFFFF;

	// Write MMCHi.MMCHS_IE and MMCi.MMCHS_ISE registers to enable required interrupts
	// (In order to use interrupts MMCHS_ISE must be configured. If polling is used configuring MMCHS_IE is enough).
	*(instance->IE) = cmd->mmchs_ie;
	*(instance->ISE) = cmd->mmchs_ise;

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
			fs_device_set_mmchs_sysctl_src_and_wait_until_reset(instance);

			// got to (end)
			return ERROR;
		} else {
			// --> no?
			// CTO = 0x1 and CCRC = 0x0?
			if ( ((mmchs_stat_value) & (MMCHS_STAT_CTO | MMCHS_STAT_CCRC)) == (MMCHS_STAT_CTO & ~MMCHS_STAT_CCRC) ) {
				// --> yes? Set the MMCI.MMCHS_SYSCTL[25] SRC bit to 0x1 and wait until it returns to 0x0
				fs_device_set_mmchs_sysctl_src_and_wait_until_reset(instance);

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
static RESPONSE_t fs_device_read_write(MMCHS_t* instance, MMCHS_OPERATION_t operation) {
	RESPONSE_t status = SUCCESS;

	if ((instance->buffer_size % instance->block.block_size) != 0) {
		fs_device_logger_error("fs_device_read_write: invalid buffer/block size.");
		status = ERROR;
//		return ERROR;
	}

	// (start)

	// Read the MMCi.MMCHS_PSTATE[1] DATI bit
	// DATI = 0x0?
	// --> no? -> the data lines are in use, wait
	// --> yes? -> data lines are not in use
	while (((*(instance->PSTATE)) & MMCHS_PSTATE_DATI) == MMCHS_PSTATE_DATI)
		;

	// TODO
	// Send a data command (see command transfer flow)
	status = fs_device_send_cmd0(instance);
	// Is there any error?
	// yes -> go to (end)
	if (status == ERROR) {
		return ERROR;
	}
	// no -> go to (A)

	// (A)

	unsigned int bytes_to_be_transfered_this_pass = instance->block.block_size; //block_size or buffer_size?;
	unsigned int bytes_remaining_to_be_transfered = instance->buffer_size;
	unsigned int block_count = bytes_to_be_transfered_this_pass / instance->block.block_size;
	// this seems to be quite some bullshit...

	int i = 0;
	int stop_condition = (((*(instance->BLK)) & MMCHS_BLEN_TRANSFER_512_BYTES) + 3) / 4;
	while (i < stop_condition) {
		// Read the MMCi.MMCHS_STAT
		// *(instance->STAT)
		// test if any error occurred
		if (((*(instance->STAT)) & MMCHS_STAT_ERRI) == MMCHS_STAT_ERRI) {
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
			if (((*(instance->STAT)) & MMCHS_STAT_BRR) != MMCHS_STAT_BRR) {
				// clear BRR
				*(instance->STAT) |= MMCHS_STAT_BRR;
				// TODO read
				fs_device_transfer_block(instance, operation);
			}
		} else {
			if (((*(instance->STAT)) & MMCHS_STAT_BWR) == MMCHS_STAT_BWR) {
				// TODO write
			}
		}

		bytes_remaining_to_be_transfered -= bytes_to_be_transfered_this_pass;
		instance->lba += block_count;
		char cdata = (*((char*) instance->buffer));
		fs_device_logger_debug(&cdata);
		instance->buffer = (unsigned char* ) instance->buffer + instance->block_size;
		char ci = (char)(((int)'0')+i);
		fs_device_logger_debug(&ci);
		++i;
	}

	// - ---------- -
	// - (marker_1) -
	// - ---------- -
	while (1) {
		// Read the MMCi.MMCHS_STAT
		// TC interrupt occurred?
		unsigned int status_reg = *(instance->STAT);
		if ((status_reg & MMCHS_STAT_TC) != MMCHS_STAT_TC) {
			// --> no
			// ----> DEB or DCRC or DTO interrupt occurred?
			if (((status_reg & MMCHS_STAT_DEB) == MMCHS_STAT_DEB) ||
					((status_reg & MMCHS_STAT_DCRC) == MMCHS_STAT_DCRC) ||
					((status_reg & MMCHS_STAT_DTO) == MMCHS_STAT_DTO)) {
				// -------> Yes -> There was an error during the data transfer
				// ---------> Set MMCi.MMCHS_SYSCTL[26] SRD bit to 0x1
				// 				and wait until it returns to 0x0
				fs_device_set_mmchs_sysctl_srd_and_wait_until_reset(instance);
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
	if (instance->protocol_type.type == BLOCK) {
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

static RESPONSE_t fs_device_transfer_block(MMCHS_t* instance, MMCHS_OPERATION_t operation) {
	RESPONSE_t status = SUCCESS;

	unsigned int count = 0;
	if (operation == READ) {
		status = fs_device_send_cmd16(instance, instance->block.block_size);
		status = fs_device_send_cmd23(instance);
		status = fs_device_send_cmd18(instance, instance->lba * instance->block.block_size);

		unsigned char* data_buffer = instance->buffer;
		unsigned int data_size = 4; //instance->block.block_size / 4;
		for (count = 0; count < data_size; count++) {
			*data_buffer++ = *(instance->DATA);
		}
	} else {
		// write
		status = fs_device_send_cmd16(instance, instance->block.block_size);
		status = fs_device_send_cmd23(instance);
		status = fs_device_send_cmd25(instance);
	}

	return status;
}

/*
static RESPONSE_t fs_device_read_write_dma_polling(FileHandle_t* handle, MMCHS_OPERATION_t* operation) {
	// (start)

	// Read the MMCi.MMCHS_PSTATE[1] DATI bit
	// DATI = 0x0?
	// --> no? -> the data lines are in use, wait
	// --> yes? -> data lines are not in use
	while (((*(handle->instance->PSTATE)) & MMCHS_PSTATE_DATI) == MMCHS_PSTATE_DATI)
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
