#include <chisql/chisql.h>

static ssize_t size_constraint = -1;

Constraint_t *NotNull(void)
{
    Constraint_t *con = (Constraint_t *)calloc(1, sizeof(Constraint_t));
    con->t = CONS_NOT_NULL;
    return con;
}

Constraint_t *AutoIncrement(void)
{
    Constraint_t *con = (Constraint_t *)calloc(1, sizeof(Constraint_t));
    con->t = CONS_AUTO_INCREMENT;
    return con;
}

Constraint_t *PrimaryKey(void)
{
    Constraint_t *con = (Constraint_t *)calloc(1, sizeof(Constraint_t));
    con->t = CONS_PRIMARY_KEY;
    return con;
}

Constraint_t *ForeignKey(ForeignKeyRef_t fkr)
{
    Constraint_t *con = (Constraint_t *)calloc(1, sizeof(Constraint_t));
    con->t = CONS_FOREIGN_KEY;
    con->constraint.ref = fkr;
    return con;
}

Constraint_t *Default(Literal_t *val)
{
    Constraint_t *con = (Constraint_t *)calloc(1, sizeof(Constraint_t));
    con->t = CONS_DEFAULT;
    con->constraint.default_val = val;
    return con;
}

Constraint_t *Unique(void)
{
    Constraint_t *con = (Constraint_t *)calloc(1, sizeof(Constraint_t));
    con->t = CONS_UNIQUE;
    return con;
}

Constraint_t *Check(Condition_t *cond)
{
    Constraint_t *con = (Constraint_t *)calloc(1, sizeof(Constraint_t));
    con->t = CONS_CHECK;
    con->constraint.check = cond;
    return con;
}

Constraint_t *ColumnSize(unsigned size)
{
    Constraint_t *con = (Constraint_t *)calloc(1, sizeof(Constraint_t));
    con->t = CONS_SIZE;
    con->constraint.size = size;
    return con;
}

static void deleteConstraint_ts(Constraint_t *constraint)
{
    if (constraint)
    {
        Constraint_t *next = constraint->next;
        switch (constraint->t)
        {
        case CONS_DEFAULT:
            Literal_free(constraint->constraint.default_val);
            break;
        case CONS_CHECK:
            Condition_free(constraint->constraint.check);
            break;
        default:
            break;
        }
        deleteConstraint_ts(next);
    }
}

void Column_freeList(Column_t *column)
{
    if (column)
    {
        Column_t *next = column->next;
        free(column->name);
        deleteConstraint_ts(column->constraints);
        free(column);
        Column_freeList(next);
    }
}

size_t Column_getSize(Column_t *col)
{
    Constraint_t *cons = col->constraints;
    while (cons)
    {
        if (cons->t == CONS_SIZE)
            return cons->constraint.size;
        cons = cons->next;
    }
    switch (col->type)
    {
    case TYPE_CHAR:
        return sizeof(char);
    case TYPE_DOUBLE:
        return sizeof(double);
    case TYPE_INT:
        return sizeof(int);
    case TYPE_TEXT:
        return 250; /* default text length */
    }

    return 0;
}

static void Column_getOffsets_r(Column_t *cols, size_t offset)
{
    if (!cols) return;
    cols->offset = offset;
    Column_getOffsets_r(cols->next, Column_getSize(cols));
}

void Column_getOffsets(Column_t *cols)
{
    Column_getOffsets_r(cols, 0);
}

void Constraint_printList(Constraint_t *constraints)
{
    int first = 1;
    if (constraints)
    {
        printf(" [");
        for (; constraints; constraints = constraints->next)
        {
            if (first)
            {
                first = 0;
            }
            else
            {
                printf(", ");
            }
            Constraint_print(constraints);
        }
        printf("]");
    }
}

ForeignKeyRef_t ForeignKeyRef_makeFull(const char *cname, ForeignKeyRef_t fkey)
{
    fkey.col_name = cname;
    return fkey;
}

ForeignKeyRef_t ForeignKeyRef_make(const char *foreign_tname,
                                   const char *foreign_cname)
{
    ForeignKeyRef_t fkey;
    fkey.col_name = NULL;
    fkey.table_name = foreign_tname;
    fkey.table_col_name = foreign_cname;
    return fkey;
}

Column_t *Column(const char *name, enum data_type type, Constraint_t *constraints)
{
    Column_t *new_column = (Column_t *)calloc(1, sizeof(Column_t));
    new_column->name = strdup(name);
    new_column->type = type;
    new_column->constraints = constraints;
    /* if the parser found a size constraint, then size_constraitn will be > 0 */
    if (size_constraint > 0)
    {
        Constraint_append(new_column->constraints, ColumnSize(size_constraint));
        size_constraint = -1;
    }
    return new_column;
}

Column_t *Column_addConstraint(Column_t *column, Constraint_t *constraints)
{
    column->constraints = Constraint_append(column->constraints, constraints);
    return column;
}

Constraint_t *Constraint_append(Constraint_t *constraints, Constraint_t *constraint)
{
    if (constraints == NULL)
        constraints = constraint;
    else
        constraints->next = constraint;
    return constraints;
}

void Constraint_print(void *constraint_voidp)
{
    Constraint_t *constraint = (Constraint_t *)constraint_voidp;
    switch(constraint->t)
    {
    case CONS_DEFAULT:
        printf("Default: ");
        Literal_print(constraint->constraint.default_val);
        break;
    case CONS_PRIMARY_KEY:
        printf("Primary Key");
        break;
    case CONS_UNIQUE:
        printf("Unique");
        break;
    case CONS_FOREIGN_KEY:
        printf("Foreign key (%s, %s)", constraint->constraint.ref.table_name,
               constraint->constraint.ref.table_col_name);
        break;
    case CONS_AUTO_INCREMENT:
        printf("Auto increment");
        break;
    case CONS_NOT_NULL:
        printf("Not null");
        break;
    case CONS_CHECK:
        printf("Check: ");
        Condition_print(constraint->constraint.check);
        break;
    case CONS_SIZE:
        printf("Size: %u", constraint->constraint.size);
        break;
    default:
        printf("Unknown constraint type");
    }
}

void Column_setSize(ssize_t size)
{
    size_constraint = size;
}

static Column_t *app_col(Column_t *col1, Column_t *col2)
{
    col1->next = col2;
    return col1;
}

Column_t *Column_append(Column_t *col1, Column_t *col2)
{
    if (!col1) return col2;
    return app_col(col1, Column_append(col1->next, col2));
}

ColumnReference_t *ColumnReference_make(const char *tname, const char *cname)
{
    ColumnReference_t *ref = (ColumnReference_t *)calloc(1, sizeof(ColumnReference_t));
    if (tname) ref->tableName = strdup(tname);
    if (cname) ref->columnName = strdup(cname);
    return ref;
}

int Column_compareByName(const void *c1, const void *c2)
{
    return strcmp(((Column_t *)c1)->name, ((Column_t *)c2)->name);
}

void *Column_copy(void *col)
{
    Column_t *copy = (Column_t *)malloc(sizeof(Column_t));
    memcpy(copy, col, sizeof(Column_t));
    copy->name = strdup(((Column_t *)col)->name);
    copy->next = NULL; /* just in case */
    return copy;
}
