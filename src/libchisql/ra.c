#include <chisql/chisql.h>


void RA_print(RA_t *ra)
{
    switch(ra->t)
    {
    case RA_TABLE:
        indent_print("Table(%s)", ra->table.name);
        break;
    case RA_SIGMA:
        indent_print("Sigma(");
        Condition_print(ra->sigma.cond);
        printf(", ");
        upInd();
        RA_print(ra->sigma.ra);
        downInd();
        indent_print(")");
        break;
    case RA_PI:
        indent_print("Pi(");
        Expression_printList(ra->pi.expr_list);
        printf(", ");
        upInd();
        RA_print(ra->pi.ra);
        downInd();
        indent_print(")");
        break;
    case RA_UNION:
        indent_print("Union(");
        upInd();
        RA_print(ra->binary.ra1);
        indent_print(", ");
        RA_print(ra->binary.ra2);
        downInd();
        indent_print(")");
        break;
    case RA_DIFFERENCE:
        indent_print("Difference(");
        upInd();
        RA_print(ra->binary.ra1);
        indent_print(", ");
        RA_print(ra->binary.ra2);
        downInd();
        indent_print(")");
        break;
    case RA_CROSS:
        indent_print("Cross(");
        upInd();
        RA_print(ra->binary.ra1);
        indent_print(", \n");
        RA_print(ra->binary.ra2);
        downInd();
        indent_print(")");
        break;
    case RA_RHO_EXPR:
        indent_print("RhoExpr(");
        Expression_print(ra->rho.to_rename);
        printf(", \"%s\",", ra->rho.new_name);
        upInd();
        RA_print(ra->rho.ra);
        downInd();
        indent_print(")");
        break;
    case RA_RHO_TABLE:
        indent_print("RhoTable(");
        upInd();
        RA_print(ra->rho.ra);
        downInd();
        printf(", \"%s\"", ra->rho.new_name);
        indent_print(")");
        break;
    default:
        puts("Unknown RA_t type");
    }
}

/* TODO: This function depended on the "mock_db" module that assumed that
 * the table schemas would be provided in the SQL file being parsed
 * (via CREATE TABLE statements). Instead, the schema is read from
 * the chidb file. This function should be modified to obtain the
 * columns some other way (possibly as an extra parameter)
 */
RA_t *RA_Table (const char *name)
{
    RA_t *new_ra = (RA_t *)calloc(1, sizeof(RA_t));
    new_ra->t = RA_TABLE;
    new_ra->table.name = strdup(name);
    new_ra->columns = NULL; /* <-- No columns. See comment above */
    return new_ra;
}

RA_t *RA_Sigma (RA_t *ra, Condition_t *cond)
{
    RA_t *new_ra = (RA_t *)calloc(1, sizeof(RA_t));
    new_ra->t = RA_SIGMA;
    new_ra->sigma.cond = cond;
    new_ra->sigma.ra = ra;
    new_ra->columns = NULL; // See comment in RA_Table. list_deepCopy(&ra->columns);
    return new_ra;
}

RA_t *RA_Pi (RA_t *ra, Expression_t *expr_list)
{
    RA_t *new_ra = (RA_t *)calloc(1, sizeof(RA_t));
    new_ra->t = RA_PI;
    new_ra->pi.ra = ra;
    new_ra->pi.expr_list = expr_list;
    new_ra->columns =  NULL; // See comment in RA_Table. list_deepCopy(&ra->columns);
    return new_ra;
}

RA_t *RA_Union (RA_t *ra1, RA_t *ra2)
{
    RA_t *new_ra = (RA_t *)calloc(1, sizeof(RA_t));
    new_ra->t = RA_UNION;
    new_ra->binary.ra1 = ra1;
    new_ra->binary.ra2 = ra2;
    new_ra->columns = NULL; // See comment in RA_Table.  list_union(&ra1->columns, &ra2->columns);
    return new_ra;
}

RA_t *RA_Difference (RA_t *ra1, RA_t *ra2)
{
    RA_t *new_ra = (RA_t *)calloc(1, sizeof(RA_t));
    new_ra->t = RA_DIFFERENCE;
    new_ra->binary.ra1 = ra1;
    new_ra->binary.ra2 = ra2;
    new_ra->columns = NULL; // See comment in RA_Table.  list_difference(&ra1->columns, &ra2->columns);
    return new_ra;
}

RA_t *RA_Cross (RA_t *ra1, RA_t *ra2)
{
    RA_t *new_ra = (RA_t *)calloc(1, sizeof(RA_t));
    new_ra->t = RA_CROSS;
    new_ra->binary.ra1 = ra1;
    new_ra->binary.ra2 = ra2;
    new_ra->columns = NULL; // See comment in RA_Table.  list_union(&ra1->columns, &ra2->columns);
    return new_ra;
}

RA_t *RA_RhoTable (RA_t *ra, const char *new_name)
{
    RA_t *new_ra = (RA_t *)calloc(1, sizeof(RA_t));
    new_ra->t = RA_RHO_TABLE;
    new_ra->rho.new_name = strdup(new_name);
    new_ra->columns = NULL; // See comment in RA_Table.  list_deepCopy(&ra->columns);
    return new_ra;
}

RA_t *RA_RhoExpr (RA_t *ra, Expression_t *expr, const char *new_name)
{
    RA_t *new_ra = (RA_t *)calloc(1, sizeof(RA_t));
    new_ra->t = RA_RHO_EXPR;
    new_ra->rho.to_rename = expr;
    new_ra->rho.new_name = strdup(new_name);
    new_ra->columns = NULL; // See comment in RA_Table.  list_deepCopy(&ra->columns);
    return new_ra;
}

void RA_free(RA_t *ra)
{
    switch(ra->t)
    {
    case RA_SIGMA:
        Condition_free(ra->sigma.cond);
        RA_free(ra->sigma.ra);
        break;
    case RA_PI:
        RA_free(ra->pi.ra);
        Expression_freeList(ra->pi.expr_list);
        break;
    case RA_UNION:
    case RA_DIFFERENCE:
    case RA_CROSS:
        RA_free(ra->binary.ra1);
        RA_free(ra->binary.ra2);
        break;
    case RA_RHO_EXPR:
        RA_free(ra->rho.ra);
        free(ra->rho.new_name);
        Expression_free(ra->rho.to_rename);
        break;
    case RA_RHO_TABLE:
        RA_free(ra->rho.ra);
        free(ra->rho.new_name);
        break;
    case RA_TABLE:
        free(ra->table.name);
        break;
    }
    free(ra);
}

#ifdef RA_TEST

int main(int argc, char const *argv[])
{
    RA_t *ra1 = Pi(Sigma(Table("bazzle"), And(Eq("foo", "bar"), Lt("popo", "toto"))),
                   3, "foo", "bar", "baz"),
                *ra2 = Pi(Rho(Sigma(Cross(Rho(Table("Foo"), "f", 1, "Col1"),
                                          Rho(Table("Foo"), "g", 1, "Col2")),Not(Eq("Col1","Col2"))
                                   ),"res", 2, "Col1", "Col2"),2, "Col1", "Col2");
    RA_print(ra1);
    RA_print(ra2);
    RA_free(ra1);
    RA_free(ra2);
    return 0;
}

#endif
