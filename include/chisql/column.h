#ifndef __COLUMN_H_
#define __COLUMN_H_

#include "common.h"
#include "literal.h"

enum constraint_type {
   CONS_NOT_NULL,
   CONS_UNIQUE,
   CONS_PRIMARY_KEY,
   CONS_FOREIGN_KEY,
   CONS_DEFAULT,
   CONS_AUTO_INCREMENT,
   CONS_CHECK,
   CONS_SIZE
};

typedef struct ForeignKeyRef_t {
   const char *col_name, *table_name, *table_col_name;
} ForeignKeyRef_t;

typedef struct Constraint_t {
   enum constraint_type t;
   union {
      ForeignKeyRef_t ref;
      Literal_t *default_val;
      unsigned size;
      Condition_t *check;
   } constraint;
   struct Constraint_t *next;
} Constraint_t;

typedef struct Column_t {
   char *name;
   enum data_type type;
   Constraint_t *constraints;
   size_t offset; /* offset in bytes from the beginning of the row */
   struct Column_t *next;
} Column_t;

typedef struct ColumnReference_t {
   char *tableName, *columnName, *columnAlias;
} ColumnReference_t;

/* constraints on single columns */
ForeignKeyRef_t ForeignKeyRef_makeFull(const char *cname, ForeignKeyRef_t fkey);
ForeignKeyRef_t ForeignKeyRef_make(const char *foreign_tname,
                                      const char *foreign_cname);

Constraint_t *NotNull(void);
Constraint_t *AutoIncrement(void);
Constraint_t *PrimaryKey(void);
Constraint_t *ForeignKey(ForeignKeyRef_t fkr);
Constraint_t *Default(Literal_t *val);
Constraint_t *Unique(void);
Constraint_t *Check(Condition_t *cond);
Constraint_t *ColumnSize(unsigned size);
Constraint_t *Constraint_append(Constraint_t *constraints, Constraint_t *constraint);
Column_t *Column_addConstraint(Column_t *column, Constraint_t *constraints);
Column_t *Column(const char *name, enum data_type type, Constraint_t *constraints);
Column_t *Column_append(Column_t *columns, Column_t *column);

ColumnReference_t *ColumnReference_make(const char *, const char *);

int Column_compareByName(const void *col1, const void *col2);
void *Column_copy(void *col);

void Column_getOffsets(Column_t *cols);
size_t Column_getSize(Column_t *col);

void Constraint_print(void *constraint);
void Constraint_printList(Constraint_t *constraints);
void Column_freeList(Column_t *column);

/* sets the size of the next column */
void Column_setSize(ssize_t size);

#endif
