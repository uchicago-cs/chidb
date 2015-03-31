#include <chisql/chisql.h>


static List_t tables;

static void print_table(void *table_ptr)
{
    Table_print((Table_t *)table_ptr);
}

void mock_db_init()
{
    puts("initializing...");
    fflush(stdout);
    list_init(&tables, Table_free);
    list_setPrintFunc(&tables, print_table);
    puts("done...");
    fflush(stdout);
}
void add_table(Table_t *table)
{
    list_addBack(&tables, table);
}

void remove_table(Table_t *table)
{
    list_removeByPointerFree(&tables, table);
}

static void toStringBuf (char *name, void *table)
{
    strcpy(name, ((Table_t *)table)->name);
}

/*
static char *toString (void *table) {
   return ((Table_t *)table)->name;
}
*/

Table_t *table_by_name(const char *name)
{
    return (Table_t *)list_findByString(&tables, toStringBuf, name);
}

void show_tables()
{
    list_print(&tables, false);
}

List_t column_list(Table_t *table)
{
    List_t res;
    Column_t *cols;
    cols = table->columns;
    /*printf("constructing column list for table %s\n", table_name);*/
    list_init(&res, NULL);
    list_setCompFunc(&res, Column_compareByName);
    while (cols)
    {
        /*printf("adding column '%s'\n", cols->name);*/
        list_addBack(&res, cols);
        cols = cols->next;
    }
    return res;
}

static void get_colname (char *name, void *col)
{
    strcpy(name, ((Column_t *)col)->name);
}

List_t columns_in_common(Table_t *table1, Table_t *table2)
{
    List_t cols1 = column_list(table1),
           cols2 = column_list(table2),
           res;
    ListNode_t *runner = cols1.front;
    list_init(&res, NULL);
    while (runner)
    {
        Column_t *col = (Column_t *)runner->data;
        /*printf("trying to find match for %s...\n", col->name); fflush(stdout);*/
        Column_t *other_col = (Column_t *)list_findByString(&cols2,
                              get_colname,
                              col->name);
        if (other_col)
        {
            /*printf("Found matching column names: %s\n", col->name);*/
            if (col->type == other_col->type)
            {
                /*printf("Types match, adding to result\n");*/
                list_addBack(&res, col);
            }
            else
            {
                /*printf("Types don't match; ignoring match\n");*/
            }
        }
        runner = runner->next;
    }
    return res;
}

List_t column_list_str(const char *table_name)
{
    Table_t *table = list_findByString(&tables, toStringBuf, table_name);
    List_t res;
    Column_t *cols;
    if (!table)
    {
        fprintf(stderr, "Error: table %s was not found\n", table_name);
        exit(1);
    }
    cols = table->columns;
    /*printf("constructing column list for table %s\n", table_name);*/
    list_init(&res, NULL);
    while (cols)
    {
        /*printf("adding column '%s'\n", cols->name);*/
        list_addBack(&res, cols);
        cols = cols->next;
    }
    return res;
}

List_t columns_in_common_str(const char *table1, const char *table2)
{
    List_t cols1 = column_list_str(table1),
           cols2 = column_list_str(table2),
           res;
    ListNode_t *runner = cols1.front;
    list_init(&res, NULL);
    while (runner)
    {
        Column_t *col = (Column_t *)runner->data;
        /*printf("trying to find match for %s...\n", col->name); fflush(stdout);*/
        Column_t *other_col = (Column_t *)list_findByString(&cols2,
                              get_colname,
                              col->name);
        if (other_col)
        {
            /*printf("Found matching column names: %s\n", col->name);*/
            if (col->type == other_col->type)
            {
                /*printf("Types match, adding to result\n");*/
                list_addBack(&res, col);
            }
            else
            {
                /*printf("Types don't match; ignoring match\n");*/
            }
        }
        runner = runner->next;
    }
    return res;
}
