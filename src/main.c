/*
 * main.c
 *
 *  Created on: 19.03.2013
 *      Author: edi
 */

#include "kernel/process_manager.h"
#include "config.h"
#include "tests/test_runner.h"

ProcessManager _processManager;

int main(void) {

	tests_run();
	return 0;
}
