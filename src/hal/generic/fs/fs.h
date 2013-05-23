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

/**
 * Initialise file system
 */
int fs_init();

int fs_read();

int fs_write();

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
