/*
 * device.h
 *
 *  Created on: 02.05.2013
 *      Author: Stephan
 */

#ifndef DEVICE_H_
#define DEVICE_H_

#include <stdint.h>

#define DEVICE_DRIVER_REGISTER(device_t, driver_t) (devices_create(device_t, driver_t))


typedef int device_t;

/*
 * Device identifiers
 *
 * WARNING: always check the <ottos/limits.h> for DEVICE_MAX_COUNT when adding
 *          new devices.
 */

#define DEVICE_MAX_COUNT 30

#define DEVICE_INVALID  -1

#define GPTIMER_1       0
#define GPTIMER_2       1
#define GPTIMER_3       2
#define GPTIMER_4       3
#define GPTIMER_5       4
#define GPTIMER_6       5
#define GPTIMER_7       6
#define GPTIMER_8       7
#define GPTIMER_9       8
#define GPTIMER_10      9
#define GPTIMER_11      10
#define LED_0           11
#define LED_1           12
#define SERIAL_0        13

/**
 * External devices
 */
#define DEV_STATUS uint32_t

typedef struct _EXTERNAL_DEVICE EXTERNAL_DEVICE;

typedef DEV_STATUS(*EXTERNAL_DEVICE_READ)(
    EXTERNAL_DEVICE *self,
    uint32_t reg,
    uint32_t length,
    void* buffer);

typedef DEV_STATUS(*EXTERNAL_DEVICE_WRITE)(
    EXTERNAL_DEVICE *self,
    uint32_t reg,
    uint32_t length,
    void* buffer);

struct _EXTERNAL_DEVICE {
    EXTERNAL_DEVICE_READ read;
    EXTERNAL_DEVICE_WRITE write;
};

#endif /* DEVICE_H_ */
