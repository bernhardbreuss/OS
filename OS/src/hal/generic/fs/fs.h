/*
 * file_system.h
 *
 *  Created on: 10.05.2013
 *      Author: lumannnn
 */

#ifndef FS_H_
#define FS_H_

#include "fat_io_lib/fat_filelib.h"
//#include "../../../platform/omap3530/mmchs.h"


#define SUCCESS 1
#define ERROR	0
typedef int RESPONSE_t;

#define TRUE 1
#define FALSE 0

RESPONSE_t fs_init();

int fs_read(uint32 sector, uint8 *buffer, uint32 sector_count);

int fs_write(uint32 sector, uint8 *buffer, uint32 sector_count);



#endif /* FS_H_ */
