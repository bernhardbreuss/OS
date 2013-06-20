/*
 * argument_helper.c
 *
 *  Created on: Jun 18, 2013
 *      Author: Patrick
 */

#include "argument_helper.h"

/**
 * check special characters
 */
static int terminal_parser_ignore(char argv[], int runner, int current, int is_arg, int *is_quote) {

	//check \"
	if (argv[runner-1] == '\\' && argv[runner] == '"') {
			return 0;
	}

	//check "....", if yes open quote else close quote
	int i = runner;
	i++;
	if (argv[runner] == '"') {
		if (*is_quote == 0) {
			for (; i != 512; i++) {
				if (argv[i] == '"') {
					*is_quote = 1;
					break;
				}
			}

		} else {
			*is_quote = 0;
		}
	}

	if ((is_arg == 0 && argv[runner] == ' ') || argv[runner] == '"') {
		return 1;
	}
	return 0;

}

int argument_helper_parse(void) {
	char* argv = (char*)&ARGS_ADDR;
	char** argc_pointers = (char**)&ARGV_ADDR;
	int argc = 0;
	int current = 0;
	int runner = 0;
	int is_arg = 0;
	int is_quote = 0; //'"'

	for (; current < ARGUMENTS_MAX_LENGTH; current++) {

		//ignore
		while (terminal_parser_ignore(argv, runner, current, is_arg, &is_quote)) {
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
		else if (argv[runner] == ' ' && is_arg == 1 && is_quote == 0) {
			argv[current] = '\0';
			is_arg = 0;
		}

		//continue arg
		else if (argv[runner] != ' ' && is_arg == 1) {
			argv[current] = argv[runner];
		}

		//continue arg with quote
		else if (argv[runner] == ' ' && is_quote == 1) {
			argv[current] = argv[runner];
		}

		runner++;
	}

	argv[current - 1] = '\0';
	return argc;
}
