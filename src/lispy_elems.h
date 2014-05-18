/*
 * lsipy_elems.h
 *
 *  Created on: May 11, 2014
 *      Author: osada
 */
#ifndef LSIPY_ELEMS_H_
#define LSIPY_ELEMS_H_

#include <stdio.h>
extern "C" {
#include "mpc.h"
}
#include <boost/variant.hpp>
#include <boost/lambda/construct.hpp>
#include <boost/variant/recursive_wrapper.hpp>
#include <readline/readline.h>
#include <readline/history.h>
#include <deque>
#include <string>
#include <cstdint>
#include <functional>
#include <memory>
#include <iterator>
#include <algorithm>
#include <cassert>
#include <stdio.h>
#include <stdlib.h>

namespace lispy {

class LispError {
public:
	enum ERROR_TYPE {
		LERR_DIV_ZERO, LERR_BAD_OP, LERR_BAD_NUM, LERR_BAD_SYNTAX
	};
	ERROR_TYPE d_errCode;
	std::string d_errStr;

	LispError(ERROR_TYPE errType, const std::string& errStr) : d_errCode(errType), d_errStr(errStr) {}
};

class LispSymbol {
public:
	std::string d_symbol;
	LispSymbol(const std::string& symbol) : d_symbol(symbol) {}
};

template <typename TYPE>
class LispSExpr {
public:
	std::deque<TYPE> d_data;
};

template <typename TYPE>
class LispQExpr {
public:
	std::deque<TYPE> d_data;
};

typedef boost::make_recursive_variant<LispError, int64_t, LispSymbol, LispSExpr<boost::recursive_variant_ >, LispQExpr<boost::recursive_variant_ > >::type LispResultType;
typedef std::shared_ptr<LispResultType> LispResultPtr;
typedef LispSExpr<LispResultType> LispSExpression;

LispResultType newLRTInt(int64_t num);
LispResultType newLRTError(LispError::ERROR_TYPE errType, const std::string& errStr);
LispResultType newLRTSym(const std::string& symbol);
LispResultType getNum(mpc_ast_t* t);
LispResultType getLispResultAst(mpc_ast_t* t);

class LispResultPrinter : public boost::static_visitor<> {
	std::ostream& d_os;
public:
	LispResultPrinter(std::ostream& os);
	void operator()(int64_t i) const;
	void operator()(LispError err) const;
	void operator()(LispSymbol symbol) const;
	void operator()(LispSExpr<LispResultType> sExpr);
	void operator()(LispQExpr<LispResultType> qExpr);
};

}

#endif /* LSIPY_ELEMS_H_ */
