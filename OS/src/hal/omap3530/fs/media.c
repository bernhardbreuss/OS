/*
 * media.c
 *
 *  Created on: 29.05.2013
 *      Author: Daniel
 */
#include "media.h"
#include "../../generic/fs/media.h"
#include "types.h"
#include <string.h>

static mmchs_card_info_t mmchsCardInfo;

static bool_t mmchsMediaChanged = TRUE;
static uint32_t mmchsBlockSize = 512;
static uint32_t mmchsClockFrequencySelect = 0;
static uint64_t mmchsLastBlock = 0;
static uint64_t mmchsNumBlocks = 0;
static bool_t mmchsMediaPresent = FALSE;
static uint32_t mmchsMediaId = 0;

#define MMCHS_REG(reg) (*((memory_mapped_io_t)(MMCHS1 + reg)))

static inline bool_t omap3530_mmchs_is_error(uint32_t status)
{
    return (status != 0);
}

static void omap3530_mmchs_change_clock(uint32_t clockDivider)
{
    // TRM page 3174

    // disable clock
    MMCHS_REG(MMCHS_SYSCTL) &= ~CEN;

    // clear clock and set the new one
    MMCHS_REG(MMCHS_SYSCTL) &= ~CLKD_MASK;
    MMCHS_REG(MMCHS_SYSCTL) |= clockDivider << 6;

    // wait for clock to get stable
    while ((MMCHS_REG(MMCHS_SYSCTL) & ICS_MASK)!= ICS)
        ;

    // enable the clock again
    MMCHS_REG(MMCHS_SYSCTL) |= CEN;
}

static uint32_t omap3530_mmchs_send_cmd(uint32_t cmd, uint32_t cmdInterrupts, uint32_t arg)
{
    // TRM p.3172
    uint32_t mmcStatus;
    uint32_t retryCount = 0;

    // wait for commandline to get ready
    while ((MMCHS_REG(MMCHS_PSTATE) & CMDI_MASK)== CMDI_NOT_ALLOWED)
        ;

    // set block size
    MMCHS_REG(MMCHS_BLK) = BLEN_512BYTES;

    // reset the timeout counter
    MMCHS_REG(MMCHS_SYSCTL) &= ~DTO_MASK;
    MMCHS_REG(MMCHS_SYSCTL) |= DTO_VAL;

    // clear status register
    MMCHS_REG(MMCHS_STAT) = 0xFFFFFFFF;

    // set command argument
    MMCHS_REG(MMCHS_ARG) = arg;

    // enable needed command interrupts
    MMCHS_REG(MMCHS_IE) = cmdInterrupts;

    // send command
    MMCHS_REG(MMCHS_CMD) = cmd;

    // check for the command status
    while (retryCount < MMCHS_MAX_RETRY_COUNT)
    {
        // wait for any status
        do
        {
            mmcStatus = MMCHS_REG(MMCHS_STAT);
        } while (mmcStatus == 0);

        // Ignore CRC errors on CMD2 (https://code.google.com/p/beagleboard-freebsd/source/browse/trunk/sys/arm/cortexa8/omap3/omap3_mmc.c?r=15#774)
        if (cmd == CMD2 && (mmcStatus & CCRC)!= 0){
            mmcStatus &= ~(CCRC);
            // clear ERRI if no other error is set
            if ((mmcStatus & (0xFFFF0000)) == 0)
            {
                mmcStatus &= ~(ERRI);
            }
        }

        // check if any error bit was set (continue on timeout)
        if ((mmcStatus & ERRI)!= 0 && ((mmcStatus & CTO)!= CTO)){

        // soft-reset
MMCHS_REG        (MMCHS_SYSCTL) |= SRC;
        while ((MMCHS_REG(MMCHS_SYSCTL) & SRC))
        ;

        printf("Error sending command: %x\n", mmcStatus);
        return MMCHS_ERROR_DEVICE;
    }

        // return if done
        if ((mmcStatus & CC)== CC)
        {
            MMCHS_REG(MMCHS_STAT) = CC;
            break;
        }

        retryCount++;
    }

    if (retryCount == MMCHS_MAX_RETRY_COUNT)
    {
        printf("MMCHS send command timeout\n");
        return MMCHS_STATUS_TIMEOUT;
    }

    return MMCHS_STATUS_SUCCESS;
}

