/*
 * fs.c
 *
 *  Created on: 10.05.2013
 *      Author: lumannnn
 */

#include "fs.h"
#include "../../generic/fs/fs.h"

// private API

static void fs_device_enable_interface_and_functional_clock(FileHandle_t* handle);

static void fs_device_soft_reset(FileHandle_t* handle);

static void fs_device_set_default_capabilities(FileHandle_t* handle);
static void fs_device_wake_up_config(FileHandle_t* handle);
static void fs_device_mmc_host_and_bus_config(FileHandle_t* handle);

static void fs_device_identify_card(FileHandle_t* handle);

// helper functions

static int fs_device_check_mmchs_stat_cc(FileHandle_t* handle);
static int fs_device_check_mmchs_stat_cto(FileHandle_t* handle);
static void fs_device_set_mmchs_sysctl_src_and_wait_until_reset(FileHandle_t* handle);

static void fs_device_finish_card_identification(FileHandle_t* handle);

static void fs_device_send_cmd0();
static void fs_device_send_cmd1();
static void fs_device_send_cmd2();
static void fs_device_send_cmd3();
static void fs_device_send_cmd5();
static void fs_device_send_cmd8();
static void fs_device_send_cmd55();
static void fs_device_send_cmd9();
static void fs_device_send_cmd7();
static void fs_device_set_bus_width_with_cmd6();
static void fs_device_enable_high_speed_feature_with_cmd6();

// * *********************** *
// * 	API
// * *********************** *

int fs_init(FileHandle_t* handle) {
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
	fs_device_enable_interface_and_functional_clock(handle);
	fs_device_soft_reset(handle);
	fs_device_set_default_capabilities(handle);
	fs_device_wake_up_config(handle);
	fs_device_mmc_host_and_bus_config(handle);

	fs_device_identify_card(handle);

	return 1;
}

int fs_read(FileHandle_t* handle) {
	fs_set_direction_read(handle->instance);

	return 1;
}

int fs_write(FileHandle_t* handle) {
	fs_set_direction_write(handle->instance);

	return 1;
}

// private API

static void fs_device_enable_interface_and_functional_clock(FileHandle_t* handle) {
	// Prior to any MMCHS register access one must enable MMCHS interface clock and functional clock in
	// PRCM module registers PRCM.CM_ICLKEN1_CORE and PRCM.CM_FCLKEN1_CORE. Please refer to
	// Chapter 4, Power, Reset, and Clock Management.
}

static void fs_device_soft_reset(FileHandle_t* handle) {
	// see page 3161
	// set the MMCi.MMCHS_SYSCONFIG[1] SOFTRESET bit to 0x1
	*(handle->instance + MMCHS_SYSCONFIG_OFFSET) |= MMCHS_SYSCONFIG_SOFTRESET;
	// Read the MMCi.MMCHS_SYSSTATUS[0] RESETDONE bit
	// RESETDONE = 0x1? --> no? -> wait.
	unsigned int* sysstatus = handle->instance + MMCHS_SYSSTATUS_OFFSET;
	unsigned int* sysstatus_resetdone = sysstatus;
	*(sysstatus_resetdone) |= MMCHS_SYSSTATUS_RESETDONE;
	while (*(sysstatus) != *(sysstatus_resetdone));
}

static void fs_device_set_default_capabilities(FileHandle_t* handle) {
	/*
	 * Software must read capabilities (in boot ROM for instance) and is allowed to set (write)
	 * MMCi.MMCHS_CAPA[26:24] and MMCi.MMCHS_CUR_CAPA[23:0] registers before the MMC/SD/SDIO
	 * host driver is started.
	 */

}

static void fs_device_wake_up_config(FileHandle_t* handle) {
	// page 3162
	// set the MMCi.MMCHS_SYSCONFIG[2] ENAWAKEUP bit to 0x1 if required
	*(handle->instance + MMCHS_SYSCONFIG_OFFSET) |= MMCHS_SYSCONFIG_ENAWAKEUP;
	// set the MMCi.MMCHS_HCTL[24] IWE bit to 0x1 to enable the wake-up event on SD card interrupt
	*(handle->instance + MMCHS_HCTL_OFFSET) |= MMCHS_HCTL_IWE;
	// set the MMCi.MMCHS[8] CIRQ_ENABLE bit enable the card interrupt (for SDIO card only)
	*(handle->instance + MMCHS_IE_OFFSET) |= MMCHS_IE_CIRQ_ENABLE;
}

