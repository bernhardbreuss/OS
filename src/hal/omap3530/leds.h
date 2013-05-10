/*
 * leds.h
 *
 *  Created on: 03.05.2013
 *      Author: Stephan
 */

#ifndef LEDS_H_
#define LEDS_H_

#include "gpio.h"

#define LED_ON        1
#define LED_OFF       0

typedef enum LED_DEVICE
{
  LED_DEVICE_USR0 = 22,
  LED_DEVICE_USR1 = 21
} LED_DEVICE;

#endif /* LEDS_H_ */
