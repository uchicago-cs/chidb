#include <chisql/chisql.h>


Expression_t *Term(const char *str)
{
    Expression_t *new_expr = (Expression_t *)calloc(1, sizeof(Expression_t));
    new_expr->t = EXPR_TERM;
    new_expr->expr.term.t = TERM_ID;
    new_expr->expr.term.id = strdup(str);
    return new_expr;
}

Expression_t *TermLiteral(Literal_t *val)
{
    Expression_t *new_expr = (Expression_t *)calloc(1, sizeof(Expression_t));
    new_expr->t = EXPR_TERM;
    new_expr->expr.term.t = TERM_LITERAL;
    new_expr->expr.term.val = val;
    return new_expr;
}

Expression_t *TermNull(void)
{
    Expression_t *new_expr = (Expression_t *)calloc(1, sizeof(Expression_t));
    new_expr->t = EXPR_TERM;
    new_expr->expr.term.t = TERM_NULL;
    return new_expr;
}

Expression_t *TermColumnReference(ColumnReference_t *ref)
{
    Expression_t *new_expr = (Expression_t *)calloc(1, sizeof(Expression_t));
    new_expr->t = EXPR_TERM;
    new_expr->expr.term.t = TERM_COLREF;
    new_expr->expr.term.ref = ref;
    return new_expr;
}

Expression_t *TermFunction(int functype, Expression_t *expr)
{
    Expression_t *new_expr = (Expression_t *)calloc(1, sizeof(Expression_t));
    new_expr->t = EXPR_TERM;
    new_expr->expr.term.t = TERM_FUNC;
    new_expr->expr.term.f.t = functype;
    new_expr->expr.term.f.expr = expr;
    return new_expr;
}

static void Term_print(ExprTerm term)
{
    switch (term.t)
    {
    case TERM_ID:
        printf("%s", term.id);
        break;
    case TERM_LITERAL:
        Literal_print(term.val);
        break;
    case TERM_NULL:
        printf("NULL");
        break;
    case TERM_COLREF:
        if (term.ref->tableName)
            printf("%s.", term.ref->tableName);
        printf("%s", term.ref->columnName);
        break;
    case TERM_FUNC:
        switch (term.f.t)
        {
        case FUNC_AVG:
            printf("AVG(");
            Expression_print(term.f.expr);
            printf(")");
            break;
        case FUNC_COUNT:
            printf("COUNT(");
            Expression_print(term.f.expr);
            printf(")");
            break;
        case FUNC_MAX:
            printf("MAX(");
            Expression_print(term.f.expr);
            printf(")");
            break;
        case FUNC_MIN:
            printf("MIN(");
            Expression_print(term.f.expr);
            printf(")");
            break;
        case FUNC_SUM:
            printf("SUM(");
            Expression_print(term.f.expr);
            printf(")");
            break;
        default:
            printf("Unknown function");
        }
        break;
    default:
        printf("Unknown term type");
    }
}

static void Term_free(ExprTerm term)
{
    switch (term.t)
    {
    case TERM_ID:
        free(term.id);
        break;
    case TERM_LITERAL:
        Literal_free(term.val);
        break;
    case TERM_NULL:
        break;
    case TERM_COLREF:
        if (term.ref->tableName)
            free(term.ref->tableName);
        free(term.ref->columnName);
        break;
    case TERM_FUNC:
        switch (term.f.t)
        {
        case FUNC_AVG:
            Expression_free(term.f.expr);
            break;
        case FUNC_COUNT:
            Expression_free(term.f.expr);
            break;
        case FUNC_MAX:
            Expression_free(term.f.expr);
            break;
        case FUNC_MIN:
            Expression_free(term.f.expr);
            break;
        case FUNC_SUM:
            Expression_free(term.f.expr);
            break;
        default:
            printf("Can't delete unknown function\n");
        }
    default:
        printf("Can't delete, unknown term type");
    }
}

Expression_t *Binary(Expression_t *expr1, Expression_t *expr2, enum ExprType t)
{
    Expression_t *expr = (Expression_t *)calloc(1, sizeof(Expression_t));
    expr->t = t;
    expr->expr.binary.expr1 = expr1;
    expr->expr.binary.expr2 = expr2;
    return expr;
}

Expression_t *Plus(Expression_t *expr1, Expression_t *expr2)
{
    return Binary(expr1, expr2, EXPR_PLUS);
}

Expression_t *Minus(Expression_t *expr1, Expression_t *expr2)
{
    return Binary(expr1, expr2, EXPR_MINUS);
}

Expression_t *Multiply(Expression_t *expr1, Expression_t *expr2)
{
    return Binary(expr1, expr2, EXPR_MULTIPLY);
}

Expression_t *Divide(Expression_t *expr1, Expression_t *expr2)
{
    return Binary(expr1, expr2, EXPR_DIVIDE);
}

