/*
 * file_system.h
 *
 *  Created on: 10.05.2013
 *      Author: lumannnn
 */

#ifndef OMAP3530_FS_H_
#define OMAP3530_FS_H_

#define MMCHS1_BASE 	((unsigned int*) 0x4809C000)	// instance one
#define MMCHS2_BASE 	((unsigned int*) 0x480B4000)	// instance two
#define MMCHS3_BASE 	((unsigned int*) 0x480AD000)	// instance three

#define MMCHS_DATA_OFSET		0x120
#define MMCHS_BLK_OFFSET 		0x104
#define MMCHS_PSTATE_OFFSET		0x124
#define MMCHS_STAT_OFFSET		0x130

/*
	Buffer read enable. R 0
	This bit is used for non-DMA read transfers. It indicates that a complete
	block specified by MMCi.MMCHS_BLK[10:0] BLEN bits has been written in
	the buffer and is ready to be read.
	It is set to 0 when the entire block is read from the buffer. It is set to 1
	when a block data is ready in the buffer and generates the Buffer read
	ready status of interrupt (MMCi.MMCHS_STAT[5] BRR bit).
 */
#define MMCHS_PSTATE_BRE		BIT11		// read only
/*
	Buffer Write enable. R 0
	This status is used for non-DMA write transfers. It indicates if space is
	available for write data.
 */
#define MMCHS_PSTATE_BWE		BIT10		// read only

/*
	Read transfer active. R 0
	This status is used for detecting completion of a read transfer. It is set to 1
	after the end bit of read command or by activating a continue request
	(MMCi.MMCHS_HCTL[17] CR bit) following a stop at block gap request.
	This bit is set to 0 when all data have been read by the local host after last
	block or after a stop at block gap request.
 */
#define MMCHS_PSTATE_RTA		BIT9		// read only

/*
	Write transfer active. R 0
	This status indicates a write transfer active. It is set to 1 after the end bit of
	write command or by activating a continue request
	(MMCi.MMCHS_HCTL[17] CR bit) following a stop at block gap request.
	This bit is set to 0 when CRC status has been received after last block or
	after a stop at block gap request.
 */
#define MMCHS_PSTATE_WTA		BIT8		// read only

/*
	Bad access to data space. RW 0
	This bit is set automatically to indicate a bad access to buffer when not
	allowed:
	- During a read access to the data register (MMCi.MMCHS_DATA) while
	buffer reads are not allowed (MMCi.MMCHS_PSTATE[11] BRE bit =0)
	- During a write access to the data register (MMCi.MMCHS_DATA) while
	buffer writes are not allowed (MMCi.MMCHS_PSTATE[10] BWE bit=0)
 */
#define MMCHS_STAT_BADA			BIT29

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
#define MMCHS_STAT_CERR			BIT28


#define MMCHS_STAT_READ_ERROR			0x0		// Read 0x0: No error
#define MMCHS_STAT_READ_NO_INTERRUPT	0x0		// Read 0x0: No interrupt
#define MMCHS_STAT_READ_CARD_ERROR		0x1		// Read 0x1: Card error
#define MMCHS_STAT_READ_BAD_ACCESS		0x1		// Read 0x1: Bad Access
#define MMCHS_STAT_WRITE_UNCHANGED		0x0		// Write 0x0: Status bit unchanged
#define MMCHS_STAT_WRITE_CLEARED		0x0		// Write 0x1: Status is cleared

// instances of the data buffer (see OMAP35x.pdf, page 3207)
typedef struct _MMCHS_DATA {
	unsigned int* MMCHS_instance;	// = MMCHSx_BASE + MMCHS_DATA_OFFSET;
} MMCHS_DATA;


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
} MMCHS_BLK;



/*
 *  Instance MMCHS1
 MMCHS3
 MMCHS2
 */

/**
 * Detect attached card and select mode.
 * See OMAP35x.pdf, page 3153
 */
static int fs_detect_mode();

#endif /* FS_H_ */
