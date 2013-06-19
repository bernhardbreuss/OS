/*
 * pwm.h
 *
 *  Created on: 10.05.2013
 *      Author: lumannnn
 */

#ifndef OMAP3530_PWM_H_
#define OMAP3530_PWM_H_


#include "../../generic/pwm/pwm.h"
#include "../../generic/timer/gptimer.h"
#include "../../../service/logger/logger.h"
#include <bit.h>

/* ************************* *
 * 			PWM
 * ************************* */

/* set mode to 4 (GPIO) see p. ~787 of omap35x.pdf */
#define CONTROL_PADCONF_UART2_CTS	((unsigned int*) 0x48002174)	/* GPIO144 15:0 GPIO145 16:32 */
#define CONTROL_PADCONF_UART2_TX	((unsigned int*) 0x48002178)	/* GPIO146 15:0 */

#define PM_PWSTCTRL_CORE ((unsigned int*) 0x48306AE0)
#define PM_PWSTCTRL_PER ((unsigned int*) 0x483070E0)


// TIOCP_CFG, OMAP35x.pdf, page 2637
#define TIOCP_CFG_9 	((unsigned int*) 0x49040010)
#define TIOCP_CFG_10 	((unsigned int*) 0x48086010)
#define TIOCP_CFG_11 	((unsigned int*) 0x48088010)

#define TISTAT_9 		((unsigned int*) 0x49040014)
#define TISTAT_10 		((unsigned int*) 0x48086014)
#define TISTAT_11 		((unsigned int*) 0x48088014)

#endif /* PWM_H_ */
