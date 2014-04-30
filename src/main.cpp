#include <stdio.h>
#include <stdlib.h>
extern "C" {
#include "mpc.h"
}
#include <boost/variant.hpp>
#include <boost/lambda/construct.hpp>
#include <boost/variant/recursive_wrapper.hpp>
#include <readline/readline.h>
#include <readline/history.h>
#include <string>
#include <cstdint>
#include <functional>
#include <memory>
#include <iterator>
#include <algorithm>
#include <cassert>

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

class LispError {
public:
	enum ERROR_TYPE {
		LERR_DIV_ZERO, LERR_BAD_OP, LERR_BAD_NUM
	};
	ERROR_TYPE d_errCode;
	std::string d_errStr;

	LispError(ERROR_TYPE errType, const std::string& errStr) : d_errCode(errType), d_errStr(errStr) {}

};

class LispSExpr;

typedef boost::make_recursive_variant<LispError, int64_t, LispSExpr, std::vector<boost::recursive_variant_> >::type LispResultType;
typedef std::shared_ptr<LispResultType> LispResultPtr;

class LispSExpr {
public:
	std::string d_symbol;
	LispSExpr(const std::string& symbol) : d_symbol(symbol) {}
};

typedef std::vector<LispResultType> LispSExprVec;

LispResultType newLRTInt(int64_t num) {
	return LispResultType(num);
}

LispResultType newLRTError(LispError::ERROR_TYPE errType, const std::string& errStr) {
	return LispResultType(LispError(errType,errStr));
}

LispResultType newLRTSym(const std::string& symbol) {
	return LispSExpr(symbol);
}

LispResultType getNum(mpc_ast_t* t) {
	int errno;
	int64_t x = strtol(t->contents, NULL, 10);
	return errno != ERANGE ? newLRTInt(x) : newLRTError(LispError::LERR_BAD_NUM, "Bad number parsed");
}

LispResultType getLispResultAst(mpc_ast_t* t) {
	/* If Symbol or Number return conversion to that type */
	if (strstr(t->tag, "number")) {
		return getNum(t);
	}
	if (strstr(t->tag, "symbol")) {
		return newLRTSym(t->contents);
	}

	/* If root (>) or sexpr then create empty list */
	LispResultType x = LispSExprVec();
	//if (strcmp(t->tag, ">") == 0) { x = LispSExprVec(); }
	//if (strstr(t->tag, "sexpr"))  { x = LispSExprVec(); }
	/* Fill this list with any valid expression contained within */
	for (int i = 0; i < t->children_num; i++) {
		if (strcmp(t->children[i]->contents, "(") == 0) {
			continue;
		}
		if (strcmp(t->children[i]->contents, ")") == 0) {
			continue;
		}
		if (strcmp(t->children[i]->contents, "}") == 0) {
			continue;
		}
		if (strcmp(t->children[i]->contents, "{") == 0) {
			continue;
		}
		if (strcmp(t->children[i]->tag, "regex") == 0) {
			continue;
		}
		boost::get<LispSExprVec>(x).push_back(getLispResultAst(t->children[i]));
	}

	return LispResultType(x);
}

class LispResultPrinter : public boost::static_visitor<> {
	std::ostream& d_os;
public:
	LispResultPrinter(std::ostream& os) : d_os(os) {}

	void operator()(int64_t i) const {
		d_os << " " << i;
	}

	void operator()(LispError err) const {
		d_os << " [" << err.d_errCode << ". " << err.d_errStr;
	}

	void operator()(LispSExpr sExpr) const {
		d_os << " " << sExpr.d_symbol;
	}

	void operator()(LispSExprVec sExprVec) {
		for(LispSExprVec::iterator it = sExprVec.begin(); it != sExprVec.end(); it++) {
			if (it->which() == 3)
				this->d_os << " (";
			boost::apply_visitor(*const_cast<LispResultPrinter*>(this), *it);
			if (it->which() == 3)
				this->d_os << ")";
		}
	}
};


/* Use operator string to see which operation to perform */
LispResultType eval_op(LispResultType& x, char* op, const LispResultType& y) {
	if (x.which() == 0) {
		return x;
	} else if (y.which() == 0) {
		return y;
	} else {
		if (strcmp(op, "+") == 0) {
			return boost::get<int64_t>(x) + boost::get<int64_t>(y);
		}
		if (strcmp(op, "-") == 0) {
			return boost::get<int64_t>(x) - boost::get<int64_t>(y);
		}
		if (strcmp(op, "*") == 0) {
			return boost::get<int64_t>(x) * boost::get<int64_t>(y);
		}
		if (strcmp(op, "/") == 0) {
			if ( boost::get<int64_t>(y) == 0) {
				return LispResultType(LispError(LispError::LERR_DIV_ZERO,"Division by zero error"));
			} else {
				return boost::get<int64_t>(x) / boost::get<int64_t>(y);
			}
		}
	}
	return x;
}

LispResultType eval(mpc_ast_t* t) {
	LispResultType res = getLispResultAst(t);
	LispResultPrinter printer(std::cout);
	std::cout << "(";
	boost::apply_visitor( printer, res);
	std::cout << ")\n";
	return res;
}

int main(int argc, char** argv) {

	MpcParser Number("number");
	MpcParser Symbol("symbol");
	MpcParser Sexpr("sexpr");
	MpcParser Expr("expr");
	MpcParser Lispy("lispy");

	mpca_lang(MPCA_LANG_DEFAULT,
	  "                                          \
	    number : /-?[0-9]+/ ;                    \
	    symbol : '+' | '-' | '*' | '/' ;         \
	    sexpr  : '(' <expr>* ')' ;               \
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
			LispResultType result(0);
			result = eval((mpc_ast_t*)r.output);
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
