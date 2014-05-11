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

LispResultPrinter::LispResultPrinter(std::ostream& os) : d_os(os) {}

void LispResultPrinter::operator()(int64_t i) const {
	d_os << " " << i;
}

void LispResultPrinter::operator()(LispError err) const {
	d_os << " [" << err.d_errCode << ". " << err.d_errStr;
}

void LispResultPrinter::operator()(LispSymbol sExpr) const {
	d_os << " " << sExpr.d_symbol;
}

void LispResultPrinter::operator()(LispSExprVec sExprVec) {
	for(LispSExprVec::iterator it = sExprVec.begin(); it != sExprVec.end(); it++) {
		if (it->which() == 3)
			this->d_os << " (";
		boost::apply_visitor(*const_cast<LispResultPrinter*>(this), *it);
		if (it->which() == 3)
			this->d_os << ")";
	}
}

}

