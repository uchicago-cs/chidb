#include <chisql/chisql.h>

static Table_t *Table_addPrimaryKey(Table_t *table, const char *col_name);

Table_t *Table_addForeignKey(Table_t *table, ForeignKeyRef_t fkr)
{
    if (table != NULL)
    {
        /* find the column that matches the cname given, and add this reference */
        Column_t *col = table->columns;
        for (; col; col=col->next)
        {
            if (!strcmp(col->name, fkr.col_name))
            {
                Column_addConstraint(col, ForeignKey(fkr));
                return table;
            }
        }
        fprintf(stderr, "Error: column %s not in table\n", fkr.col_name);
        return table;
    }
    fprintf(stderr, "Error: table is null\n");
    return table;
}

Table_t *Table_addKeyDecs(Table_t *table, KeyDec_t *decs)
{
    StrList_t *slist;
    for (; decs; decs = decs->next)
    {
        switch (decs->t)
        {
        case KEY_DEC_PRIMARY:
            for (slist = decs->dec.primary_keys; slist; slist = slist->next)
            {
                if (!Table_addPrimaryKey(table, slist->str))
                    fprintf(stderr, "Error: column '%s' not found\n", slist->str);
            }
            break;
        case KEY_DEC_FOREIGN:
            if (!Table_addForeignKey(table, decs->dec.fkey))
                fprintf(stderr, "Error: column '%s' not found\n", decs->dec.fkey.col_name);
            break;
        default:
            fprintf(stderr, "Unknown declaration type\n");
        }
    }
    return table;
}

KeyDec_t *ForeignKeyDec(ForeignKeyRef_t fkr)
{
    KeyDec_t *kdec = (KeyDec_t *)calloc(1, sizeof(KeyDec_t));
    kdec->t = KEY_DEC_FOREIGN;
    kdec->dec.fkey = fkr;
    return kdec;
}
KeyDec_t *PrimaryKeyDec(StrList_t *col_names)
{
    KeyDec_t *kdec = (KeyDec_t *)calloc(1, sizeof(KeyDec_t));
    kdec->t = KEY_DEC_PRIMARY;
    kdec->dec.primary_keys = col_names;
    return kdec;
}

Table_t *Table_make(char *name, Column_t *columns, KeyDec_t *decs)
{
    Table_t *new_table = (Table_t *)calloc(1, sizeof(Table_t));
    new_table->name = name;
    new_table->columns = columns;
    Column_getOffsets(columns);
    return Table_addKeyDecs(new_table, decs);
}

static Table_t *Table_addPrimaryKey(Table_t *table, const char *col_name)
{
    Column_t *col = table->columns;
    for (; col; col = col->next)
    {
        if (!strcmp(col->name, col_name))
        {
            Column_addConstraint(col, PrimaryKey());
            return table;
        }
    }
    return NULL;
}

void Table_free(void *table_vptr)
{
    Table_t *table = (Table_t *)table_vptr;
    Column_freeList(table->columns);
    free(table->name);
    free(table);
}

void TableReference_free(TableReference_t *tref)
{
    if (!tref)
    {
        fprintf(stderr, "Warning: TableReference_free called on null pointer\n");
        return;
    }
    free(tref->table_name);
    /* alias is optional */
    if (tref->alias)
        free(tref->alias);
    free(tref);
}

void Table_print(Table_t *table)
{
    Column_t *col = table->columns;
    int first = 1, count = 0;
    char buf[100];
    printf("Table %s (\n", table->name);
    for (; col; col = col->next)
    {
        if (first) first = 0;
        else printf(",\n");
        printf("\t%s %s", col->name, typeToString(col->type, buf));
        Constraint_printList(col->constraints);
        if (++count == 10) break;
    }
    printf("\n)\n");
}

KeyDec_t *KeyDec_append(KeyDec_t *decs, KeyDec_t *dec)
{
    decs->next = dec;
    return decs;
}

TableReference_t *TableReference_make(char *table_name, char *alias)
{
    TableReference_t *ref = (TableReference_t *)calloc(1, sizeof(TableReference_t));
    ref->table_name = table_name;
    ref->alias = alias;
    return ref;
}

Index_t *Index_make(char *name, char *table_name, char *column_name)
{
    Index_t *idx = (Index_t *)calloc(1, sizeof(Index_t));
    idx->name = name;
    idx->table_name = table_name;
    idx->column_name = column_name;
    return idx;
}

Index_t *Index_makeUnique(Index_t *idx)
{
    idx->unique = 1;
    return idx;
}

void Index_print(Index_t *idx)
{
    printf("Index '%s' on %s (%s)", idx->column_name,
           idx->table_name,
           idx->column_name);
    if (idx->unique) printf(", unique");
    puts("");
}

void Index_free(Index_t *idx)
{
    free(idx->name);
    free(idx->column_name);
    free(idx->table_name);
    free(idx);
}

Create_t *Create_fromTable(Table_t *table)
{
    Create_t *c = (Create_t *)calloc(1, sizeof(Create_t));
    c->t = CREATE_TABLE;
    c->table = table;
    return c;
}

Create_t *Create_fromIndex(Index_t *idx)
{
    Create_t *c = (Create_t *)calloc(1, sizeof(Create_t));
    c->t = CREATE_INDEX;
    c->index = idx;
    return c;
}

void Create_print(Create_t *cre)
{
    printf("CREATE ");
    if (cre->t == CREATE_TABLE)
        Table_print(cre->table);
    else
        Index_print(cre->index);
}

void Create_free(Create_t *cre)
{
    if (cre->t == CREATE_TABLE)
        Table_free(cre->table);
    else
        Index_free(cre->index);
    free(cre);
}
