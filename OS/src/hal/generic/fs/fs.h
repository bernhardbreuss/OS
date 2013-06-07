/*
 * file_system.h
 *
 *  Created on: 10.05.2013
 *      Author: lumannnn
 */

#ifndef FS_H_
#define FS_H_

#include <stdio.h>
#include "fat_io_lib/fat_filelib.h"

// FIXME move this to "platform.h"
#include "../../omap3530/fs/mmchs.h"


#define SUCCESS 0
#define ERROR	1
typedef int RESPONSE_t;

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

typedef struct _FileHandle {
	MMCHS_t* instance;				// associated with the above 'FIXME.'
	StorageType_t storage_type;
	Protocol_t* protocol_type;
} FileHandle_t;

/**
 * Initialise file system
 */
RESPONSE_t fs_init(FileHandle_t* handle);

RESPONSE_t fs_read(FileHandle_t* handle);

RESPONSE_t fs_write(FileHandle_t* handle);

/*
int media_init() {
    // ...
    return 1;
}

int media_read(unsigned long sector, unsigned char *buffer, unsigned long sector_count) {
    unsigned long i;

    fl_init();

    for (i=0;i<sector_count;i++) {
        // ...
        // Add platform specific sector (512 bytes) read code here
        //..

        sector ++;
        buffer += 512;
    }

    return 1;
}

int media_write(unsigned long sector, unsigned char *buffer, unsigned long sector_count) {
    unsigned long i;

    for (i=0;i<sector_count;i++) {
        // ...
        // Add platform specific sector (512 bytes) write code here
        //..

        sector ++;
        buffer += 512;
    }

    return 1;
}
*/

#endif /* FS_H_ */