static void omap3530_mmchs_calculate_card_CLKD(uint32_t *clockFrequencySelect)
{
    uint8_t maxDataTransferRate;
    uint32_t transferRateValue = 0;
    uint32_t timeValue = 0;
    uint32_t frequency = 0;

    maxDataTransferRate = mmchsCardInfo.csdData.maxBusClock;

    switch (maxDataTransferRate & 0x7)
    {
        case 0:
            transferRateValue = 100 * 1000;
            break;
        case 1:
            transferRateValue = 1 * 1000 * 1000;
            break;
        case 2:
            transferRateValue = 10 * 1000 * 1000;
            break;
        case 3:
            transferRateValue = 100 * 1000 * 1000;
            break;
        default:
            printf("Invalid rate unit parameter.\n");
            break;
    }

    switch ((maxDataTransferRate >> 3) & 0xF)
    {
        case 1:
            timeValue = 10;
            break;
        case 2:
            timeValue = 12;
            break;
        case 3:
            timeValue = 13;
            break;
        case 4:
            timeValue = 15;
            break;
        case 5:
            timeValue = 20;
            break;
        case 6:
            timeValue = 25;
            break;
        case 7:
            timeValue = 30;
            break;
        case 8:
            timeValue = 35;
            break;
        case 9:
            timeValue = 40;
            break;
        case 10:
            timeValue = 45;
            break;
        case 11:
            timeValue = 50;
            break;
        case 12:
            timeValue = 55;
            break;
        case 13:
            timeValue = 60;
            break;
        case 14:
            timeValue = 70;
            break;
        case 15:
            timeValue = 80;
            break;
        default:
            printf("Invalid transfer speed parameter.\n");
            break;
    }

    frequency = transferRateValue * timeValue / 10;
    *clockFrequencySelect = ((MMC_REFERENCE_CLK / frequency) + 1);
}

static uint32_t omap3530_mmchs_read_block_data(void *buffer)
{
    uint32_t mmcStatus;
    uint32_t *dataBuffer = buffer;
    uint32_t dataSize = mmchsBlockSize / 4;
    uint32_t count;
    uint32_t retryCount = 0;

    while (retryCount < MMCHS_MAX_RETRY_COUNT)
    {
        do
        {
            mmcStatus = MMCHS_REG(MMCHS_STAT);
        } while (mmcStatus == 0);

        // Wait till buffer is ready
        if (mmcStatus & BRR)
        {
            MMCHS_REG(MMCHS_STAT) |= BRR;

            // Read the data
            for (count = 0; count < dataSize; count++)
            {
                *dataBuffer++ = MMCHS_REG(MMCHS_DATA);
            }
            break;
        }
        retryCount++;
    }

    if (retryCount == MMCHS_MAX_RETRY_COUNT)
    {
        return MMCHS_STATUS_TIMEOUT;
    }

    return MMCHS_STATUS_SUCCESS;
}

static uint32_t omap3530_mmchs_write_block_data(void *buffer)
{
    uint32_t mmcStatus;
    uint32_t *dataBuffer = buffer;
    uint32_t dataSize = mmchsBlockSize / 4;
    uint32_t count;
    uint32_t retryCount = 0;

    while (retryCount < MMCHS_MAX_RETRY_COUNT)
    {
        do
        {
            mmcStatus = MMCHS_REG(MMCHS_STAT);
        } while (mmcStatus == 0);

        // Wait fur write buffer to be ready
        if (mmcStatus & BWR)
        {
            MMCHS_REG(MMCHS_STAT) |= BWR;

            // Write the data
            for (count = 0; count < dataSize; count++)
            {
                MMCHS_REG(MMCHS_DATA) = *dataBuffer++;
            }

            break;
        }
        retryCount++;
    }

    if (retryCount == MMCHS_MAX_RETRY_COUNT)
    {
        return MMCHS_STATUS_TIMEOUT;
    }

    return MMCHS_STATUS_SUCCESS;
}

