#include "lispy_elems.h"

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

LispResultType builtin_op(LispResultType a, const char* op) {

	/* Ensure all arguments are numbers */
	auto resVec = boost::get<LispSExpression>(a).d_data;
	for (unsigned int i = 0; i < resVec.size(); i++) {
		if (resVec[i].which() != 1) {
			return LispError(LispError::LERR_BAD_SYNTAX, "Cannot operator on non number!");
		}
	}

	/* Pop the first element */
	LispResultType x = resVec.front();
	resVec.pop_front();

	/* If no arguments and sub then perform unary negation */
	if ((strcmp(op, "-") == 0) && resVec.size() == 0) {
		x = -(boost::get<int64_t>(x));
	}

	/* While there are still elements remaining */
	while (!resVec.empty()) {

		/* Pop the next element */
		LispResultType y = resVec.front();
		resVec.pop_front();

		/* Perform operation */
		if (strcmp(op, "+") == 0) {
			boost::get<int64_t>(x) += boost::get<int64_t>(y);
		}
		if (strcmp(op, "-") == 0) {
			boost::get<int64_t>(x) -= boost::get<int64_t>(y);
		}
		if (strcmp(op, "*") == 0) {
			boost::get<int64_t>(x) *= boost::get<int64_t>(y);
		}
		if (strcmp(op, "/") == 0) {
			if (boost::get<int64_t>(y) == 0) {
				x = LispError(LispError::LERR_DIV_ZERO, "Division By Zero!");
				break;
			}
			boost::get<int64_t>(x) /= boost::get<int64_t>(y);
		}
	}

	return x;
}

LispResultType eval_op(LispResultType x);

LispResultType evalSExpr(LispResultType res) {
	if (res.which() == 3) {
		return eval_op(res);
	} else {
		return res;
	}
}

/* Use operator string to see which operation to perform */
LispResultType eval_op(LispResultType x) {
	try {
		std::deque<LispResultType>& results = boost::get<LispSExpression>(x).d_data;
		if (results.empty()) {
			return x;
		}
		for (auto it = results.begin(); it != results.end(); it++) {
			*it = evalSExpr(*it);
			if ((*it).which() == 0) {
				return *it;
			}
		}
		if (results.size() == 1) {
			return results[0];
		}
		if (results.front().which() != 2) {
			results.pop_front();
			return LispError(LispError::LERR_BAD_SYNTAX, "S-expression does not start with a syymbol!");
		}
		LispResultType f = results.front();
		results.pop_front();
		return builtin_op(x, (boost::get<LispSymbol>(f)).d_symbol.c_str());

	} catch (const std::exception& ex) {
		return LispError(LispError::LERR_BAD_SYNTAX, "Bad Syntax!");
	}
}

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
	    sexpr  : '{' <expr>* '}' ;               \
	    expr   : <number> | <symbol> | <sexpr> ; \
	    lispy  : /^/ <expr>* /$/ ;               \
	  ",
			Number.get(), Symbol.get(), Sexpr.get(), Expr.get(), Lispy.get());

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
			LispResultType result = eval_op(getLispResultAst((mpc_ast_t*) r.output));
			LispResultPrinter printer(std::cout);
			std::cout << "(";
			boost::apply_visitor(printer, result);
			std::cout << ")\n";
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
