/*
 * file_system.h
 *
 *  Created on: 10.05.2013
 *      Author: lumannnn
 */

#ifndef OMAP3530_FS_H_
#define OMAP3530_FS_H_

#include "../../../bit.h"

#define MMCHS1_BASE 	((unsigned int*) 0x4809C000)	// instance one
#define MMCHS2_BASE 	((unsigned int*) 0x480B4000)	// instance two
#define MMCHS3_BASE 	((unsigned int*) 0x480AD000)	// instance three

#define MMCHS_SYSCONFIG_OFFSET	0x010
#define MMCHS_SYSSTATUS_OFFSET	0x014
#define MMCHS_CON_OFFSET		0x02C
#define MMCHS_BLK_OFFSET 		0x104
#define MMCHS_ARG_OFFSET		0x108
#define MMCHS_CMD_OFFSET		0x10C
#define MMCHS_RSP10_OFFSET		0x110
#define MMCHS_DATA_OFSET		0x120
#define MMCHS_PSTATE_OFFSET		0x124
#define MMCHS_HCTL_OFFSET		0x128
#define MMCHS_SYSCTL_OFFSET		0x12C
#define MMCHS_STAT_OFFSET		0x130
#define MMCHS_IE_OFFSET			0x134
#define MMCHS_ISE_OFFSET		0x138
#define MMCHS_CAPA_OFFSET		0x140
#define MMCHS_CUR_CAPA_OFFSET	0x148

/*
	Buffer read enable. R 0
	This bit is used for non-DMA read transfers. It indicates that a complete
	block specified by MMCi.MMCHS_BLK[10:0] BLEN bits has been written in
	the buffer and is ready to be read.
	It is set to 0 when the entire block is read from the buffer. It is set to 1
	when a block data is ready in the buffer and generates the Buffer read
	ready status of interrupt (MMCi.MMCHS_STAT[5] BRR bit).
 */
#define MMCHS_PSTATE_BRE				BIT11		// read only
/*
	Buffer Write enable. R 0
	This status is used for non-DMA write transfers. It indicates if space is
	available for write data.
 */
#define MMCHS_PSTATE_BWE				BIT10		// read only

/*
	Read transfer active. R 0
	This status is used for detecting completion of a read transfer. It is set to 1
	after the end bit of read command or by activating a continue request
	(MMCi.MMCHS_HCTL[17] CR bit) following a stop at block gap request.
	This bit is set to 0 when all data have been read by the local host after last
	block or after a stop at block gap request.
 */
#define MMCHS_PSTATE_RTA				BIT9		// read only

/*
	Write transfer active. R 0
	This status indicates a write transfer active. It is set to 1 after the end bit of
	write command or by activating a continue request
	(MMCi.MMCHS_HCTL[17] CR bit) following a stop at block gap request.
	This bit is set to 0 when CRC status has been received after last block or
	after a stop at block gap request.
 */
#define MMCHS_PSTATE_WTA				BIT8		// read only

/*
	Bad access to data space. RW 0
	This bit is set automatically to indicate a bad access to buffer when not
	allowed:
	- During a read access to the data register (MMCi.MMCHS_DATA) while
	buffer reads are not allowed (MMCi.MMCHS_PSTATE[11] BRE bit =0)
	- During a write access to the data register (MMCi.MMCHS_DATA) while
	buffer writes are not allowed (MMCi.MMCHS_PSTATE[10] BWE bit=0)
 */
#define MMCHS_STAT_BADA					BIT29

/*
	Card error. RW 0
	This bit is set automatically when there is at least one error in a response of
	type R1, R1b, R6, R5 or R5b. Only bits referenced as type E (error) in status
	field in the response can set a card status error. An error bit in the response
	is flagged only if corresponding bit in card status response error
	MMCi.MMCHS_CSRE in set.
	There is no card error detection for autoCMD12 command. The host driver
	shall read MMCi.MMCHS_RSP76 register to detect error bits in the
	command response.
 */