static uint32_t omap3530_mmchs_transfer_block(uint32_t lba, void* buffer, uint8_t operationType)
{
    uint32_t status;
    uint32_t mmcStatus;
    uint32_t retryCount = 0;
    uint32_t cmd = 0;
    uint32_t cmdInterruptEnable = 0;
    uint32_t cmdArgument = 0;

    // Determine operation
    switch (operationType)
    {
        case MMCHS_READ: // Single block read
            cmd = CMD17;
            cmdInterruptEnable = CMD18_INT_EN;
            break;
        case MMCHS_WRITE: // Single block Write
            cmd = CMD24;
            cmdInterruptEnable = CMD24_INT_EN;
            break;
    }

    // Set command argument based on the card access mode (Byte mode or Block mode)
    if (mmchsCardInfo.ocrData.accessMode & (0x01 << 1))
    {
        cmdArgument = lba;
    }
    else
    {
        cmdArgument = lba * mmchsBlockSize;
    }

    status = omap3530_mmchs_send_cmd(cmd, cmdInterruptEnable, cmdArgument);
    if (omap3530_mmchs_is_error(status))
    {
        return status;
    }

    // Read or Write data.
    if (operationType == MMCHS_READ)
    {
        status = omap3530_mmchs_read_block_data(buffer);
        if (omap3530_mmchs_is_error(status))
        {
            return status;
        }
    }
    else if (operationType == MMCHS_WRITE)
    {
        status = omap3530_mmchs_write_block_data(buffer);
        if (omap3530_mmchs_is_error(status))
        {
            return status;
        }
    }

    while (retryCount < MMCHS_MAX_RETRY_COUNT)
    {
        do
        {
            mmcStatus = MMCHS_REG(MMCHS_STAT);
        } while (mmcStatus == 0);

        // If the transfer is complete
        if (mmcStatus & TC)
        {
            break;
        }

        // Handle errors
        if ((mmcStatus & DEB)|| (mmcStatus & DCRC) || (mmcStatus & DTO)){
        MMCHS_REG(MMCHS_SYSCTL) |= SRD;
        while ((MMCHS_REG(MMCHS_SYSCTL) & SRD) != 0x0)
        ;

        return MMCHS_ERROR_DEVICE;
    }
        retryCount++;
    }

    if (retryCount == MMCHS_MAX_RETRY_COUNT)
    {
        return MMCHS_STATUS_TIMEOUT;
    }

    return MMCHS_STATUS_SUCCESS;
}

static uint32_t omap3530_mmchs_card_specific_data()
{
    uint32_t status;
    uint32_t cmdArgument;

    cmdArgument = mmchsCardInfo.rca << 16;
    status = omap3530_mmchs_send_cmd(CMD9, CMD9_INT_EN, cmdArgument);
    if (omap3530_mmchs_is_error(status))
    {
        return status;
    }

    //
    // Populate 128-bit card specific data.
    ((uint32_t *) &(mmchsCardInfo.csdData))[0] = MMCHS_REG(MMCHS_RSP10);
    ((uint32_t *) &(mmchsCardInfo.csdData))[1] = MMCHS_REG(MMCHS_RSP32);
    ((uint32_t *) &(mmchsCardInfo.csdData))[2] = MMCHS_REG(MMCHS_RSP54);
    ((uint32_t *) &(mmchsCardInfo.csdData))[3] = MMCHS_REG(MMCHS_RSP76);

    //
    // Block Count and Size
    mmchs_csd_sdv2_t* csdData;
    uint32_t cardSize;
    if (mmchsCardInfo.cardType == SD_CARD_2_HIGH)
    {
        csdData = (mmchs_csd_sdv2_t *) &mmchsCardInfo.csdData; // SDHC have other csd information
        mmchsCardInfo.blockSize = (0x1UL << csdData->readBlockLength);
        cardSize = csdData->deviceSizeLow16 | (csdData->deviceSizeHigh6 << 2);
        mmchsCardInfo.numBlocks = ((cardSize + 1) * 1024);
    }
    else
    {
        mmchsCardInfo.blockSize = (0x1UL << mmchsCardInfo.csdData.readBlockLength);
        cardSize = mmchsCardInfo.csdData.deviceSizeLow2 | (mmchsCardInfo.csdData.deviceSizeHigh10 << 2);
        mmchsCardInfo.numBlocks = (cardSize + 1) * (1 << (mmchsCardInfo.csdData.deviceSizeMultiplier + 2));
    }

    // HACK: For large cards (2gb) we might get a big block size, we reduce it for our needs
    if (mmchsCardInfo.blockSize > 512)
    {
        mmchsCardInfo.blockSize = 512;
    }

    //
    // Card Frequency
    uint32_t clockFrequencySelect;
    omap3530_mmchs_calculate_card_CLKD(&clockFrequencySelect);
    mmchsCardInfo.clockFrequencySelect = clockFrequencySelect;

    return status;
}

