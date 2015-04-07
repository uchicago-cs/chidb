#ifndef __DELETE_H_
#define __DELETE_H_

#include "common.h"

typedef struct Delete_s {
   char *table_name;
   Condition_t *where;
} Delete_t;

Delete_t *Delete_make(const char *table_name, Condition_t *where);
void deleteDelete(Delete_t *del);
void Delete_print(Delete_t *del);
void Delete_free(Delete_t *del);

#endif