static void fs_device_mmc_host_and_bus_config(FileHandle_t* handle) {
	// page 3163
	// write MMCi.MMCHS_CON register (OD, DW8, CEATA) to configure specific data and command transfer
	*(handle->instance + MMCHS_CON_OFFSET) |= MMCHS_CON_OD
			& ~MMCHS_CON_DW8
			& ~MMCHS_CON_CEATA;

	// write MMCi.MMCHS_HCTL register (SDVS, SDBP, DTW) to configure the card voltage value and power mode and dat bus width
	*(handle->instance + MMCHS_HCTL_OFFSET) &= ~MMCHS_HCTL_SVDS_RESET;

	if (handle->instance != MMCHS1_BASE) {
		/*
		 * MMCHS2: This field must be set to 0x5.
		 * MMCHS3: This field must be set to 0x5.
		 */
		*(handle->instance + MMCHS_HCTL_OFFSET) |= MMCHS_HCTL_SDVS_1V8;
	} else {
		// TODO what about instance 1?
		*(handle->instance + MMCHS_HCTL_OFFSET) |= MMCHS_HCTL_SDVS_3V3;
	}

	*(handle->instance + MMCHS_HCTL_OFFSET) |= MMCHS_HCTL_SDBP | MMCHS_HCTL_DTW;
	// TODO
	// SET_BUS_WIDTH command (ACMD6) with a value written in bit 1 of the argument.
	// Prior to this command, the SD card configuration register (SCR) must be verified for the supported bus width by the SD card.


	// Read back the MMCi.MMCHS_HCTL[8] SDBP bit
	// SDBP = 0x1? --> no? -> wait.
	unsigned int* hctl = handle->instance + MMCHS_HCTL_OFFSET;
	unsigned int* hctl_sdbp = hctl;
	*(hctl_sdbp) |= MMCHS_HCTL_SDBP;
	while (*(hctl) != *(hctl_sdbp));

	// set the MMCi.MMCHS_SYSCTL[0] ICE bit to 0x1 to enable the internal clock
	*(handle->instance + MMCHS_SYSCTL_OFFSET) |= MMCHS_SYSCTL_ICE;

	// XXX: WHAT???? what you want from me? leave me alone with your horseshit! TODO -.-
	// set the CONTROL.CONTROL_PADCONF_<module>_<signal>[x] INPUTENABLE bit to 0x1
	// `-> Reason: Enable the input buffer of the clock output. As a result, there is a loopback through the output and input buffers. The goal is to support the synchronization of the mmci_clk.

	// configure the MMCi.MMCHS_SYSCTL[15:6] CLKD bit field
	// `-> Reason: For initialization sequence, you should have 80 clock cycles in 1ms. It means clock frequency should be <= 80kHz.
	*(handle->instance + MMCHS_SYSCTL_OFFSET) &= ~MMCHS_SYSCTL_CLKD_RESET;
	*(handle->instance + MMCHS_SYSCTL_OFFSET) |= MMCHS_SYSCTL_CLKD_BYPASS0;

	// read the MMCi.MMCHS_SYSCTL[1] ICS bit
	// ICS = 0x1? --> no? -> wait.
	// --> yes? -> ok. clock is stable
	unsigned int* sysctl = handle->instance + MMCHS_SYSCTL_OFFSET;
	unsigned int* sysctl_ics = sysctl;
	*(sysctl_ics) |= MMCHS_SYSCTL_ICS;
	while (*(sysctl) != *(sysctl_ics));

	// write the MMCi.MMCHS_SYSCONFIG CLOCKACTIVITY, SIDLEMODE and AUTOIDLE field to configure the behavior of the module in idle mode.
	*(handle->instance + MMCHS_SYSCONFIG_OFFSET) |= MMCHS_SYSCOFNIG_AUTOIDLE | MMCHS_SYSCOFNIG_CLOCKACTIVITY_MAINTAIN;
	*(handle->instance + MMCHS_SYSCONFIG_OFFSET) &= ~MMCHS_SYSCONFIG_SIDLEMODE_RESET;
	*(handle->instance + MMCHS_SYSCONFIG_OFFSET) |= MMCHS_SYSCONFIG_SIDLEMODE_IGNORE_IDLE_REQUEST;
}

