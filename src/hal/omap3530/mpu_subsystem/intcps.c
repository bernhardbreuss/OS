/*
 * intcps.c
 *
 *  Created on: 03.05.2013
 *      Author: edi
 */

#include "../../generic/mpu_subsystem/intcps.h"
#include "../timer/gptimer.h"
#include "intcps.h"
#include "../../../service/logger/logger.h"

void intcps_activate_gptimer(const gptimer_t* const timer) {

	unsigned int* mpuintc_mirn = (unsigned int*) (INTCPS_MIRn+((38/32)*0x20));
	logger_log_register("INTPCS_MIR before unmasking %s: ", mpuintc_mirn);

	// activate the specific interrupt mask
	unsigned int* mpuintc_mir_clearn =
			(unsigned int*)(INTCPS_MIR_CLEARn +
					((timer->intcps_mapping_id/INTCPS_REGISTER_N_BIT_SIZE)*INTCPS_REGISTER_N_BIT_SIZE));
	unsigned int gp2_irq = (1 << (timer->intcps_mapping_id % INTCPS_REGISTER_N_BIT_SIZE));
	*(mpuintc_mir_clearn) = gp2_irq;

	logger_log_register("INTPCS_MIR after unmasking %s: ", mpuintc_mirn);
}

