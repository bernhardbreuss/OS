/*
 * main.c
 *
 *  Created on: 19.03.2013
 *      Author: edi
 */

#include "kernel/process_manager.h"
#include "config.h"
#include "tests/test_runner.h"
#include "service/logger/logger.h"

#define TRUE 1

void main(void) {
	
	tests_run();
	
	int doIt = 0;
	if (doIt == TRUE) {
		logger_init();
		logger_debug("This is a test message");
	}
}
