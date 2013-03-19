/*
 * main.c
 *
 *  Created on: 19.03.2013
 *      Author: edi
 */

#include <inttypes.h>
#include "kernel/ps/ps.h"
#include "config.h"

uint32_t foo(void) {
	char foo[3]=  "foo";
}

int main(void) {

	Process p1;
	p1.pid = 1;
	p1.priority = HIGH;
	p1.state = READY;
	p1.func = foo;
	p1.func();

	return 0;
}
