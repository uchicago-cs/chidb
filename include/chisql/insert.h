#ifndef __INSERT_H_
#define __INSERT_H_ 

#include "common.h"
#include "ra.h"
#include "create.h"

typedef struct Insert_s {
   char *table_name;
   StrList_t *col_names;
   Literal_t *values;
} Insert_t;

Insert_t *Insert_make(const char *table_name, StrList_t *opt_col_names, Literal_t *values);
void Insert_print(Insert_t *insert);
void Insert_free(Insert_t *insert);

#endif