Expression_t *Concat(Expression_t *expr1, Expression_t *expr2)
{
    return Binary(expr1, expr2, EXPR_CONCAT);
}

Expression_t *Neg(Expression_t *expr)
{
    Expression_t *new_expr = (Expression_t *)calloc(1, sizeof(Expression_t));
    new_expr->t = EXPR_NEG;
    new_expr->expr.unary.expr = expr;
    return new_expr;
}

enum TermType Expression_type(Expression_t *expr)
{
    enum TermType t1, t2;
    switch (expr->t)
    {
    case EXPR_TERM:
        return expr->expr.term.t;
    case EXPR_NEG:
        return Expression_type(expr->expr.unary.expr);
    default:
        t1 = Expression_type(expr->expr.binary.expr1);
        t2 = Expression_type(expr->expr.binary.expr2);
        if (t1 != t2)
        {
            fprintf(stderr, "Type mismatch on binary expression:\n");
            Expression_print(expr->expr.binary.expr1);
            fprintf(stderr, "\nis not the same type as\n");
            Expression_print(expr->expr.binary.expr2);
            exit(1);
        }
        return t1;
    }
}

void Expression_print(Expression_t *expr)
{
    if (expr->t != EXPR_TERM) printf("(");
    switch (expr->t)
    {
    case EXPR_CONCAT:
        Expression_print(expr->expr.binary.expr1);
        printf(" || ");
        Expression_print(expr->expr.binary.expr2);
        break;
    case EXPR_PLUS:
        Expression_print(expr->expr.binary.expr1);
        printf(" + ");
        Expression_print(expr->expr.binary.expr2);
        break;
    case EXPR_MINUS:
        Expression_print(expr->expr.binary.expr1);
        printf(" - ");
        Expression_print(expr->expr.binary.expr2);
        break;
    case EXPR_MULTIPLY:
        Expression_print(expr->expr.binary.expr1);
        printf(" * ");
        Expression_print(expr->expr.binary.expr2);
        break;
    case EXPR_DIVIDE:
        Expression_print(expr->expr.binary.expr1);
        printf(" / ");
        Expression_print(expr->expr.binary.expr2);
        break;
    case EXPR_NEG:
        printf("-");
        Expression_print(expr->expr.unary.expr);
        break;
    case EXPR_TERM:
        Term_print(expr->expr.term);
        break;
    default:
        printf("(Unknown expression type '%d')", expr->t);
    }
    if (expr->t != EXPR_TERM) printf(")");
    if (expr->alias) printf(" as %s", expr->alias);
}

static Expression_t *app_exp(Expression_t *e1, Expression_t *e2)
{
    e1->next = e2;
    return e1;
}

Expression_t *append_expression(Expression_t *e1, Expression_t *e2)
{
    if (!e1) return e2;
    return app_exp(e1, append_expression(e1->next, e2));
}

void Expression_printList (Expression_t *expr)
{
    int first = 1;
    printf("[");
    while (expr)
    {
        if (first) first = 0;
        else printf(", ");
        Expression_print(expr);
        expr = expr->next;
    }
    printf("]");
}

Expression_t *add_alias(Expression_t *expr, const char *alias)
{
    if (alias) expr->alias = strdup(alias);
    return expr;
}

void Expression_free(Expression_t *expr)
{
    switch (expr->t)
    {
    case EXPR_CONCAT:
        Expression_free(expr->expr.binary.expr1);
        Expression_free(expr->expr.binary.expr2);
        break;
    case EXPR_PLUS:
        Expression_free(expr->expr.binary.expr1);
        Expression_free(expr->expr.binary.expr2);
        break;
    case EXPR_MINUS:
        Expression_free(expr->expr.binary.expr1);
        Expression_free(expr->expr.binary.expr2);
        break;
    case EXPR_MULTIPLY:
        Expression_free(expr->expr.binary.expr1);
        Expression_free(expr->expr.binary.expr2);
        break;
    case EXPR_DIVIDE:
        Expression_free(expr->expr.binary.expr1);
        Expression_free(expr->expr.binary.expr2);
        break;
    case EXPR_NEG:
        Expression_free(expr->expr.unary.expr);
        break;
    case EXPR_TERM:
        Term_free(expr->expr.term);
        break;
    default:
        printf("Can't delete unknown expression type '%d')", expr->t);
    }
    if (expr->alias) free(expr->alias);
    free(expr);
}

void Expression_freeList(Expression_t *expr)
{

}

/*#define EXPRESSION_TEST*/
#ifdef EXPRESSION_TEST
int main(int argc, char const *argv[])
{
    Expression_t *a = Term("a"),
                  *b = Term("b"),
                   *c = TermLiteral(litInt(5)),
                    *plus = Plus(a,b),
                     *mult = Multiply(plus, c);
    Expression_print(mult);
    append_expression(mult, plus);
    puts("");
    Expression_printList(mult);
    puts("");
    return 0;
}
#endif
