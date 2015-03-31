#include <chisql/chisql.h>

char *typeToString(enum data_type type, char *buf)
{
    switch (type)
    {
    case TYPE_INT:
            sprintf(buf, "int");
        break;
    case TYPE_DOUBLE:
        sprintf(buf, "double");
        break;
    case TYPE_CHAR:
        sprintf(buf, "char");
        break;
    case TYPE_TEXT:
        sprintf(buf, "text");
        break;
    }
    return buf;
}

StrList_t *StrList_makeWithNext(const char *str, StrList_t *next)
{
    StrList_t *list = (StrList_t *)calloc(1, sizeof(StrList_t));
    list->str = strdup(str);
    list->next = next;
    return list;
}

void StrList_print(StrList_t *list)
{
    int first = 1;
    printf("[");
    while (list)
    {
        if (first) first=0;
        else printf(", ");
        printf("%s", list->str);
        list = list->next;
    }
    printf("]");
}

void StrList_free(StrList_t *list)
{
    while (list)
    {
        StrList_t *next = list->next;
        free(list);
        list = next;
    }
}

int ind = 0;

void upInd()
{
    ind++;
    printf("\n");
}

void downInd()
{
    ind--;
    printf("\n");
    if (ind < 0) printf("error, ind is < 0");
}

#define BUF_SIZE 5000

void indent_print(const char *format,...)
{
    /* indent */
    int i;
    va_list argptr;
    char buffer[BUF_SIZE];
    if (ind < 1) ind = 0;
    for (i=0; i<ind; ++i)
    {
        if (i == 0)
            sprintf(buffer, "\t");
        else
            strcat(buffer, "\t");
    }
    va_start(argptr, format);
    vsnprintf(buffer + ind, BUF_SIZE, format, argptr);
    va_end(argptr);
    fputs(buffer, stdout);
    fflush(stdout);
}

static StrList_t *StrList_app(StrList_t *list1, StrList_t *list2)
{
    list1->next = list2;
    return list1;
}

StrList_t *StrList_append(StrList_t *list1, StrList_t *list2)
{
    if (!list1) return list2;
    return StrList_app(list1, StrList_append(list1->next, list2));
}

StrList_t *StrList_make(char *str)
{
    StrList_t *list = (StrList_t *)calloc(1, sizeof(StrList_t));
    list->str = str;
    return list;
}


void Query_free(Query_t *query)
{
    switch (query->t)
    {
    case SELECT_Q:
        SRA_free(query->sra);
        return;
    case CREATE_T_Q:
        Table_free(query->table);
        return;
    case CREATE_I_Q:
        Index_free(query->index);
        return;
    case INSERT_Q:
        Insert_free(query->insert);
        return;
    case DELETE_Q:
        Delete_free(query->del);
        return;
    }
}

/*#define COMMON_TEST*/
#ifdef COMMON_TEST
int main(int argc, char const *argv[])
{
    StrList_t *list = StrList_makeWithNext("hello", NULL);
    const char *strs[] = {"hi", "how", "are", "you"};
    int i;
    for (i=0; i<sizeof(strs)/sizeof(char *); ++i)
    {
        printf("going to add %s\n", strs[i]);
        fflush(stdout);
        StrList_append(list, StrList_makeWithNext(strs[i], NULL));
        StrList_print(list);
    }
    return 0;
}
#endif
