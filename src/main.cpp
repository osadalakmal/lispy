#include <stdio.h>
#include <stdlib.h>
extern "C" {
	#include "mpc.h"
}
#include <readline/readline.h>
#include <readline/history.h>
#include <string>
#include <functional>
#include <memory>

class MpcParser {
	std::shared_ptr<mpc_parser_t> d_parser;
public:
	MpcParser(const std::string& name) : d_parser(mpc_new(name.c_str()), std::bind(&mpc_cleanup,1,std::placeholders::_1)) { }
};

int main(int argc, char** argv) {

	/* Create Some Parsers */
	mpc_parser_t* Number = mpc_new("number");
	mpc_parser_t* Operator = mpc_new("operator");
	mpc_parser_t* Expr = mpc_new("expr");
	mpc_parser_t* Lispy = mpc_new("lispy");

	/* Define them with the following Language */
	mpca_lang(MPCA_LANG_DEFAULT,
			"                                                     \
	    number   : /-?[0-9]+/ ;                             \
	    operator : '+' | '-' | '*' | '/' ;                  \
	    expr     : <number> | '(' <operator> <expr>+ ')' ;  \
	    lispy    : /^/ <operator> <expr>+ /$/ ;             \
	  ",
	  Number, Operator, Expr, Lispy);

	/* Print Version and Exit Information */
	puts("Lispy Version 0.0.0.0.1");
	puts("Press Ctrl+c to Exit\n");

	/* In a never ending loop */
	while (1) {

		/* Output our prompt and get input */
		char* input = readline("lispy> ");

		/* Add input to history */
		add_history(input);

		/* Echo input back to user */
		printf("No you're a %s\n", input);

		/* Free retrived input */
		free(input);

	}

	/* Undefine and Delete our Parsers */
	mpc_cleanup(4, Number, Operator, Expr, Lispy);

	return 0;
}