static inline void omap3530_mmchs_parse_cid_data(uint32_t r0, uint32_t r1, uint32_t r2, uint32_t r3)
{
    mmchsCardInfo.cidData.manufacturingDate = ((r0 >> 8) & 0xFFF);
    mmchsCardInfo.cidData.productSerialNumber = (((r0 >> 24) & 0xFF) | ((r1 & 0xFFFFFF) << 8));
    mmchsCardInfo.cidData.productRevision = ((r1 >> 24) & 0xFF);
    mmchsCardInfo.cidData.productName[4] = ((r2) & 0xFF);
    mmchsCardInfo.cidData.productName[3] = ((r2 >> 8) & 0xFF);
    mmchsCardInfo.cidData.productName[2] = ((r2 >> 16) & 0xFF);
    mmchsCardInfo.cidData.productName[1] = ((r2 >> 24) & 0xFF);
    mmchsCardInfo.cidData.productName[0] = ((r3) & 0xFF);
    mmchsCardInfo.cidData.oemId = ((r3 >> 8) & 0xFFFF);
    mmchsCardInfo.cidData.manufacturerId = ((r3 >> 24) & 0xFF);
}

/**
 * TRM page. 3164
 * @return
 */
static uint32_t omap3530_mmchs_perform_card_identification()
{
    //  TRM page. 3164
    uint32_t status;
    uint32_t cmdArg = 0;
    uint32_t response = 0;
    uint32_t retryCount = 0;
    uint8_t sdCmd8Supported = FALSE;

    // Module Initialization
    MMCHS_REG(MMCHS_IE) = (BADA_EN | CERR_EN | DEB_EN | DCRC_EN | DTO_EN | CIE_EN | CEB_EN | CCRC_EN | CTO_EN | BRR_EN
            | BWR_EN | TC_EN | CC_EN);

    // Begin Initialization
    MMCHS_REG(MMCHS_CON) |= INIT;
    MMCHS_REG(MMCHS_CMD) = 0x00000000;

//    kernel_sleep(1);
    int i = 0;
    while(++i < 200000);

    MMCHS_REG(MMCHS_STAT) |= CC;

    //End Initialization
    MMCHS_REG(MMCHS_CON) &= ~INIT;
    MMCHS_REG(MMCHS_HCTL) |= (SDVS_3_0_V | DTW_1_BIT | SDBP_ON);

    // Clear Status Register
    MMCHS_REG(MMCHS_STAT) = 0xFFFFFFFF;

    // Change clock frequency to 400KHz to fit protocol
    omap3530_mmchs_change_clock(CLKD_400KHZ);
    MMCHS_REG(MMCHS_CON) |= OD;

    // Send CMD0 command. (card reset for MMC)
    status = omap3530_mmchs_send_cmd(CMD0, CMD0_INT_EN, cmdArg);
    if (omap3530_mmchs_is_error(status))
    {
        return status;
    }

    // Send CMD5 command. (check if SDIO)
    status = omap3530_mmchs_send_cmd(CMD5, CMD5_INT_EN, cmdArg);
    if (!omap3530_mmchs_is_error(status))
    {
        // NOTE: We don't support SDIO
        return MMCHS_ERROR_UNSUPPORTED;
    }

    // Wait till ready
    MMCHS_REG(MMCHS_SYSCTL) |= SRC;
    while ((MMCHS_REG(MMCHS_SYSCTL) & SRC))
        ;

    //Send CMD8 command. (Check for SD2.0)
    cmdArg = CMD8_ARG;
    status = omap3530_mmchs_send_cmd(CMD8, CMD8_INT_EN, cmdArg);
    if (status == MMCHS_STATUS_SUCCESS)
    {
        response = MMCHS_REG(MMCHS_RSP10);
        if (response != cmdArg)
        {
            return MMCHS_ERROR_DEVICE;
        }
        sdCmd8Supported = TRUE; //Supports high capacity.
    }

    // Wait till ready
    MMCHS_REG(MMCHS_SYSCTL) |= SRC;
    while ((MMCHS_REG(MMCHS_SYSCTL) & SRC))
        ;

    // wait for card to get ready
    while (retryCount < MMCHS_MAX_RETRY_COUNT)
    {
        // Send CMD55 command. (initialize application specific command)
        cmdArg = 0;
        status = omap3530_mmchs_send_cmd(CMD55, CMD55_INT_EN, cmdArg);
        if (status == MMCHS_STATUS_SUCCESS)
        {
            mmchsCardInfo.cardType = SD_CARD;

            //
            // Send ACMD41 to load operating conditions

            cmdArg = ((uint32_t *) &(mmchsCardInfo.ocrData))[0];
            // Set high capacity support bit.
            if (sdCmd8Supported)
            {
                cmdArg |= (uint32_t) MMCHS_HCS;
            }
            status = omap3530_mmchs_send_cmd(ACMD41, ACMD41_INT_EN, cmdArg);
            if (omap3530_mmchs_is_error(status))
            {
                return status;
            }

            ((uint32_t *) &(mmchsCardInfo.ocrData))[0] = MMCHS_REG(MMCHS_RSP10);
        }
        else
        {
            mmchsCardInfo.cardType = MMC_CARD;
            cmdArg = 0;
            status = omap3530_mmchs_send_cmd(CMD1, CMD1_INT_EN, cmdArg);
            if (omap3530_mmchs_is_error(status))
            {
                return status;
            }
            response = MMCHS_REG(MMCHS_RSP10);
            // NOTE: MMC not supported yet
            return MMCHS_ERROR_UNSUPPORTED;
        }

        if (mmchsCardInfo.ocrData.busy == 1)
        {
            if (sdCmd8Supported)
            {
                mmchsCardInfo.cardType = SD_CARD_2;
            }

            // Check for card mode
            if (mmchsCardInfo.ocrData.accessMode & (0x01 << 1))
            {
                mmchsCardInfo.cardType = SD_CARD_2_HIGH;
            }
            break;
        }
//        kernel_sleep(1);
        int i = 0;
		while(++i < 200000);

        retryCount++;
    }

    if (retryCount == MMCHS_MAX_RETRY_COUNT)
    {
        printf("Timeout error. retry_count: %d\n", retryCount);
        return MMCHS_STATUS_TIMEOUT;
    }

    // Read card identification number (CID)
    cmdArg = 0;
    status = omap3530_mmchs_send_cmd(CMD2, CMD2_INT_EN, cmdArg);
    if (omap3530_mmchs_is_error(status))
    {
        return status;
    }

    // Parse card information.
    omap3530_mmchs_parse_cid_data(MMCHS_REG(MMCHS_RSP10), MMCHS_REG(MMCHS_RSP32), MMCHS_REG(MMCHS_RSP54),
            MMCHS_REG(MMCHS_RSP76) );

    // Read card relative address
    cmdArg = 0;
    status = omap3530_mmchs_send_cmd(CMD3, CMD3_INT_EN, cmdArg);
    if (omap3530_mmchs_is_error(status))
    {
        return status;
    }
    mmchsCardInfo.rca = (MMCHS_REG(MMCHS_RSP10) >> 16);

    MMCHS_REG(MMCHS_CON) &= ~OD;
    MMCHS_REG(MMCHS_HCTL) |= SDVS_3_0_V;
    omap3530_mmchs_change_clock(CLKD_400KHZ);

    return MMCHS_STATUS_SUCCESS;
}

