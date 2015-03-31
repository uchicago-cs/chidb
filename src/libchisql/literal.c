#include <chisql/chisql.h>


Literal_t *litInt(int i)
{
    Literal_t *lval = (Literal_t *)calloc(1, sizeof(Literal_t));
    lval->t = TYPE_INT;
    lval->val.ival = i;
    return lval;
}

Literal_t *litDouble(double d)
{
    Literal_t *lval = (Literal_t *)calloc(1, sizeof(Literal_t));
    lval->t = TYPE_DOUBLE;
    lval->val.dval = d;
    return lval;
}

Literal_t *litChar(char c)
{
    Literal_t *lval = (Literal_t *)calloc(1, sizeof(Literal_t));
    lval->t = TYPE_CHAR;
    lval->val.cval = c;
    return lval;
}

Literal_t *litText(char *str)
{
    Literal_t *lval = (Literal_t *)calloc(1, sizeof(Literal_t));
    lval->t = TYPE_TEXT;
    lval->val.strval = str;
    return lval;
}

void Literal_print(Literal_t *val)
{
    char buf[100];
    printf("%s ", typeToString(val->t, buf));
    switch (val->t)
    {
    case TYPE_INT:
        printf("%d", val->val.ival);
        break;
    case TYPE_DOUBLE:
        printf("%f", val->val.dval);
        break;
    case TYPE_CHAR:
        printf("'%c'", val->val.cval);
        break;
    case TYPE_TEXT:
        printf("\"%s\"", val->val.strval);
        break;
    default:
        printf("(unknown type)");
    }
}

void Literal_printList(Literal_t *val_list)
{
    int first = 1;
    printf("[");
    while (val_list)
    {
        if (first) first = 0;
        else printf(", ");
        Literal_print(val_list);
        val_list = val_list->next;
    }
    printf("]");
}

static Literal_t *Literal_app(Literal_t *lit1, Literal_t *lit2)
{
    lit1->next = lit2;
    return lit1;
}

Literal_t *Literal_append(Literal_t *lit1, Literal_t *lit2)
{
    if (!lit1) return lit2;
    return Literal_app(lit1, Literal_append(lit1->next, lit2));
}

void Literal_free(Literal_t *lval)
{
    if (lval->t == TYPE_TEXT)
        free(lval->val.strval);
    free(lval);
}

void Literal_freeList(Literal_t *lval)
{
    Literal_t *temp;
    while (lval)
    {
        temp = lval;
        lval = lval->next;
        free(temp);
    }
}
