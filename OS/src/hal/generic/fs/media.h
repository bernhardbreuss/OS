/*
 * media.h
 *
 *  Created on: 29.05.2013
 *      Author: Daniel
 */

#ifndef MEDIA_GENARCH_H_
#define MEDIA_GENARCH_H_

void __media_init(void);

typedef unsigned int uint32_t;
typedef int int32_t;

int32_t __media_read(uint32_t sector, void* buffer, uint32_t bufferSize);
int32_t __media_write(uint32_t sector, void* buffer, uint32_t bufferSize);

#endif /* MEDIA_H_ */