static uint32_t omap3530_mmchs_perform_card_configuration()
{
    uint32_t cmdArgument = 0;
    uint32_t status;

    // Send CMD7 (Select card)
    cmdArgument = mmchsCardInfo.rca << 16;
    status = omap3530_mmchs_send_cmd(CMD7, CMD7_INT_EN, cmdArgument);
    if (omap3530_mmchs_is_error(status))
    {
        return status;
    }

    // Is a SD Card?
    if ((mmchsCardInfo.cardType != UNKNOWN_CARD) && (mmchsCardInfo.cardType != MMC_CARD))
    {
        // CMD55 for application specific command
        status = omap3530_mmchs_send_cmd(CMD55, CMD55_INT_EN, cmdArgument);
        if (!omap3530_mmchs_is_error(status))
        {
            // set device into 4-bit data bus mode
            status = omap3530_mmchs_send_cmd(ACMD6, ACMD6_INT_EN, 0x2);
            if (!omap3530_mmchs_is_error(status))
            {
                // and set host to 4-bit data bus mode
                MMCHS_REG(MMCHS_HCTL) |= DTW_4_BIT;
            }
        }
    }

    // Send CMD16 to set the block length
    cmdArgument = mmchsCardInfo.blockSize;
    status = omap3530_mmchs_send_cmd(CMD16, CMD16_INT_EN, cmdArgument);
    if (omap3530_mmchs_is_error(status))
    {
        return status;
    }

    // Set correct clock
    omap3530_mmchs_change_clock(mmchsClockFrequencySelect);

    return MMCHS_STATUS_SUCCESS;
}