#define MMCHS_STAT_CERR					BIT28

#define MMCHS_STAT_CC					BIT0
#define MMCHS_STAT_CTO					BIT16

#define MMCHS_STAT_READ_ERROR			0x0		// Read 0x0: No error
#define MMCHS_STAT_READ_NO_INTERRUPT	0x0		// Read 0x0: No interrupt
#define MMCHS_STAT_READ_CARD_ERROR		0x1		// Read 0x1: Card error
#define MMCHS_STAT_READ_BAD_ACCESS		0x1		// Read 0x1: Bad Access
#define MMCHS_STAT_WRITE_UNCHANGED		0x0		// Write 0x0: Status bit unchanged
#define MMCHS_STAT_WRITE_CLEARED		0x0		// Write 0x1: Status is cleared

// must be configured before a transfer to indicate the direction of the transfer
#define MMCHS_CMD_DDIR					BIT4	// 0x0 - Data write (host to card), 0x1 - Data read (card to host)


/*
	Card open drain mode (Only for MMC cards). RW 0
	This bit must be set to 1 for MMC card commands 1, 2, 3 and 40,
	and if the MMC card bus is operating in open-drain mode during
	the response phase to the command sent. Typically, during card
	identification mode when the card is either in idle, ready or ident
	state.
	It is also necessary to set this bit to 1, for a broadcast host
	response (see Broadcast host response register
	MMCi.MMCHS_CON[2] HR bit)

	0x0: No Open Drain
	0x1: Open Drain or Broadcast host response
 */
#define MMCHS_CON_OD					BIT0

/*
 	Send initialization stream (All cards). RW 0
	When this bit is set to 1, and the card is idle, an initialization
	sequence is sent to the card.
	An initialization sequence consists of setting the mmci_cmd line to
	1 during 80 clock cycles. The initialization sequence is mandatory -
	but it is not required to do it through this bit - this bit makes it
	easier. Clock divider (MMCi.MMCHS_SYSCTL[15:6] CLKD bits)
	should be set to ensure that 80 clock periods are greater than
	1ms.
	Note: in this mode, there is no command sent to the card and no
	response is expected. A command complete interrupt will be
	generated once the initialization sequence is completed.
	MMCi.MMCHS_STAT[0] CC bit can be polled.

	0x0: The host does not send an initialization sequence.
	0x1: The host sends an initialization sequence.
 */
#define MMCHS_CON_INT					BIT1

/*
	8-bit mode MMC select RW 0
	For SD/SDIO cards, this bit must be set to 0.
	For MMC card, this bit must be set following a valid SWITCH
	command (CMD6) with the correct value and extend CSD index
	written in the argument. Prior to this command, the MMC card
	configuration register (CSD and EXT_CSD) must be verified for
	compliancy with MMC standard specification.

	0x0: 1-bit or 4-bit Data width (mmci_dat[0] or mmci_dat[3:0]
		 used, MMC, SD cards)
	0x1: 8-bit Data width (mmci_dat[7:0] used, MMC cards)
 */
#define MMCHS_CON_DW8					BIT5

/*
	CE-ATA control mode (MMC cards compliant with CE-ATA): RW 0
	By default, this bit is set to 0. It is use to indicate that next
	commands are considered as specific CE-ATA commands that
	potentially use 'command completion' features.

	0x0: Standard MMC/SD/SDIO mode.
	0x1: CE-ATA mode.
	Next commands are considered as CE-ATA
	commands.
 */
#define MMCHS_CON_CEATA					BIT12

/*
	SD bus voltage select (All cards). RW 0x0
	The host driver should set these bits to select the voltage level for the card
	according to the voltage supported by the system
	(MMCi.MMCHS_CAPA[26] VS18 bit, MMCi.MMCHS_CAPA[25] VS30 bit,
	MMCi.MMCHS_CAPA[24] VS33 bit) before starting a transfer.

	0x5: 1.8V (Typical)
	0x6: 3.0V (Typical)
	0x7: 3.3V (Typical)
	MMCHS2: This field must be set to 0x5.
	MMCHS3: This field must be set to 0x5.

	11:9
 */
