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
	mpc_parser_t* get() { return d_parser.get(); }
};

int main(int argc, char** argv) {

	/* Create Some Parsers */
	MpcParser Number("number");
	MpcParser RealNum("real_num");
	MpcParser Operator("operator");
	MpcParser Expr("expr");
	MpcParser Lispy("lispy");

	/* Define them with the following Language */
	mpca_lang(MPCA_LANG_DEFAULT,
			"                                               \
		real_num : /-?[0-9]*\\.[0-9]+/ ;                    \
	    number   : /-?[0-9]+/ ;                             \
	    operator : '+' | '-' | '*' | '/' | '%' ;            \
	    expr     : <real_num> | <number> | '(' <operator> <expr>+ ')' ;  \
	    lispy    : /^/ <operator> <expr>+ /$/ ;             \
	  ",
	  RealNum.get(), Number.get(), Operator.get(), Expr.get(), Lispy.get());

	/* Print Version and Exit Information */
	puts("Lispy Version 0.0.0.0.1");
	puts("Press Ctrl+c to Exit\n");

	/* In a never ending loop */
	while (1) {

		/* Output our prompt and get input */
		char* input = readline("lispy> ");

		/* Add input to history */
		add_history(input);

		if (!input)
			break;

		mpc_result_t r;
		if (mpc_parse("<stdin>", input, Lispy.get(), &r)) {
		  /* On Success Print the AST */
		  mpc_ast_print((mpc_ast_t*)r.output);
		  mpc_ast_delete((mpc_ast_t*)r.output);
		} else {
		  /* Otherwise Print the Error */
		  mpc_err_print(r.error);
		  mpc_err_delete(r.error);
		}

		/* Free retrived input */
		free(input);

	}

	return 0;
}
