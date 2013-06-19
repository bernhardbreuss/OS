/*
 * mmchs.h
 *
 *  Created on: 10.05.2013
 *      Author: lumannnn
 */

#ifndef OMAP3530_MMCHS_H_
#define OMAP3530_MMCHS_H_

typedef enum {
	UNKNOWN=0,
	MMC,
	SD_1x,
	SD_2,
	SD_2_HC,
	SDIO
} StorageType_t;

typedef enum {
	STREAM=0,
	BLOCK
} ProtocolType_t;

typedef struct _Protocol_t {
	ProtocolType_t type;
	int block_size;
} Protocol_t;

typedef struct {
	unsigned int  reserved0:   	7; // 0
	unsigned int  V170_V195:   	1; // 1.70V - 1.95V
	unsigned int  V200_V260:   	7; // 2.00V - 2.60V
	unsigned int  V270_V360:   	9; // 2.70V - 3.60V
	unsigned int  reserved1:  	5; // Reserved
	unsigned int  access_mode:	2; // 00b (byte mode), 10b (sector mode)
	unsigned int  busy:        	1; // This bit is set to LOW if the card has not finished the power up routine
} MMCHS_OCR_t;

typedef struct {
	unsigned int  	not_used;   // 1 [0:0]
	unsigned int  	CRC;        // CRC7 checksum [7:1]
	unsigned int  	MDT;        // Manufacturing date [19:8]
	unsigned int  	reserved_1; // Reserved [23:20]
	unsigned int  	PSN;        // Product serial number [55:24]
	unsigned int   	PRV;        // Product revision [63:56]
	unsigned int   	PNM[5];     // Product name [64:103]
	unsigned int  	OID;        // OEM/Application ID [119:104]
	unsigned int	MID;        // Manufacturer ID [127:120]
} MMCHS_CID_t;

typedef struct _MMCHS_CSD {
	unsigned int  NOT_USED:           1; // Not used, always 1 [0:0]
	unsigned int  CRC:                7; // CRC [7:1]

	unsigned int  RESERVED_1:         2; // Reserved [9:8]
	unsigned int  FILE_FORMAT:        2; // File format [11:10]
	unsigned int  TMP_WRITE_PROTECT:  1; // Temporary write protection [12:12]
	unsigned int  PERM_WRITE_PROTECT: 1; // Permanent write protection [13:13]
	unsigned int  COPY:               1; // Copy flag (OTP) [14:14]
	unsigned int  FILE_FORMAT_GRP:    1; // File format group [15:15]

	unsigned int  RESERVED_2:         5; // Reserved [20:16]
	unsigned int  WRITE_BL_PARTIAL:   1; // Partial blocks for write allowed [21:21]
	unsigned int  WRITE_BL_LEN:       4; // Max. write data block length [25:22]
	unsigned int  R2W_FACTOR:         3; // Write speed factor [28:26]
	unsigned int  RESERVED_3:         2; // Reserved [30:29]
	unsigned int  WP_GRP_ENABLE:      1; // Write protect group enable [31:31]

	unsigned int  WP_GRP_SIZE:        7; // Write protect group size [38:32]
	unsigned int  SECTOR_SIZE:        7; // Erase sector size [45:39]
	unsigned int  ERASE_BLK_EN:       1; // Erase single block enable [46:46]
	unsigned int  C_SIZE_MULT:        3; // Device size multiplier [49:47]
	unsigned int  VDD_W_CURR_MAX:     3; // Max. write current @ VDD max [52:50]
	unsigned int  VDD_W_CURR_MIN:     3; // Max. write current @ VDD min [55:53]
	unsigned int  VDD_R_CURR_MAX:     3; // Max. read current @ VDD max [58:56]
	unsigned int  VDD_R_CURR_MIN:     3; // Max. read current @ VDD min [61:59]
	unsigned int  C_SIZELow2:         2; // Device size [63:62]

	unsigned int  C_SIZEHigh10:       10;// Device size [73:64]
	unsigned int  RESERVED_4:         2; // Reserved [75:74]
	unsigned int  DSR_IMP:            1; // DSR implemented [76:76]
	unsigned int  READ_BLK_MISALIGN:  1; // Read block misalignment [77:77]
	unsigned int  WRITE_BLK_MISALIGN: 1; // Write block misalignment [78:78]
	unsigned int  READ_BL_PARTIAL:    1; // Partial blocks for read allowed [79:79]
	unsigned int  READ_BL_LEN:        4; // Max. read data block length [83:80]
	unsigned int  CCC:                12;// Card command classes [95:84]

	unsigned char TRAN_SPEED          ;  // Max. bus clock frequency [103:96]
	unsigned char NSAC                ;  // Data read access-time 2 in CLK cycles (NSAC*100) [111:104]
	unsigned char TAAC                ;  // Data read access-time 1 [119:112]

	unsigned int  RESERVED_5:         6; // Reserved [125:120]
	unsigned int  CSD_STRUCTURE:      2; // CSD structure [127:126]
} MMCHS_CSD_t;

