#ifndef __EXPRESSION_H_
#define __EXPRESSION_H_

#include "common.h"
#include "literal.h"
#include "column.h"
/*

data Expression_t = Term String
                | Plus Expression_t Expression
                | Minus Expression_t Expression
                | Multiply Expression_t Expression
                | Divide Expression_t Expression
                | Concat Expression_t Expression
                | Neg Expression

*/

typedef struct Expression_s Expression_t;

enum TermType {
   TERM_LITERAL,
   TERM_ID,
   TERM_NULL,
   TERM_COLREF,
   TERM_FUNC
};

enum FuncType { 
   FUNC_MAX, 
   FUNC_MIN, 
   FUNC_COUNT, 
   FUNC_AVG, 
   FUNC_SUM 
};

typedef struct Func {
   enum FuncType t;
   Expression_t *expr;
} Func;

typedef struct ExprTerm {
   enum TermType t;
   union {
      char *id;
      Literal_t *val;
      ColumnReference_t *ref;
      Func f;
   };
} ExprTerm;

typedef struct ExprBinary {
   Expression_t *expr1, *expr2;
} ExprBinary;

typedef struct ExprUnary {
   Expression_t *expr;
} ExprUnary;

enum ExprType {
   EXPR_TERM,
   EXPR_PLUS,
   EXPR_MINUS,
   EXPR_MULTIPLY,
   EXPR_DIVIDE,
   EXPR_CONCAT,
   EXPR_NEG
};

struct Expression_s {
   enum ExprType t;
   union {
      ExprTerm term;
      ExprBinary binary;
      ExprUnary unary;
   } expr;
   char *alias;
   struct Expression_s *next;
};


Expression_t *Term(const char *str);
Expression_t *TermLiteral(Literal_t *val);
Expression_t *TermNull(void);
Expression_t *TermColumnReference(ColumnReference_t *ref);
Expression_t *TermFunction(int type, Expression_t *expr);


Expression_t *Plus(Expression_t *, Expression_t *);
Expression_t *Minus(Expression_t *, Expression_t *);
Expression_t *Multiply(Expression_t *, Expression_t *);
Expression_t *Divide(Expression_t *, Expression_t *);
Expression_t *Concat(Expression_t *, Expression_t *);
Expression_t *Neg(Expression_t *);

Expression_t *append_expression(Expression_t *expr_list, Expression_t *expr);
Expression_t *add_alias(Expression_t *expr, const char *alias);
void Expression_print(Expression_t *);
void Expression_printList(Expression_t *);

char *Expression_toString(Expression_t *);

void Expression_free(Expression_t *expr);
void Expression_freeList(Expression_t *);

#endif
