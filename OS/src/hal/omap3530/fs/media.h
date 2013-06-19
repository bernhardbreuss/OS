/*
 * media.h
 *
 *  Created on: 29.05.2013
 *      Author: Daniel
 */

#ifndef MEDIA_OMAP3530_H_
#define MEDIA_OMAP3530_H_

#include "types.h"

#define MMCHS_MAX_RETRY_COUNT   (100*5)

//
// Registers
//

// TRM. 22.7.1 (p.3189)
#define MMCHS1 0x4809C000
#define MMCHS2 0x480B4000
#define MMCHS3 0x480AD000

#define MMC_REFERENCE_CLK (96000000)

#define MMCHS_SYSCONFIG 0x0010
    #define SOFTRESET         (0x01 << 1)
    #define ENAWAKEUP         (0x01 << 2)

#define MMCHS_SYSSTATUS 0x0014
    #define RESETDONE         (0x01 << 0)

#define MMCHS_CSRE      0x0024

#define MMCHS_SYSTEST   0x0028

#define MMCHS_CON       0x002C
    #define OD                (0x01 << 0)
    #define NOINIT            (0x0UL << 1)
    #define INIT              (0x01 << 1)
    #define HR                (0x01 << 2)
    #define STR               (0x01 << 3)
    #define MODE              (0x01 << 4)
    #define DW8_1_4_BIT       (0x0UL << 5)
    #define DW8_8_BIT         (0x01 << 5)
    #define MIT               (0x01 << 6)
    #define CDP               (0x01 << 7)
    #define WPP               (0x01 << 8)
    #define CTPL              (0x01 << 11)
    #define CEATA_OFF         (0x0UL << 12)
    #define CEATA_ON          (0x01 << 12)

#define MMCHS_PWCNT     0x0030

#define MMCHS_BLK       0x0104
    #define BLEN_512BYTES     (0x200UL << 0)

#define MMCHS_ARG       0x0108

#define MMCHS_CMD       0x010C
    #define DE_ENABLE         (0x01 << 0)
    #define BCE_ENABLE        (0x01 << 1)
    #define ACEN_ENABLE       (0x01 << 2)
    #define DDIR_READ         (0x01 << 4)
    #define DDIR_WRITE        (0x0UL << 4)
    #define MSBS_SGLEBLK      (0x0UL << 5)
    #define MSBS_MULTBLK      (0x01 << 5)
    #define RSP_TYPE_MASK     (0x3UL << 16)
    #define RSP_TYPE_136BITS  (0x01 << 16)
    #define RSP_TYPE_48BITS   (0x2UL << 16)
    #define CCCE_ENABLE       (0x01 << 19)
    #define CICE_ENABLE       (0x01 << 20)
    #define DP_ENABLE         (0x01 << 21)
    #define INDX(CMD_INDX)    ((CMD_INDX & 0x3F) << 24)

#define MMCHS_RSP10     0x0110

#define MMCHS_RSP32     0x0114

#define MMCHS_RSP54     0x0118

#define MMCHS_RSP76     0x011C

#define MMCHS_DATA      0x0120

#define MMCHS_PSTATE    0x0124
    #define CMDI_MASK         (0x01 << 0)
    #define CMDI_ALLOWED      (0x0UL << 0)
    #define CMDI_NOT_ALLOWED  (0x01 << 0)
    #define DATI_MASK         (0x01 << 1)
    #define DATI_ALLOWED      (0x0UL << 1)
    #define DATI_NOT_ALLOWED  (0x01 << 1)

#define MMCHS_HCTL      0x0128
    #define DTW_1_BIT         (0x0UL << 1)
    #define DTW_4_BIT         (0x01 << 1)
    #define SDBP_MASK         (0x01 << 8)
    #define SDBP_OFF          (0x0UL << 8)
    #define SDBP_ON           (0x01 << 8)
    #define SDVS_1_8_V        (0x5UL << 9)
    #define SDVS_3_0_V        (0x6UL << 9)
    #define IWE               (0x01 << 24)

