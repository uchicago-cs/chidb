#ifndef __CONDITION_H_
#define __CONDITION_H_

#include "common.h"
#include "expression.h"

/*
data Condition_t = Eq Expression_t Expression
               | Lt Expression_t Expression
               | Gt Expression_t Expression
               | And Condition_t Condition
               | Or Condition_t Condition
               | Not Condition
*/

typedef struct CondComp {
   Expression_t *expr1, *expr2;
} CondComp;

typedef struct CondBinary {
   Condition_t *cond1, *cond2;
} CondBinary;

typedef struct CondUnary {
   Condition_t *cond;
} CondUnary;

/* note: we might want to also let CondIn hold SELECT queries. */
typedef struct CondIn {
   Expression_t *expr;
   Literal_t *values_list;
} CondIn;

enum CondType {
   RA_COND_EQ,
   RA_COND_LT,
   RA_COND_GT,
   RA_COND_LEQ,
   RA_COND_GEQ,
   RA_COND_AND,
   RA_COND_OR,
   RA_COND_NOT,
   RA_COND_IN,
};


struct Condition_s {
   enum CondType t;
   union {
      CondComp comp;
      CondBinary binary;
      CondUnary unary;
      CondIn in;
   } cond;
};

Condition_t *Eq(Expression_t *expr1, Expression_t *expr2);
Condition_t *Lt(Expression_t *expr1, Expression_t *expr2);
Condition_t *Gt(Expression_t *expr1, Expression_t *expr2);
Condition_t *Leq(Expression_t *expr1, Expression_t *expr2);
Condition_t *Geq(Expression_t *expr1, Expression_t *expr2);
Condition_t *And(Condition_t *cond1, Condition_t *cond2);
Condition_t *Or(Condition_t *cond1, Condition_t *cond2);
Condition_t *Not(Condition_t *cond);
Condition_t *In(Expression_t *expr, Literal_t *values_list);

void Condition_free(Condition_t *cond);
void Condition_print(Condition_t *cond);

#endif