// see page 3164:
static void fs_device_identify_card(FileHandle_t* handle) {
	// (Start)

	// TODO? should already be handled in the init
	// do module initialization before this

	unsigned int* base = handle->instance;

	// set MMCi.MMCHS_CON[1] INT bit to 0x1 to send an initialization stream
	*(base + MMCHS_CON_OFFSET) |= MMCHS_CON_INT;

	// write 0x00000000 in the MMCi.MMCHS_CMD register
	*(base + MMCHS_CMD_OFFSET) = 0x00000000;

	// (wait 1 ms)
	// FIXME
	volatile int i = 0;
	while (++i < 200000);

	// set MMCi.MMCHS_STAT[0] CC bit to 0x1 to clear the flag
	*(base + MMCHS_STAT_OFFSET) |= MMCHS_STAT_CC;

	// set MMCi.MMCHS_CON[1] INT bit to 0x0 to end the initialization sequence
	*(base + MMCHS_CON_OFFSET) &= ~MMCHS_CON_INT;

	// clear MMCHS_STAT register (write 0xFFFF FFFF)
	*(base + MMCHS_STAT_OFFSET) = 0xFFFFFFFF;

	// TODO
	// change clock frequency to fit protocol

	fs_device_send_cmd0();
	// (A)
	// send a CMD5 command
	fs_device_send_cmd5();

	// read the MMCi.MMCHS_STAT register
	while (1) {
		// CC = 0x1
		if (fs_device_check_mmchs_stat_cc(handle) == 1) {
			// it is an SDIO card
			handle->card_type = SDIO;

			// TODO
			// See the SDIO Standard Specification to identify the card type:
			// Memory only, I/O only, Combo

			// goto (End)
			return;
		}
		// CTO = 0x1
		if (fs_device_check_mmchs_stat_cto(handle) == 1) {
			break;
		}
	}

	fs_device_set_mmchs_sysctl_src_and_wait_until_reset(handle);
	fs_device_send_cmd8();

	// read the MMCi.MMCHS_STAT register
	while (1) {
		// CC = 0x1
		if (fs_device_check_mmchs_stat_cc(handle) == 1) {
			// yes? (it is an SD card compliant with standard 2.0 or later)
			handle->card_type = SD;

			// TODO
			// see the SD Standard Specification version 2.0 or later
			// to identify the card type: High Capacity; Standard Capacity

			// goto (End)
			return;
		}
		// CTO = 0x1
		if (fs_device_check_mmchs_stat_cto(handle) == 1) {
			break;
		}
	}

	fs_device_set_mmchs_sysctl_src_and_wait_until_reset(handle);

	int abort = 0;
	while (abort == 0) {
		// see page 3165.
		// marker:
		// ---
		// |A|
		//  V

		// TODO:
		fs_device_send_cmd55();
		// send an ACMD41 command

		// read the MMCi.MMCHS_STAT register
		while (1) {
			// CC = 0x1
			if (fs_device_check_mmchs_stat_cc(handle) == 1) {
				// (it is a SD card compliant with standard 1.x)
				handle->card_type = SD;

				// verify the card is busy:
				// read the MMCi.MMCHS_RSP10[31] bit
				// equal to 0x1?
				unsigned int* mmchs_rsp10 = base + MMCHS_RSP10_OFFSET;
				if (*(mmchs_rsp10) == (*(mmchs_rsp10)) | BIT31) {
					// --> yes? -> the card is not busy
					// 			goto (B)
					fs_device_finish_card_identification(handle);
					return;
				} else {
					// --> no? -> the card is busy
					//			goto ---
					//				 |A|
					//				  V
					break;
				}
			}
			// CTO = 0x1
			if (fs_device_check_mmchs_stat_cto(handle) == 1) {
				// it is a MMC card
				abort = 1;
				break;
			}
		}
	}

	// (it is a MMC card)
	handle->card_type = MMC;
	fs_device_set_mmchs_sysctl_src_and_wait_until_reset(handle);

	while (1) {
		// ----------
		// (marker_1)
		// ----------

		// TODO
		// send a CMD1 command* (*With OCR 0. In case of a CMD1 with OCR=0, a second CMD1 must be sent to the card with the "negociated" voltage.
		fs_device_send_cmd1();

		// read the MMCi.MMCHS_STAT register
		while (1) {
			// CTO = 0x1
			if (fs_device_check_mmchs_stat_cto(handle) == 1) {
				// unknown type of card
				// goto (end)
				return;
			}

			// CC = 0x1
			if (fs_device_check_mmchs_stat_cc(handle) == 1) {
				break;
			}
		}

		// it is a MMC card
		handle->card_type = MMC;

		// Verify the card is busy:
		// read the MMCi.MMCHS_RSP19[31] bit --> typo in OMPA35x.pdf: should be MMCHS_RSP10
		// equal to 0x1?
		unsigned int* mmchs_rsp10 = base + MMCHS_RSP10_OFFSET;
		if (*(mmchs_rsp10) == (*(mmchs_rsp10)) | BIT31) {
			// --> yes? -> the card is not busy
			// 			goto (B)
			fs_device_finish_card_identification(handle);
			return;
		} else {
			// --> no? (the card is busy)
			// 			-> goto (marker_1)
		}
	}

	// (end)
}