#define MMCHS_SYSCTL    0x012C
    #define ICE               (0x01 << 0)
    #define ICS_MASK          (0x01 << 1)
    #define ICS               (0x01 << 1)
    #define CEN               (0x01 << 2)
    #define CLKD_MASK         (0x3FFUL << 6)
    #define CLKD_80KHZ        (0x258UL)
    #define CLKD_400KHZ       (0xF0UL)
    #define DTO_MASK          (0xFUL << 16)
    #define DTO_VAL           (0xEUL << 16)
    #define SRA               (0x01 << 24)
    #define SRC_MASK          (0x01 << 25)
    #define SRC               (0x01 << 25)
    #define SRD               (0x01 << 26)

#define MMCHS_STAT      0x0130
    #define CC                (0x01 << 0)
    #define TC                (0x01 << 1)
    #define BWR               (0x01 << 4)
    #define BRR               (0x01 << 5)
    #define ERRI              (0x01 << 15)
    #define CTO               (0x01 << 16)
    #define CCRC              (0x01 << 17)
    #define DTO               (0x01 << 20)
    #define DCRC              (0x01 << 21)
    #define DEB               (0x01 << 22)

#define MMCHS_IE        0x0134
    #define CC_EN             (0x01 << 0)
    #define TC_EN             (0x01 << 1)
    #define BWR_EN            (0x01 << 4)
    #define BRR_EN            (0x01 << 5)
    #define CTO_EN            (0x01 << 16)
    #define CCRC_EN           (0x01 << 17)
    #define CEB_EN            (0x01 << 18)
    #define CIE_EN            (0x01 << 19)
    #define DTO_EN            (0x01 << 20)
    #define DCRC_EN           (0x01 << 21)
    #define DEB_EN            (0x01 << 22)
    #define CERR_EN           (0x01 << 28)
    #define BADA_EN           (0x01 << 29)

#define MMCHS_ISE       0x0138
    #define CC_SIGEN          (0x01 << 0)
    #define TC_SIGEN          (0x01 << 1)
    #define BWR_SIGEN         (0x01 << 4)
    #define BRR_SIGEN         (0x01 << 5)
    #define CTO_SIGEN         (0x01 << 16)
    #define CCRC_SIGEN        (0x01 << 17)
    #define CEB_SIGEN         (0x01 << 18)
    #define CIE_SIGEN         (0x01 << 19)
    #define DTO_SIGEN         (0x01 << 20)
    #define DCRC_SIGEN        (0x01 << 21)
    #define DEB_SIGEN         (0x01 << 22)
    #define CERR_SIGEN        (0x01 << 28)
    #define BADA_SIGEN        (0x01 << 29)

#define MMCHS_AC12      0x013C

#define MMCHS_CAPA      0x0140
    #define VS30              (0x01 << 25)
    #define VS18              (0x01 << 26)

#define MMCHS_CUR_CAPA  0x0148

#define MMCHS_REV       0x01FC


//
// Commands
//
#define CMD0              INDX(0)
#define CMD0_INT_EN       (CC_EN | CEB_EN)

#define CMD1              (INDX(1) | RSP_TYPE_48BITS)
#define CMD1_INT_EN       (CC_EN | CEB_EN | CTO_EN)

#define CMD2              (INDX(2) | CCCE_ENABLE | RSP_TYPE_136BITS)
#define CMD2_INT_EN       (CERR_EN | CIE_EN | CCRC_EN | CC_EN | CEB_EN | CTO_EN)

#define CMD3              (INDX(3) | CICE_ENABLE | CCCE_ENABLE | RSP_TYPE_48BITS)
#define CMD3_INT_EN       (CERR_EN | CIE_EN | CCRC_EN | CC_EN | CEB_EN | CTO_EN)

#define CMD5              (INDX(5) | RSP_TYPE_48BITS)
#define CMD5_INT_EN       (CC_EN | CEB_EN | CTO_EN)

#define CMD7              (INDX(7) | CICE_ENABLE | CCCE_ENABLE | RSP_TYPE_48BITS)
#define CMD7_INT_EN       (CERR_EN | CIE_EN | CCRC_EN | CC_EN | CEB_EN | CTO_EN)

#define CMD8              (INDX(8) | CICE_ENABLE | CCCE_ENABLE | RSP_TYPE_48BITS)
#define CMD8_INT_EN       (CERR_EN | CIE_EN | CCRC_EN | CC_EN | CEB_EN | CTO_EN)
#define CMD8_ARG          (0x0UL << 12 | (0x01 << 8) | 0xCEUL << 0)

