/*
 * lispy_elems.cpp
 *
 *  Created on: May 11, 2014
 *      Author: osada
 */
#include "lispy_elems.h"

namespace lispy {

LispResultType newLRTInt(int64_t num) {
	return LispResultType(num);
}

LispResultType newLRTError(LispError::ERROR_TYPE errType, const std::string& errStr) {
	return LispResultType(LispError(errType,errStr));
}

LispResultType newLRTSym(const std::string& symbol) {
	return LispSymbol(symbol);
}

LispResultType newLRTSexpr() {
	return LispSExpr<LispResultType>();
}

LispResultType newLRTQexpr() {
	return LispQExpr<LispResultType>();
}

LispResultType getNum(mpc_ast_t* t) {
	int64_t x = strtol(t->contents, NULL, 10);
	return ( errno != ERANGE ? newLRTInt(x) : newLRTError(LispError::LERR_BAD_NUM, "Bad number parsed"));
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
	LispResultType x = LispSExpr<LispResultType>();
	for (int i = 0; i < t->children_num; i++) {
		if (strcmp(t->children[i]->tag,"qexpr|>") == 0) {
			x = LispQExpr<LispResultType>();
		}
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
			x = LispQExpr<LispResultType>();
			continue;
		}
		if (strcmp(t->children[i]->tag, "regex") == 0) {
			continue;
		}
		if (x.which() == 3) {
			boost::get<LispSExpr<LispResultType> >(x).d_data.push_back(getLispResultAst(t->children[i]));
		} else if (x.which() == 4) {
			boost::get<LispQExpr<LispResultType> >(x).d_data.push_back(getLispResultAst(t->children[i]));
		}
	}

	return LispResultType(x);
}

LispResultPrinter::LispResultPrinter(std::ostream& os) : d_os(os) {}

void LispResultPrinter::operator()(int64_t i) const {
	d_os << " " << i << " ";
}

void LispResultPrinter::operator()(LispError err) const {
	d_os << " [" << err.d_errCode << ". " << err.d_errStr;
}

void LispResultPrinter::operator()(LispSymbol sExpr) const {
	d_os << " " << sExpr.d_symbol << " ";
}

void LispResultPrinter::operator()(LispSExpr<LispResultType> sExpr) {
	this->d_os << " (";
	for(auto it = sExpr.d_data.begin(); it != sExpr.d_data.end(); it++) {
		boost::apply_visitor(*const_cast<LispResultPrinter*>(this), *it);
	}
	this->d_os << ")";
}

void LispResultPrinter::operator()(LispQExpr<LispResultType> qExpr) {
	this->d_os << " {";
	for(auto it = qExpr.d_data.begin(); it != qExpr.d_data.end(); it++) {
		boost::apply_visitor(*const_cast<LispResultPrinter*>(this), *it);
	}
	this->d_os << "}";
}

}

