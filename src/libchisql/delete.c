#include <chisql/chisql.h>

Delete_t *Delete_make(const char *table_name, Condition_t *where)
{
    Delete_t *new_free = (Delete_t *)calloc(1, sizeof(Delete_t));
    new_free->table_name = strdup(table_name);
    new_free->where = where;
    return new_free;
}

void deleteDelete(Delete_t *del)
{
    Condition_free(del->where);
    free(del->table_name);
    free(del);
}

void Delete_print(Delete_t *del)
{
    printf("Delete from %s where ", del->table_name);
    Condition_print(del->where);
    puts("");
}

void Delete_free(Delete_t *del)
{
    if (!del)
    {
        fprintf(stderr, "Warning: Delete_free called on null pointer\n");
        return;
    }
    free(del->table_name);
    if (del->where)
        Condition_free(del->where);
    free(del);
}