static uint32_t omap3530_mmchs_detect_card(void)
{
    uint32_t status;

    // Software reset of the MMCHS host controller.
    MMCHS_REG(MMCHS_SYSCONFIG) = SOFTRESET;
    while ((MMCHS_REG(MMCHS_SYSSTATUS) & RESETDONE)!= RESETDONE)
        ;

    // Voltage capabilities initialization. Activate VS18 and VS30.
    MMCHS_REG(MMCHS_CAPA) |= (VS30 | VS18);

    // Wake up configuration
    MMCHS_REG(MMCHS_SYSCONFIG) |= ENAWAKEUP;
    MMCHS_REG(MMCHS_HCTL) |= IWE;

    // MMCHS Controller default initialization
    MMCHS_REG(MMCHS_CON) |= (OD | DW8_1_4_BIT | CEATA_OFF);
    MMCHS_REG(MMCHS_HCTL) = (SDVS_3_0_V | DTW_1_BIT | SDBP_OFF);

    // Enable internal clock
    MMCHS_REG(MMCHS_SYSCTL) |= ICE;

    // Set the clock frequency to 80KHz.
    omap3530_mmchs_change_clock(CLKD_80KHZ);

    // Enable SD bus power.
    MMCHS_REG(MMCHS_HCTL) |= SDBP_ON;

    // Poll till SD bus power bit is set.
    while ((MMCHS_REG(MMCHS_HCTL) & SDBP_MASK)!= SDBP_ON)
        ;

    status = omap3530_mmchs_perform_card_identification();
    if (omap3530_mmchs_is_error(status))
    {
        printf("No MMC/SD card detected.\n");
        return status;
    }

    status = omap3530_mmchs_card_specific_data();
    if (omap3530_mmchs_is_error(status))
    {
        return status;
    }

    status = omap3530_mmchs_perform_card_configuration();
    if (omap3530_mmchs_is_error(status))
    {
        return status;
    }

    // Configure media info
    mmchsLastBlock = (mmchsNumBlocks - 1);
    mmchsBlockSize = mmchsCardInfo.blockSize;
    mmchsClockFrequencySelect = mmchsCardInfo.clockFrequencySelect;
    mmchsMediaPresent = TRUE;
    mmchsMediaId++;

    printf("SD Card Media Change SUCCESS\n");
    return status;
}