/**
 * (B)
 */
static void fs_device_finish_card_identification(FileHandle_t* handle) {
	// (B)

	// TODO
	// send a CMD2 command to get information on how to access the card content
	fs_device_send_cmd2();
	fs_device_send_cmd3();

	// card type?
	if (handle->card_type == MMC) {
		// --> MMC card? -> Is there more than one MMC connected to the same bus, and are they all identified?
		//			--> yes -> goto (B)
		//			--> no -> (continue)

		// nothing to do here...
		// we do not support MMC cards yet.
	}

	// (continue)

	fs_device_send_cmd7();
}

static int fs_detect_mode() {
	return -1;
}

static void fs_set_direction_read(unsigned int* MMCHS_instance) {
	*(MMCHS_instance + MMCHS_CMD_OFFSET) |= MMCHS_CMD_DDIR;
}

static void fs_set_direction_write(unsigned int* MMCHS_instance) {
	*(MMCHS_instance + MMCHS_CMD_OFFSET) &= ~MMCHS_CMD_DDIR;
}


// helper functions

static int fs_device_check_mmchs_stat_cc(FileHandle_t* handle) {
	if (*(handle->instance + MMCHS_STAT_OFFSET) == (*(handle->instance + MMCHS_STAT_OFFSET) | MMCHS_STAT_CC)) {
		return 1;
	}

	return 0;
}

static int fs_device_check_mmchs_stat_cto(FileHandle_t* handle) {
	if (*(handle->instance + MMCHS_STAT_OFFSET) == (*(handle->instance + MMCHS_STAT_OFFSET) | MMCHS_STAT_CTO)) {
		return 1;
	}

	return 0;
}

static void fs_device_set_mmchs_sysctl_src_and_wait_until_reset(FileHandle_t* handle) {
	// TODO: wooow... wtf.. check this. it was quite late when i wrote this!
	unsigned int* sysctl = handle->instance + MMCHS_SYSCTL_OFFSET;
	unsigned int* sysctl_unset = sysctl;
	*(sysctl_unset) &= ~MMCHS_SYSCTL_SRC;

	// set MMCi.MMCHS_SYSCTL[25] SRC bit to 0x1
	*(sysctl) |= MMCHS_SYSCTL_SRC;
	// and wait until it returns to 0x0
	while (*(sysctl) != *(sysctl_unset));
}