#define MMCHS_HCTL_SVDS_RESET			BIT11 | BIT10 | BIT9
#define MMCHS_HCTL_SDVS_1V8				BIT11 | BIT9
#define MMCHS_HCTL_SDVS_3V0				BIT11 | BIT10
#define MMCHS_HCTL_SDVS_3V3				BIT11 | BIT10 | BIT9

/*
	SD bus power. RW 0
	Before setting this bit, the host driver shall select the SD bus voltage
	(MMCi.MMCHS_HCTL[11:9] SDVS bits). If the host controller detects the
	No card state, this bit is automatically set to 0. If the module is power off, a
	write in the command register (MMCi.MMCHS_CMD) will not start the
	transfer. A write to this bit has no effect if the selected SD bus voltage is
	not supported according to capability register
	(MMCi.MMCHS_CAPA[VS*]).

	0x0: Power off
	0x1: Power on
 */
#define MMCHS_HCTL_SDBP					BIT8

/*
	Data transfer width. RW 0
	For MMC card, this bit must be set following a valid SWITCH command
	(CMD6) with the correct value and extend CSD index written in the
	argument. Prior to this command, the MMC card configuration register
	(CSD and EXT_CSD) must be verified for compliance with MMC standard
	specification 4.x
	This register has no effect when the MMC 8-bit mode is selected
	(MMCi.MMCHS_CON[5] DW8 bit set to 1)
	For SD/SDIO cards, this bit must be set following a valid
	SET_BUS_WIDTH command (ACMD6) with the value written in bit 1 of
	the argument. Prior to this command, the SD card configuration register
	(SCR) must be verified for the supported bus width by the SD card.

	0x0: 1-bit Data width (mmci_dat[0] used)
	0x1: 4-bit Data width (mmci_dat[3:0] used)
 */
#define MMCHS_HCTL_DTW					BIT1


#define MMCHS_CAPA_VS18					BIT26
#define MMCHS_CAPA_VS30					BIT25
#define MMCHS_CAPA_VS33					BIT24

//#define MMCHS_CUR_CAPA_CUR_1V8			// 23:16
//#define MMCHS_CUR_CAPA_CUR_3V0			// 15:8
//#define MMCHS_CUR_CAPA_CUR_3V3			// 7:0

/*
	Internal clock enable. This register controls the internal clock activity. RW 0
	In very low power state, the internal clock is stopped. Note: The activity of
	the debounce clock (used for wake-up events) and the interface clock
	(used for reads and writes to the module register map) are not affected by
	this register.

	0x0: The internal clock is stopped (very low power state).
	0x1: The internal clock oscillates and can be automatically gated
	when MMCi.MMCHS_SYSCONFIG[0] AUTOIDLE bit is set to 1
	(default value) .
 */
#define MMCHS_SYSCTL_ICE				BIT0
#define MMCHS_SYSCTL_ICS				BIT1
#define MMCHS_SYSCTL_SRC				BIT25

/*
	Clock frequency select These bits define the ratio between a reference RW 0x000
	clock frequency (system dependant) and the output clock frequency on the
	mmci_clk pin of either the memory card (MMC, SD or SDIO).

	0x0: Clock Ref bypass
	0x1: Clock Ref bypass
	0x2: Clock Ref / 2
	0x3: Clock Ref / 3
	0x3FF: Clock Ref / 1023
 */
#define MMCHS_SYSCTL_CLKD_RESET			(1023 << 6)
#define MMCHS_SYSCTL_CLKD_BYPASS0		(0 << 6)
#define MMCHS_SYSCTL_CLKD_BYPASS1		(1 << 6)
#define MMCHS_SYSCTL_CLKD_REF_DIV_2		(2 << 6)
#define MMCHS_SYSCTL_CLKD_REF_DIV_3		(1023 << 6)

