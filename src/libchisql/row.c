#include "row.h"

Row_t *Row_makeFirst(Column_t *cols)
{
    Row_t *row = (Row_t *)calloc(1, sizeof(Row_t));
    Column_t *c = cols;
    while (c)
    {
        row->size += Column_getSize(c);
        row->num_cols++;
        row->
        c = c->next;
    }

}

Row_t *Row_make(Row_t *row);

void Row_insertData(Row_t *row, ssize_t *cols, void **data);

int Row_matchByInt(Row_t *row, int i);
int Row_matchByChar(Row_t *row, char c);
int Row_matchByDouble(Row_t *row, double d);
int Row_matchByString(Row_t *row, const char *str);
