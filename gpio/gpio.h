/*
 * gpio.h
 *
 *  Created on: May 9, 2013
 *      Author: Bernhard
 */

#ifndef GPIO_H_
#define GPIO_H_

unsigned int GPIO5_OE = 0x49056034;
unsigned int GPIO5_DATAOUT = 0x4905603C;

#define GPIO_ON		0x1
#define GPIO_TOGGLE	0x3
#define GPIO_OFF	0x2

#endif /* GPIO_H_ */