#define MMCHS_SYSCONFIG_SOFTRESET		BIT1
#define MMCHS_SYSCONFIG_ENAWAKEUP		BIT2

/*
	Clocks activity during wake up mode period. RW 0x0

	Bit8: Interface clock
	Bit9: Functional clock
	0x0: Interface and Functional clock may be switched off.
	0x1: Interface clock is maintained. Functional clock may be switched-off.
	0x2: Functional clock is maintained. Interface clock may be switched-off.
	0x3: Interface and Functional clocks are maintained.

	9:8
 */
#define MMCHS_SYSCOFNIG_CLOCKACTIVITY_MAINTAIN	BIT8 | BIT9

/*
	SIDLEMODE Power management RW 0x2

	0x0: If an idle request is detected, the MMC/SD/SDIO host controller
	acknowledges it unconditionally and goes in Inactive mode.
	Interrupt and DMA requests are unconditionally deasserted.
	0x1: If an idle request is detected, the request is ignored and the module
	keeps on behaving normally.
	0x2: If an idle request is detected, the module will switch to wake up
	mode based on its internal activity, and the wake up capability can
	be used if the wake up capability is enabled (bit
	MMCi.MMCHS_SYSCONFIG[2] ENAWAKEUP bit is set to 1).
	0x3: Reserved - do not use

	4:3
 */
#define MMCHS_SYSCONFIG_SIDLEMODE_RESET					BIT3 | BIT4
#define MMCHS_SYSCONFIG_SIDLEMODE_GO_INACTIVE			BIT3 | BIT4
#define MMCHS_SYSCONFIG_SIDLEMODE_IGNORE_IDLE_REQUEST	BIT3
#define MMCHS_SYSCONFIG_SIDLEMODE_SWITCH_TO_WAKE_UP		BIT3 | BIT4

/*
	Internal Clock gating strategy RW 1

	0x0: Clocks are free-running
	0x1: Automatic clock gating strategy is applied, based on the
	interconnect and MMC interface activity
 */
#define MMCHS_SYSCOFNIG_AUTOIDLE		BIT0

#define MMCHS_SYSSTATUS_RESETDONE		BIT0

#define MMCHS_HCTL_IWE					BIT24

#define MMCHS_IE_CIRQ_ENABLE			BIT8

// instances of the data buffer (see OMAP35x.pdf, page 3207)
typedef struct _MMCHS_DATA {
	unsigned int* MMCHS_instance;	// = MMCHSx_BASE + MMCHS_DATA_OFFSET;
} MMCHS_DATA_t;


#define MMCHS_BLEN_TRANSFER_NO			0x0
#define MMCHS_BLEN_TRANSFER_1_BYTE		0x1
#define MMCHS_BLEN_TRANSFER_2_BYTES		0x2
#define MMCHS_BLEN_TRANSFER_3_BYTES		0x3
#define MMCHS_BLEN_TRANSFER_511_BYTES	0x1FF
#define MMCHS_BLEN_TRANSFER_512_BYTES	0x200
#define MMCHS_BLEN_TRANSFER_1023_BYTES	0x3FF
#define MMCHS_BLEN_TRANSFER_1024_BYTES	0x400

// (see OMAP35x.pdf, page 3200)
typedef struct _MMCHS_BLK {
	unsigned int* MMCHS_instance;	// = MMCHSx_BASE + MMCHS_BLK_OFFSET;
	int data_transfer;				// = MMCHS_BLEN_TRANSFER_x
} MMCHS_BLK_t;



/**
 * Detect attached card and select mode.
 * See OMAP35x.pdf, page 3153
 */
static int fs_device_detect_mode();

/**
 * TODO: really a separate function?
 *
 * Caution: Read access to the command response registers is allowed only when the command process is completed.
 * page 3153
 */
static int fs_device_read_command_response_register();

static void fs_set_direction_read(unsigned int* MMCHS_instance);
static void fs_set_direction_write(unsigned int* MMCHS_instance);

#endif /* FS_H_ */