static uint32_t omap3530_mmchs_read_write(uint32_t lba, void* buffer, size_t bufferSize, uint8_t operationType)
{
    uint32_t status = MMCHS_STATUS_SUCCESS;
    uint32_t retryCount = 0;
    uint32_t blockCount;
    uint32_t bytesToTransfer = 0;
    uint32_t bytesRemaining;

    //
    // Detect new cards
    if (mmchsMediaChanged)
    {
        status = omap3530_mmchs_detect_card();
        if (omap3530_mmchs_is_error(status))
        {
            mmchsMediaPresent = FALSE;
            mmchsLastBlock = 0;
            mmchsBlockSize = 512;
            mmchsMediaChanged = FALSE;
            return status;
        }
        mmchsMediaChanged = FALSE;
    }
    else if (!mmchsMediaPresent)
    {
        return MMCHS_ERROR_NO_MEDIA;
    }

    //
    // Validate parameters
    if (buffer == NULL)
    {
        return MMCHS_ERROR_INVALID_PARAMETER;
    }

    if (lba > mmchsLastBlock)
    {
        return MMCHS_ERROR_INVALID_PARAMETER;
    }

    if ((bufferSize % mmchsBlockSize) != 0)
    {
        return MMCHS_ERROR_BAD_BUFFER_SIZE;
    }

    //
    // Wait till ready
    while ((retryCount++ < MMCHS_MAX_RETRY_COUNT)&& ((MMCHS_REG(MMCHS_PSTATE) & DATI_MASK) != DATI_ALLOWED));
    if (retryCount == MMCHS_MAX_RETRY_COUNT)
    {
        return MMCHS_STATUS_TIMEOUT;
    }

    // Read the requested blocks
    bytesRemaining = bufferSize;
    while (bytesRemaining > 0)
    {
        // media changed during reading?
        if (mmchsMediaChanged)
        {
            return MMCHS_ERROR_NO_MEDIA;
        }

        bytesToTransfer = mmchsBlockSize;
        blockCount = bytesToTransfer / mmchsBlockSize;
        status = omap3530_mmchs_transfer_block(lba, buffer, operationType);

        if (omap3530_mmchs_is_error(status))
        {
            return status;
        }

        bytesRemaining -= bytesToTransfer;
        lba += blockCount;
        buffer = (uint8_t *) buffer + mmchsBlockSize;
    }

    return status;
}

void __media_init()
{
    // Start Clock
	unsigned int CORE_CM = 0x48004A00;
	unsigned int CM_ICLKEN = 0x0010;
	unsigned int CM_FCLKEN = 0x0000;
    *((memory_mapped_io_t) (CORE_CM + CM_ICLKEN)) |= EN_MMC1/* | EN_MMC2 | EN_MMC3*/;
    *((memory_mapped_io_t) (CORE_CM + CM_FCLKEN)) |= EN_MMC1/* | EN_MMC2 | EN_MMC3*/;

    memset(&mmchsCardInfo, 0, sizeof(mmchsCardInfo));
}

int32_t __media_read(uint32_t sector, void* buffer, uint32_t bufferSize)
{
    return omap3530_mmchs_read_write(sector, buffer, bufferSize, MMCHS_READ);
}

int32_t __media_write(uint32_t sector, void* buffer, uint32_t bufferSize)
{
    return omap3530_mmchs_read_write(sector, buffer, bufferSize, MMCHS_WRITE);
}