#define CMD9              (INDX(9) | CCCE_ENABLE | RSP_TYPE_136BITS)
#define CMD9_INT_EN       (CCRC_EN | CC_EN | CEB_EN | CTO_EN)

#define CMD16             (INDX(16) | CICE_ENABLE | CCCE_ENABLE | RSP_TYPE_48BITS)
#define CMD16_INT_EN      (CERR_EN | CIE_EN | CCRC_EN | CC_EN | CEB_EN | CTO_EN)

#define CMD17             (INDX(17) | DP_ENABLE | CICE_ENABLE | CCCE_ENABLE | RSP_TYPE_48BITS | DDIR_READ)
#define CMD17_INT_EN      (CERR_EN | CIE_EN | CCRC_EN | CC_EN | TC_EN | BRR_EN | CTO_EN | DTO_EN | DCRC_EN | DEB_EN | CEB_EN)

#define CMD18             (INDX(18) | DP_ENABLE | CICE_ENABLE | CCCE_ENABLE | RSP_TYPE_48BITS | MSBS_MULTBLK | DDIR_READ | BCE_ENABLE | DE_ENABLE)
#define CMD18_INT_EN      (CERR_EN | CIE_EN | CCRC_EN | CC_EN | TC_EN | BRR_EN | CTO_EN | DTO_EN | DCRC_EN | DEB_EN | CEB_EN)

#define CMD23             (INDX(23) | CICE_ENABLE | CCCE_ENABLE | RSP_TYPE_48BITS)
#define CMD23_INT_EN      (CERR_EN | CIE_EN | CCRC_EN | CC_EN | CEB_EN | CTO_EN)

#define CMD24             (INDX(24) | DP_ENABLE | CICE_ENABLE | CCCE_ENABLE | RSP_TYPE_48BITS | DDIR_WRITE)
#define CMD24_INT_EN      (CERR_EN | CIE_EN | CCRC_EN | CC_EN | TC_EN | BWR_EN | CTO_EN | DTO_EN | DCRC_EN | DEB_EN | CEB_EN)

#define CMD25             (INDX(25) | DP_ENABLE | CICE_ENABLE | CCCE_ENABLE | RSP_TYPE_48BITS | MSBS_MULTBLK | DDIR_READ | BCE_ENABLE | DE_ENABLE)
#define CMD25_INT_EN      (CERR_EN | CIE_EN | CCRC_EN | CC_EN | TC_EN | BRR_EN | CTO_EN | DTO_EN | DCRC_EN | DEB_EN | CEB_EN)

#define CMD55             (INDX(55) | CICE_ENABLE | CCCE_ENABLE | RSP_TYPE_48BITS)
#define CMD55_INT_EN      (CERR_EN | CIE_EN | CCRC_EN | CC_EN | CEB_EN | CTO_EN)

#define ACMD41            (INDX(41) | RSP_TYPE_48BITS)
#define ACMD41_INT_EN     (CERR_EN | CIE_EN | CCRC_EN | CC_EN | CEB_EN | CTO_EN)

#define ACMD6             (INDX(6) | RSP_TYPE_48BITS)
#define ACMD6_INT_EN      (CERR_EN | CIE_EN | CCRC_EN | CC_EN | CEB_EN | CTO_EN)

//
// Other flags
//

#define MMCHS_HCS (0x01<< 30)

#define EN_MMC3 (0x01 << 30)
#define EN_MMC2 (0x01 << 25)
#define EN_MMC1 (0x01 << 23)

#define MMCHS_READ 0
#define MMCHS_WRITE 1

//
// Status Codes
//
#define MMCHS_STATUS_SUCCESS     0
#define MMCHS_STATUS_LOAD_ERROR  1
#define MMCHS_STATUS_TIMEOUT     2

#define MMCHS_ERROR_DEVICE            3
#define MMCHS_ERROR_UNSUPPORTED       4
#define MMCHS_ERROR_INVALID           5
#define MMCHS_ERROR_NO_MEDIA          6
#define MMCHS_ERROR_INVALID_PARAMETER 7
#define MMCHS_ERROR_BAD_BUFFER_SIZE   8
#define MMCHS_ERROR_MEDIA_CHANGED     9

