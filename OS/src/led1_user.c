/*
 * led1_user.c
 *
 *  Created on: May 31, 2013
 *      Author: Bernhard
 */

#include <inttypes.h>

#define LED1_PIN			(1 << 22)
uint32_t led1_user(void) {
	int i;
	while (1) {
		for(i = 0; i < 900000; i++);
		memory_mapped_read(&gpio5, (unsigned int*) 0x4905603C);
		gpio5 ^= LED1_PIN;
		memory_mapped_write(gpio5, (unsigned int*) 0x4905603C);
	}
}
