#ifndef __RA_H_
#define __RA_H_

#include "common.h"
#include "condition.h"
#include "column.h"

/*
RA_t in Haskell
data RA_t = Table String
        | Select Expression_t RA_t -- see below for Expression_t def
        | Project [String] RA
        | Union RA_t RA
        | Difference RA_t RA
        | Cross RA_t RA
        | Rename String [String] RA
*/

enum RA_Type {
   RA_TABLE, 
   RA_SIGMA, 
   RA_PI, 
   RA_UNION, 
   RA_DIFFERENCE, 
   RA_CROSS, 
   RA_RHO_TABLE,
   RA_RHO_EXPR,
};

typedef struct RA_s RA_t;

struct RA_s {
   enum RA_Type t;
   union {
      struct { char *name; } table;
      struct { RA_t *ra; Condition_t *cond; } sigma;
      struct { RA_t *ra; Expression_t *expr_list; } pi;
      struct { RA_t *ra1, *ra2; } binary;
      struct { RA_t *ra; Expression_t *to_rename; char *new_name;} rho;
   };
   Column_t *columns;
};

void RA_print(RA_t *ra);

RA_t *RA_Table(const char *name);
RA_t *RA_Sigma(RA_t *ra, Condition_t *expr);
RA_t *RA_Pi(RA_t *ra, Expression_t *expr_list);
RA_t *RA_Union(RA_t *ra1, RA_t *ra2);
RA_t *RA_Difference(RA_t *ra1, RA_t *ra2);
RA_t *RA_Cross(RA_t *ra1, RA_t *ra2);
RA_t *RA_RhoTable(RA_t *ra, const char *new_name);
RA_t *RA_RhoExpr(RA_t *ra, Expression_t *expr, const char *new_name);

void RA_free(RA_t *ra);

#endif
