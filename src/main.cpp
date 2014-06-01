#include "lispy_elems.h"
#include "lispy_operators.h"

using namespace lispy;

class MpcParser {
	std::shared_ptr<mpc_parser_t> d_parser;
public:
	MpcParser(const std::string& name) :
			d_parser(mpc_new(name.c_str()), std::bind(&mpc_cleanup, 1, std::placeholders::_1)) {
	}
	mpc_parser_t* get() {
		return d_parser.get();
	}
};

int main(int argc, char** argv) {

	MpcParser Number("number");
	MpcParser Symbol("symbol");
	MpcParser Sexpr("sexpr");
	MpcParser Qexpr("qexpr");
	MpcParser Expr("expr");
	MpcParser Lispy("lispy");

	mpca_lang(MPCA_LANG_DEFAULT,
			"                                          \
	    number : /-?[0-9]+/ ;                    \
	    symbol : '+' | '-' | '*' | '/' ;         \
	    sexpr  : '(' <expr>* ')' ;               \
	    qexpr  : '{' <expr>* '}' ;               \
	    expr   : <number> | <symbol> | <sexpr> | <qexpr> ; \
	    lispy  : /^/ <expr>* /$/ ;               \
	  ",
			Number.get(), Symbol.get(), Sexpr.get(), Qexpr.get(), Expr.get(), Lispy.get());

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
			lispy::OperatorEval opEvaluator;
			LispResultType resultTemp = getLispResultAst((mpc_ast_t*) r.output);
			LispResultType result = boost::apply_visitor(opEvaluator, resultTemp);
			LispResultPrinter printer(std::cout);
			boost::apply_visitor(printer, result);
			std::cout << "\n";
			mpc_ast_delete((mpc_ast_t*) r.output);
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