static void fs_device_send_cmd0() {
	*(MMCHS1_BASE + MMCHS_CON_OFFSET) |= 0x00000001;
	*(MMCHS1_BASE + MMCHS_IE_OFFSET) |= 0x00040001;
	*(MMCHS1_BASE + MMCHS_ISE_OFFSET) |= 0x00040001;
	*(MMCHS1_BASE + MMCHS_CMD_OFFSET) |= 0x00000000;
}
static void fs_device_send_cmd1() {
	*(MMCHS1_BASE + MMCHS_CON_OFFSET) |= 0x00000001;
	*(MMCHS1_BASE + MMCHS_IE_OFFSET) |= 0x00050001;
	*(MMCHS1_BASE + MMCHS_ISE_OFFSET) |= 0x00050001;
	*(MMCHS1_BASE + MMCHS_CMD_OFFSET) |= 0x01020000;
}
static void fs_device_send_cmd2() {
	*(MMCHS1_BASE + MMCHS_CON_OFFSET) |= 0x00000001;
	*(MMCHS1_BASE + MMCHS_IE_OFFSET) |= 0x00070001;
	*(MMCHS1_BASE + MMCHS_ISE_OFFSET) |= 0x00070001;
	*(MMCHS1_BASE + MMCHS_CMD_OFFSET) |= 0x02090000;
}
static void fs_device_send_cmd3() {
	*(MMCHS1_BASE + MMCHS_CON_OFFSET) |= 0x00000001;
	*(MMCHS1_BASE + MMCHS_IE_OFFSET) |= 0x100f0001;
	*(MMCHS1_BASE + MMCHS_ISE_OFFSET) |= 0x100f0001;
	*(MMCHS1_BASE + MMCHS_CMD_OFFSET) |= 0x031a0000;
	*(MMCHS1_BASE + MMCHS_ARG_OFFSET) |= 0x00010000;
}
static void fs_device_send_cmd5() {
	*(MMCHS1_BASE + MMCHS_CON_OFFSET) |= 0x00000001;
	*(MMCHS1_BASE + MMCHS_IE_OFFSET) |= 0x00050001;
	*(MMCHS1_BASE + MMCHS_ISE_OFFSET) |= 0x00050001;
	*(MMCHS1_BASE + MMCHS_CMD_OFFSET) |= 0x05020000;
}
static void fs_device_send_cmd8() {
	*(MMCHS1_BASE + MMCHS_CON_OFFSET) |= 0x00000001;
	*(MMCHS1_BASE + MMCHS_IE_OFFSET) |= 0x100f0001;
	*(MMCHS1_BASE + MMCHS_ISE_OFFSET) |= 0x100f0001;
	*(MMCHS1_BASE + MMCHS_CMD_OFFSET) |= 0x81a0000;
}
static void fs_device_send_cmd55() {
	*(MMCHS1_BASE + MMCHS_CON_OFFSET) |= 0x00000001;
	*(MMCHS1_BASE + MMCHS_IE_OFFSET) |= 0x100f0001;
	*(MMCHS1_BASE + MMCHS_ISE_OFFSET) |= 0x100f0001;
	*(MMCHS1_BASE + MMCHS_CMD_OFFSET) |= 0x371a0000;
}
static void fs_device_send_cmd9() {
	*(MMCHS1_BASE + MMCHS_CON_OFFSET) |= 0x00000000;
	*(MMCHS1_BASE + MMCHS_IE_OFFSET) |= 0x00070001;
	*(MMCHS1_BASE + MMCHS_ISE_OFFSET) |= 0x00070001;
	*(MMCHS1_BASE + MMCHS_CMD_OFFSET) |= 0x09090000;
	*(MMCHS1_BASE + MMCHS_ARG_OFFSET) |= 0x00010000;
}
static void fs_device_send_cmd7() {
	*(MMCHS1_BASE + MMCHS_CON_OFFSET) |= 0x00000000;
	*(MMCHS1_BASE + MMCHS_IE_OFFSET) |= 0x100f0001;
	*(MMCHS1_BASE + MMCHS_ISE_OFFSET) |= 0x100f0001;
	*(MMCHS1_BASE + MMCHS_CMD_OFFSET) |= 0x071a0000;
	*(MMCHS1_BASE + MMCHS_ARG_OFFSET) |= 0x00010000;
}
static void fs_device_set_bus_width_with_cmd6() {
	*(MMCHS1_BASE + MMCHS_CON_OFFSET) |= 0x00000000;
	*(MMCHS1_BASE + MMCHS_IE_OFFSET) |= 0x100f0001;
	*(MMCHS1_BASE + MMCHS_ISE_OFFSET) |= 0x100f0001;
	*(MMCHS1_BASE + MMCHS_CMD_OFFSET) |= 0x061b0000;
	*(MMCHS1_BASE + MMCHS_ARG_OFFSET) |= 0x03b70200;
}
static void fs_device_enable_high_speed_feature_with_cmd6() {
	*(MMCHS1_BASE + MMCHS_CON_OFFSET) |= 0x00000020;
	*(MMCHS1_BASE + MMCHS_IE_OFFSET) |= 0x100f0001;
	*(MMCHS1_BASE + MMCHS_ISE_OFFSET) |= 0x100f0001;
	*(MMCHS1_BASE + MMCHS_CMD_OFFSET) |= 0x061b0000;
	*(MMCHS1_BASE + MMCHS_ARG_OFFSET) |= 0x03b90100;
}
