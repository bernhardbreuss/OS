/*
 * test_runner.c
 *
 *  Created on: 20.03.2013
 *      Author: edi
 */

#include "process_manager_test.h"
#include "../service/logger/logger.h"

void tests_run(void) {

	int testResult = -1;

	testResult = test_process_manager_1();
	if (testResult != 1) {
//		char doSomething[7] = "failed";
		logger_error("failed!\n");
	}
}

