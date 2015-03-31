#include <chisql/chisql.h>

void Condition_print(Condition_t *cond)
{
    if (!cond) return; /* just in case */
    switch(cond->t)
    {
    case RA_COND_EQ:
        Expression_print(cond->cond.comp.expr1);
        printf(" = ");
        Expression_print(cond->cond.comp.expr2);
        break;
    case RA_COND_LT:
        Expression_print(cond->cond.comp.expr1);
        printf(" < ");
        Expression_print(cond->cond.comp.expr2);
        break;
    case RA_COND_GT:
        Expression_print(cond->cond.comp.expr1);
        printf(" > ");
        Expression_print(cond->cond.comp.expr2);
        break;
    case RA_COND_LEQ:
        Expression_print(cond->cond.comp.expr1);
        printf(" <= ");
        Expression_print(cond->cond.comp.expr2);
        break;
    case RA_COND_GEQ:
        Expression_print(cond->cond.comp.expr1);
        printf(" >= ");
        Expression_print(cond->cond.comp.expr2);
        break;
    case RA_COND_AND:
        Condition_print(cond->cond.binary.cond1);
        printf(" and ");
        Condition_print(cond->cond.binary.cond2);
        break;
    case RA_COND_OR:
        Condition_print(cond->cond.binary.cond1);
        printf(" or ");
        Condition_print(cond->cond.binary.cond2);
        break;
    case RA_COND_NOT:
        if (cond->cond.unary.cond->t == RA_COND_EQ)
        {
            Expression_print(cond->cond.unary.cond->cond.comp.expr1);
            printf(" != ");
            Expression_print(cond->cond.unary.cond->cond.comp.expr2);
        }
        else
        {
            printf("not (");
            Condition_print(cond->cond.unary.cond);
            printf(")");
        }
        break;
    case RA_COND_IN:
        Expression_print(cond->cond.in.expr);
        printf(" in ");
        Literal_printList(cond->cond.in.values_list);
        break;
    default:
        puts("Unknown condession type");
    }
}

Condition_t *Eq(Expression_t *expr1, Expression_t *expr2)
{
    Condition_t *new_cond = (Condition_t *)calloc(1, sizeof(Condition_t));
    new_cond->t = RA_COND_EQ;
    new_cond->cond.comp.expr1 = expr1;
    new_cond->cond.comp.expr2 = expr2;
    return new_cond;
}

Condition_t *Lt(Expression_t *expr1, Expression_t *expr2)
{
    Condition_t *new_cond = (Condition_t *)calloc(1, sizeof(Condition_t));
    new_cond->t = RA_COND_LT;
    new_cond->cond.comp.expr1 = expr1;
    new_cond->cond.comp.expr2 = expr2;
    return new_cond;
}

Condition_t *Gt(Expression_t *expr1, Expression_t *expr2)
{
    Condition_t *new_cond = (Condition_t *)calloc(1, sizeof(Condition_t));
    new_cond->t = RA_COND_GT;
    new_cond->cond.comp.expr1 = expr1;
    new_cond->cond.comp.expr2 = expr2;
    return new_cond;
}

Condition_t *Leq(Expression_t *expr1, Expression_t *expr2)
{
    Condition_t *new_cond = (Condition_t *)calloc(1, sizeof(Condition_t));
    new_cond->t = RA_COND_LEQ;
    new_cond->cond.comp.expr1 = expr1;
    new_cond->cond.comp.expr2 = expr2;
    return new_cond;
}

Condition_t *Geq(Expression_t *expr1, Expression_t *expr2)
{
    Condition_t *new_cond = (Condition_t *)calloc(1, sizeof(Condition_t));
    new_cond->t = RA_COND_GEQ;
    new_cond->cond.comp.expr1 = (expr1);
    new_cond->cond.comp.expr2 = (expr2);
    return new_cond;
}

Condition_t *And(Condition_t *cond1, Condition_t *cond2)
{
    Condition_t *new_cond = (Condition_t *)calloc(1, sizeof(Condition_t));
    new_cond->t = RA_COND_AND;
    new_cond->cond.binary.cond1 = cond1;
    new_cond->cond.binary.cond2 = cond2;
    return new_cond;
}

Condition_t *Or(Condition_t *cond1, Condition_t *cond2)
{
    Condition_t *new_cond = (Condition_t *)calloc(1, sizeof(Condition_t));
    new_cond->t = RA_COND_OR;
    new_cond->cond.binary.cond1 = cond1;
    new_cond->cond.binary.cond2 = cond2;
    return new_cond;
}

Condition_t *Not(Condition_t *cond)
{
    Condition_t *new_cond = (Condition_t *)calloc(1, sizeof(Condition_t));
    new_cond->t = RA_COND_NOT;
    new_cond->cond.unary.cond = cond;
    return new_cond;
}

Condition_t *In(Expression_t *expr, Literal_t *values_list)
{
    Condition_t *new_cond = (Condition_t *)calloc(1, sizeof(Condition_t));
    new_cond->t = RA_COND_IN;
    new_cond->cond.in.expr = expr;
    new_cond->cond.in.values_list = values_list;
    return new_cond;
}

void Condition_free(Condition_t *cond)
{
    switch (cond->t)
    {
    case RA_COND_EQ:
    case RA_COND_LEQ:
    case RA_COND_GEQ:
    case RA_COND_GT:
    case RA_COND_LT:
        free(cond->cond.comp.expr1);
        free(cond->cond.comp.expr2);
        break;
    case RA_COND_AND:
    case RA_COND_OR:
        Condition_free(cond->cond.binary.cond1);
        Condition_free(cond->cond.binary.cond2);
        break;
    case RA_COND_NOT:
        Condition_free(cond->cond.unary.cond);
        break;
    case RA_COND_IN:
        Literal_freeList(cond->cond.in.values_list);
        Expression_freeList(cond->cond.in.expr);
        break;
    }
    free(cond);
}
