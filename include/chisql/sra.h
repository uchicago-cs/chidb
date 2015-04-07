#ifndef __SRA_H_
#define __SRA_H_

#include "common.h"
#include "expression.h"
#include "create.h"
#include "condition.h"
#include "ra.h"

/*
SQL:
select f.a as Col1, g.a as Col2 from Foo f, Foo g where Col1 != Col2;

--> To SRA:
Pi([(f,a,Col1), (g,a,Col2)],
   Sigma(Col1 != Col2,
      Join([(Foo,f), (Foo,g)])
   )
)

--> To RA:
Pi([Col1, Col2],
   Sigma(Col1 != Col2,
      Cross(
         Rho(Foo, f, [Col1]),
         Rho(Foo, g, [Col2])
      )
   )
)
*/

/*
data SRA = Table TableReference
         | Project SRA [Expression] 
         | Select SRA Condition 
         | NaturalJoin [SRA]
         | Join [SRA] (Maybe JoinCondition)
         | OuterJoin [SRA] OJType (Maybe JoinCondition)
         | Union SRA SRA
         | Except SRA SRA
         | Intersect SRA SRA

data OJType = Left 
            | Right 
            | Full

data ColumnReference = ColumnReference (Maybe String) String
data TableReference = TableName String (Maybe String)
data JoinCondition = On Condition
                   | Using [String]

*/

/* Forward declarations */
typedef struct SRA_s SRA_t;
typedef struct JoinCondition_s JoinCondition_t;

enum SRAType {
   SRA_TABLE,
   SRA_PROJECT,
   SRA_SELECT,
   SRA_NATURAL_JOIN,
   SRA_JOIN,
   SRA_FULL_OUTER_JOIN,
   SRA_LEFT_OUTER_JOIN,
   SRA_RIGHT_OUTER_JOIN,
   SRA_UNION,
   SRA_EXCEPT,
   SRA_INTERSECT
};

enum OJType { OJ_LEFT, OJ_RIGHT, OJ_FULL };
enum OrderBy {ORDER_BY_ASC, ORDER_BY_DESC};

typedef struct SRA_Table_s {
   TableReference_t *ref; /* TableReference_t defined in create.h */
} SRA_Table_t;

typedef struct SRA_Project_s {
   SRA_t *sra;
   Expression_t *expr_list;
   Expression_t *order_by;
   int distinct;
   enum OrderBy asc_desc;
   Expression_t *group_by;
} SRA_Project_t;

typedef struct SRA_Select_s {
   SRA_t *sra;
   Condition_t *cond;
} SRA_Select_t;

typedef struct SRA_Join_s {
   SRA_t *sra1, *sra2;
   JoinCondition_t *opt_cond;
} SRA_Join_t;

typedef struct SRA_Binary_s {
   SRA_t *sra1, *sra2;
} SRA_Binary_t;

struct SRA_s {
   enum SRAType t;
   union {
      SRA_Table_t table;
      SRA_Project_t project;
      SRA_Select_t select;
      SRA_Join_t join;
      SRA_Binary_t binary;
   };
};

typedef struct SRAList_s {
   SRA_t *sra;
   struct SRAList_s *next;
} SRAList_t;

enum JoinConditionType {
   JOIN_COND_ON, JOIN_COND_USING
};

struct JoinCondition_s {
   enum JoinConditionType t;
   union {
      Condition_t *on;
      StrList_t *col_list;
   };
};

typedef struct ProjectOption_s {
   Expression_t *order_by, *group_by;
   enum OrderBy asc_desc; /* not used by group by */
} ProjectOption_t;

SRA_t *SRATable(TableReference_t *ref);
SRA_t *SRAProject(SRA_t *sra, Expression_t *expr_list);
SRA_t *SRASelect(SRA_t *sra, Condition_t *cond);
SRA_t *SRANaturalJoin(SRA_t *sra1, SRA_t *sra2);
SRA_t *SRAJoin(SRA_t *sra1, SRA_t *sra2, JoinCondition_t *cond);
SRA_t *SRALeftOuterJoin(SRA_t *sra1, SRA_t *sra2, JoinCondition_t *cond);
SRA_t *SRARightOuterJoin(SRA_t *sra1, SRA_t *sra2, JoinCondition_t *cond);
SRA_t *SRAFullOuterJoin(SRA_t *sra1, SRA_t *sra2, JoinCondition_t *cond);
SRA_t *SRAUnion(SRA_t *sra1, SRA_t *sra2);
SRA_t *SRAExcept(SRA_t *sra1, SRA_t *sra2);
SRA_t *SRAIntersect(SRA_t *sra1, SRA_t *sra2);

/* the folloing two only work on SRAProject */
SRA_t *SRA_applyOption(SRA_t *sra, ProjectOption_t *option);
SRA_t *SRA_makeDistinct(SRA_t *sra);

ProjectOption_t *OrderBy_make(Expression_t *expr, enum OrderBy o);
ProjectOption_t *GroupBy_make(Expression_t *expr);
ProjectOption_t *ProjectOption_combine(ProjectOption_t *order_by, 
                                        ProjectOption_t *group_by);
void ProjectOption_print(ProjectOption_t *sra);

JoinCondition_t *On(Condition_t *cond);
JoinCondition_t *Using(StrList_t *col_list);

void SRA_free(SRA_t *sra);

void SRA_print(SRA_t *sra);
void JoinCondition_print(JoinCondition_t *cond);
void JoinCondition_free(JoinCondition_t *cond);

RA_t *SRA_desugar(SRA_t *sra);

#endif
