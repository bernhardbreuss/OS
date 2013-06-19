/*
 * argument_helper.c
 *
 *  Created on: Jun 18, 2013
 *      Author: Patrick
 */

#include "argument_helper.h"

int argument_helper_parse(void) {
	char* argv = (char*)&ARGS_ADDR;
	char** argc_pointers = (char**)&ARGV_ADDR;
	int argc = 0;
	int current = 0;
	int runner = 0;
	int is_arg = 0;

	for (; current < ARGUMENTS_MAX_LENGTH; current++) {

		//ignore spaces, '"'
		while((is_arg == 0 && argv[runner] == ' ') || argv[runner] == '"'){
			runner++;
		}

		//end
		if (argv[runner] == '\0') {
			argv[current] = argv[runner];
			return argc;
		}

		//open arg
		if (argv[runner] != ' ' && is_arg == 0) {
			is_arg = 1;
			argc_pointers[argc] = &argv[current];
			argv[current] = argv[runner];
			argc++;
		}

		//close arg
		else if (argv[runner] == ' ' && is_arg == 1) {
			argv[current] = '\0';
			is_arg = 0;
		}

		//continue arg
		else if (argv[runner] != ' ' && is_arg == 1) {
			argv[current] = argv[runner];
		}

		runner++;
	}

	argv[current - 1] = '\0';
	return argc;
}
