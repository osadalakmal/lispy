#ifndef INCLUDED_OPERATOR_EVAL_INCLUDED_H
#define INCLUDED_OPERATOR_EVAL_INCLUDED_H

#include "boost/variant/apply_visitor.hpp"
#include "lispy_elems.h"
#include "stdint.h"

namespace lispy {

class OperatorEval : public boost::static_visitor<LispResultType> {
  public:
    OperatorEval() = default;
    LispResultType operator()(LispResultType& qExpr) { return LispResultType(qExpr); }
    LispResultType operator()(int64_t& longNumber) { return LispResultType(longNumber); } 
    LispResultType operator()(LispSymbol& symbol) { return LispResultType(symbol); }
    LispResultType operator()(LispQExpression& qExpr) { return LispResultType(qExpr); }
    LispResultType operator()(LispSExpression& sExpr) { return LispResultType(sExpr); }
    LispResultType operator()(LispError& error) { return LispResultType(error); }

};

}

#endif
