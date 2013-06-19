/*
 * fs.c
 *
 *  Created on: 19.06.2013
 *      Author: lumannnn
 */


#include "../../generic/fs/fs.h"
#include "../../generic/fs/media.h"

RESPONSE_t fs_init() {
	__media_init();

	fl_init();
	fl_attach_media(&fs_read, &fs_write);

	return 1;
}

int fs_read(uint32 sector, uint8 *buffer, uint32 sector_count) {
	 return (!__media_read((uint32_t)sector, (void*)buffer, (uint32_t)(sector_count * FAT_SECTOR_SIZE)));
}

int fs_write(uint32 sector, uint8 *buffer, uint32 sector_count) {
	 return (!__media_write((uint32_t)sector, (void*)buffer, (uint32_t)(sector_count * FAT_SECTOR_SIZE)));
}
