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
	
	logger_init();

	tests_run();
	
	int doIt = 1;
	if (doIt == TRUE) {
//		logger_init();
		logger_error("This is a test message\n");
		logger_warn("And another message\n");
		logger_debug("Hello from BeagleBoard!\n");
		logger_debug("I'm so happy, now that I can speak to the world! :D\n");
	}
}
