/*
 * intcps.c
 *
 *  Created on: 03.05.2013
 *      Author: edi
 */

#include "intcps.h"
#include <bit.h>

void intcps_active_interrupt_line(int interrupt_line_id) {

	// activate the specific interrupt mask
	unsigned int* mpuintc_mir_clearn =
			(unsigned int*)(INTCPS_MIR_CLEARn +
					( ( interrupt_line_id / INTCPS_REGISTER_N_BIT_SIZE)*INTCPS_REGISTER_N_BIT_SIZE ) );
	unsigned int gp2_irq = ( 1 << ( interrupt_line_id % INTCPS_REGISTER_N_BIT_SIZE ) );
	*(mpuintc_mir_clearn) = gp2_irq;

}

void intcps_enable_new_irq_generation(void) {
	*((unsigned int*)(INTCPS_CONTROL)) |= BIT0;
}
