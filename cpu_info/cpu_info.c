/*
 * cpu_info.c
 *
 *  Created on: 20.06.2013
 *      Author: lumannnn
 */

#include <stdio.h>
#include <os_stdio.h>
#include <mem_io.h>
#include "cpu_info.h"

static int _sysclk;
static unsigned int _core_dpll_mult;
static unsigned int _core_dpll_div;
static int _clkoutx2;

int main(int argc, char* argv[]) {
	printf("CPU info\n");
	printf("========\n");
	if (cpu_info_calk_clkoutX2() == 1) {
		printf("SystemClock:\t%u", _sysclk);
		printf("CLKOUT x2:\t\t%i Hz", _clkoutx2);
		printf("CLKOUT:\t\t%i Hz", _clkoutx2 / 2);
		printf("CORE_DPLL_MULT\t%u", _core_dpll_mult);
		printf("CORE_DPLL_DIV\t%u", _core_dpll_div);
	} else {
		printf("no info available.");
	}

	return 0;
}


int cpu_info_calk_clkoutX2() {
	unsigned int addresses[2];
	addresses[0] = CM_CLKSEL1_PLL;
	addresses[1] = PRM_CLKSEL;

	if (memory_mapped_read(addresses, 2) != 0) {
		return -1;
	}

	_core_dpll_mult = 	((addresses[0] & (0x7FF << 16)) >> 16);	// clksel1_pll[26:16]
	_core_dpll_div = 	((addresses[0] & (0x7F << 8)) >> 8);	// clksel1_pll[14:8]

	_sysclk = 0;
	switch (addresses[1] & (7)) {	// PRM_CLKSEL[2:0]
		case 0x0:
			_sysclk = 12000000;
			break;
		case 0x1:
			_sysclk = 13000000;
			break;
		case 0x2:
			_sysclk = 19200000;
			break;
		case 0x3:
			_sysclk = 26000000;
			break;
		case 0x4:
			_sysclk = 38400000;
			break;
		case 05:
			_sysclk = 16800000;
			break;
	}

	_clkoutx2 = (_sysclk) * 2 * _core_dpll_mult / (_core_dpll_div + 1);
	return 1;
}
