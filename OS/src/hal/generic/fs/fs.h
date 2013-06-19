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
#include "../../../platform/omap3530/mmchs.h"


#define SUCCESS 1
#define ERROR	0
typedef int RESPONSE_t;

#define TRUE 1
#define FALSE 0

//typedef struct _FileHandle {
//
//} FileHandle_t;

/**
 * Initialise file system
 */

RESPONSE_t fs_init();

int fs_read(uint32 sector, uint8 *buffer, uint32 sector_count);

int fs_write(uint32 sector, uint8 *buffer, uint32 sector_count);

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