typedef struct _MMCHS_CSD_SD2 {
  unsigned int  NOT_USED:           1; // Not used, always 1 [0:0]
  unsigned int  CRC:                7; // CRC [7:1]
  unsigned int  RESERVED_1:         2; // Reserved [9:8]
  unsigned int  FILE_FORMAT:        2; // File format [11:10]
  unsigned int  TMP_WRITE_PROTECT:  1; // Temporary write protection [12:12]
  unsigned int  PERM_WRITE_PROTECT: 1; // Permanent write protection [13:13]
  unsigned int  COPY:               1; // Copy flag (OTP) [14:14]
  unsigned int  FILE_FORMAT_GRP:    1; // File format group [15:15]
  unsigned int  RESERVED_2:         5; // Reserved [20:16]
  unsigned int  WRITE_BL_PARTIAL:   1; // Partial blocks for write allowed [21:21]
  unsigned int  WRITE_BL_LEN:       4; // Max. write data block length [25:22]
  unsigned int  R2W_FACTOR:         3; // Write speed factor [28:26]
  unsigned int  RESERVED_3:         2; // Reserved [30:29]
  unsigned int  WP_GRP_ENABLE:      1; // Write protect group enable [31:31]
  unsigned int  WP_GRP_SIZE:        7; // Write protect group size [38:32]
  unsigned int  SECTOR_SIZE:        7; // Erase sector size [45:39]
  unsigned int  ERASE_BLK_EN:       1; // Erase single block enable [46:46]
  unsigned int  RESERVED_4:         1; // Reserved [47:47]
  unsigned int  C_SIZELow16:        16;// Device size [69:48]
  unsigned int  C_SIZEHigh6:        6; // Device size [69:48]
  unsigned int  RESERVED_5:         6; // Reserved [75:70]
  unsigned int  DSR_IMP:            1; // DSR implemented [76:76]
  unsigned int  READ_BLK_MISALIGN:  1; // Read block misalignment [77:77]
  unsigned int  WRITE_BLK_MISALIGN: 1; // Write block misalignment [78:78]
  unsigned int  READ_BL_PARTIAL:    1; // Partial blocks for read allowed [79:79]
  unsigned int  READ_BL_LEN:        4; // Max. read data block length [83:80]
  unsigned int  CCC:                12;// Card command classes [95:84]
  unsigned char TRAN_SPEED          ;  // Max. bus clock frequency [103:96]
  unsigned char NSAC                ;  // Data read access-time 2 in CLK cycles (NSAC*100) [111:104]
  unsigned char TAAC                ;  // Data read access-time 1 [119:112]
  unsigned int  RESERVED_6:         6; // 0 [125:120]
  unsigned int  CSD_STRUCTURE:      2; // CSD structure [127:126]
} MMCHS_CSD_SD2_t;

typedef struct _MMCHS_BLOCK {
	unsigned int id;
	char removable;				// not implemented
	char logical_partition;
	char read_only;				// not implemented
	char write_caching;			// not implemented
	unsigned int block_size;
	unsigned int io_align;		// alignment for any buffer that read/writes blocks
	unsigned long long lba;		// the logical last block.
} MMCHS_BLOCK_t;

typedef struct _MMCHS_t {
	unsigned int* volatile SYSCONFIG;
	unsigned int* volatile SYSSTATUS;
	unsigned int* volatile CSRE;
	unsigned int* volatile CON;
	unsigned int* volatile BLK;
	unsigned int* volatile ARG;
	unsigned int* volatile CMD;
	unsigned int* volatile RSP10;
	unsigned int* volatile RSP32;
	unsigned int* volatile RSP54;
	unsigned int* volatile RSP76;
	unsigned int* volatile DATA;
	unsigned int* volatile PSTATE;
	unsigned int* volatile HCTL;
	unsigned int* volatile SYSCTL;
	unsigned int* volatile STAT;
	unsigned int* volatile IE;
	unsigned int* volatile ISE;
	unsigned int* volatile AC12;
	unsigned int* volatile CAPA;

	unsigned int RCA;	// relative card address.
	MMCHS_OCR_t OCR;
	MMCHS_CID_t CID;
	MMCHS_CSD_t CSD;
	//unsigned int block_size;
	unsigned int num_blocks;
	unsigned int clk_frq_select;

	MMCHS_BLOCK_t block;

	StorageType_t storage_type;
	Protocol_t protocol_type;

	void* buffer;
	unsigned int lba;		// from fs lib
	unsigned int block_size;
	unsigned int buffer_size;
	unsigned int expected_block_count;
} MMCHS_t;

#endif
