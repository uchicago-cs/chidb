#include "common.h"
#include "column.h"

typedef struct DBTable_s {
   
} DBTable_t;

typedef struct Row_s {
   size_t size, num_cols;
   size_t *offsets;
   void *data;
} Row_t;

Row_t *Row_makeFirst(Column_t *cols);
Row_t *Row_make(Row_t *row);

void Row_insertData(Row_t *row, ssize_t *cols, void **data);

int Row_matchByInt(Row_t *row, int i);
int Row_matchByChar(Row_t *row, char c);
int Row_matchByDouble(Row_t *row, double d);
int Row_matchByString(Row_t *row, const char *str);