//
// Types
//
typedef enum
{
    UNKNOWN_CARD, MMC_CARD, SD_CARD, SD_CARD_2, SD_CARD_2_HIGH
} mmchs_card_type_t;

typedef struct
{
    uint32_t reserved0 :7;
    uint32_t v170v195 :1;
    uint32_t v200v260 :7;
    uint32_t v270v360 :9;
    uint32_t reserved1 :5;
    uint32_t accessMode :2;
    uint32_t busy :1;
} mmchs_ocr_t;

typedef struct
{
    uint32_t reserved0;
    uint32_t crc;
    uint32_t manufacturingDate;
    uint32_t reserved1;
    uint32_t productSerialNumber;
    uint8_t productRevision;
    uint8_t productName[5];
    uint16_t oemId;
    uint8_t manufacturerId;
} mmchs_cid_t;

typedef struct
{
    uint32_t reserved0 :1;
    uint32_t crc :7;

    uint32_t reserved1 :2;
    uint32_t fileFromat :2;
    uint32_t tmpWriteProtect :1;
    uint32_t permWriteProtect :1;
    uint32_t copy :1;
    uint32_t fileFormatgroup :1;

    uint32_t reserved2 :5;
    uint32_t writePartialBlocks :1;
    uint32_t writePartialBlockLength :4;
    uint32_t writeSpeedFactor :3;
    uint32_t reserved3 :2;
    uint32_t writeProtectGroupEnable :1;

    uint32_t writeProtectGroupSize :7;
    uint32_t eraseSectorSize :7;
    uint32_t eraseBlockEnable :1;
    uint32_t deviceSizeMultiplier :3;
    uint32_t maxWriteCurrent :3;
    uint32_t minWriteCurrent :3;
    uint32_t maxReadCurrent :3;
    uint32_t minReadCurrent :3;
    uint32_t deviceSizeLow2 :2;

    uint32_t deviceSizeHigh10 :10;
    uint32_t reserved4 :2;
    uint32_t dsrImplemented :1;
    uint32_t readBlockMisalignment :1;
    uint32_t writeBlockMisalignment :1;
    uint32_t readPartialBlocks :1;
    uint32_t readBlockLength :4;
    uint32_t cardCommandClasses :12;

    uint8_t maxBusClock;
    uint8_t dataReadAccessTime2;
    uint8_t dataReadAccessTime1;

    uint32_t reserved5 :6;
    uint32_t csdStructure :2;
} mmchs_csd_t;

typedef struct
{
    uint32_t reserved0 :1;
    uint32_t crc :7;
    uint32_t reserved1 :2;
    uint32_t fileFormat :2;
    uint32_t tmpWriteProtect :1;
    uint32_t permWriteProtect :1;
    uint32_t copy :1;
    uint32_t fileFormatGroup :1;
    uint32_t reserved2 :5;
    uint32_t partialBlockWrite :1;
    uint32_t partialBlockLength :4;
    uint32_t writeSpeedFactor :3;
    uint32_t reserved3 :2;
    uint32_t writeProtectGroupEnable :1;
    uint32_t writeProtectGroupSize :7;
    uint32_t sectorSize :7;
    uint32_t eraseBlockEnable :1;
    uint32_t reserved4 :1;
    uint32_t deviceSizeLow16 :16;
    uint32_t deviceSizeHigh6 :6;
    uint32_t reserved5 :6;
    uint32_t dsrImplemented :1;
    uint32_t readBlockMisalignment :1;
    uint32_t writeBlockMisalignment :1;
    uint32_t readPartialBlocks :1;
    uint32_t readBlockLength :4;
    uint32_t cardCommandClasses :12;
    uint8_t busClockFrequency;
    uint8_t dataReadAccessTime2;
    uint8_t dataReadAccessTime1;
    uint32_t reserved6 :6;
    uint32_t csdStructure :2;
} mmchs_csd_sdv2_t;

typedef struct
{
    uint16_t rca;
    uint32_t blockSize;
    uint32_t numBlocks;
    uint32_t clockFrequencySelect;
    mmchs_card_type_t cardType;
    mmchs_ocr_t ocrData;
    mmchs_cid_t cidData;
    mmchs_csd_t csdData;
} mmchs_card_info_t;

#endif /* MEDIA_H_ */
