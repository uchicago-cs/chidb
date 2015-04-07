#include <chisql/chisql.h>


static SRA_t *SRABinary(SRA_t *sra1, SRA_t *sra2, enum SRAType t);

SRA_t *SRATable(TableReference_t *ref)
{
    SRA_t *sra = (SRA_t *)calloc(1, sizeof(SRA_t));
    sra->t = SRA_TABLE;
    sra->table.ref = ref;
    return sra;
}

SRA_t *SRAProject(SRA_t *sra, Expression_t *expr)
{
    SRA_t *new_sra = (SRA_t *)calloc(1, sizeof(SRA_t));
    new_sra->t = SRA_PROJECT;
    new_sra->project.sra = sra;
    new_sra->project.expr_list = expr;
    return new_sra;
}

SRA_t *SRASelect(SRA_t *sra, Condition_t *cond)
{
    if (!cond)
    {
        return sra;
    }
    else
    {
        SRA_t *new_sra = (SRA_t *)calloc(1, sizeof(SRA_t));
        new_sra->t = SRA_SELECT;
        new_sra->select.sra = sra;
        new_sra->select.cond = cond;
        return new_sra;
    }
}

SRA_t *SRAJoin(SRA_t *sra1, SRA_t *sra2, JoinCondition_t *cond)
{
    SRA_t *new_sra = (SRA_t *)calloc(1, sizeof(SRA_t));
    new_sra->t = SRA_JOIN;
    new_sra->join.sra1 = sra1;
    new_sra->join.sra2 = sra2;
    new_sra->join.opt_cond = cond;
    return new_sra;
}

SRA_t *SRALeftOuterJoin(SRA_t *sra1, SRA_t *sra2, JoinCondition_t *cond)
{
    SRA_t *res = SRAJoin(sra1, sra2, cond);
    res->t = SRA_LEFT_OUTER_JOIN;
    return res;
}

SRA_t *SRARightOuterJoin(SRA_t *sra1, SRA_t *sra2, JoinCondition_t *cond)
{
    SRA_t *res = SRAJoin(sra1, sra2, cond);
    res->t = SRA_RIGHT_OUTER_JOIN;
    return res;
}
SRA_t *SRAFullOuterJoin(SRA_t *sra1, SRA_t *sra2, JoinCondition_t *cond)
{
    SRA_t *res = SRAJoin(sra1, sra2, cond);
    res->t = SRA_FULL_OUTER_JOIN;
    return res;
}

static SRA_t *SRABinary(SRA_t *sra1, SRA_t *sra2, enum SRAType t)
{
    SRA_t *sra = (SRA_t *)calloc(1, sizeof(SRA_t));
    sra->t = t;
    sra->binary.sra1 = sra1;
    sra->binary.sra2 = sra2;
    return sra;
}

SRA_t *SRAUnion(SRA_t *sra1, SRA_t *sra2)
{
    return SRABinary(sra1, sra2, SRA_UNION);
}

SRA_t *SRAExcept(SRA_t *sra1, SRA_t *sra2)
{
    return SRABinary(sra1, sra2, SRA_EXCEPT);
}

SRA_t *SRAIntersect(SRA_t *sra1, SRA_t *sra2)
{
    return SRABinary(sra1, sra2, SRA_INTERSECT);
}

SRA_t *SRANaturalJoin(SRA_t *sra1, SRA_t *sra2)
{
    return SRABinary(sra1, sra2, SRA_NATURAL_JOIN);
}

void SRA_print(SRA_t *sra)
{
    if (!sra) return;
    switch(sra->t)
    {
    case SRA_TABLE:
        indent_print("Table(%s", sra->table.ref->table_name);
        if (sra->table.ref->alias) printf(" as %s", sra->table.ref->alias);
        printf(")");
        break;
    case SRA_SELECT:
        indent_print("Select(");
        Condition_print(sra->select.cond);
        printf(", ");
        upInd();
        SRA_print(sra->select.sra);
        downInd();
        indent_print(")");
        break;
    case SRA_PROJECT:
        indent_print("Project(");
        Expression_printList(sra->project.expr_list);
        printf(", ");
        upInd();
        SRA_print(sra->project.sra);
        if (sra->project.distinct ||
                sra->project.group_by ||
                sra->project.order_by)
        {
            printf(",\n");
            indent_print("Options: ");
            if (sra->project.distinct)
                printf("Distinct ");
            if (sra->project.group_by)
            {
                printf("Group by ");
                Expression_print(sra->project.group_by);
                printf(" ");
            }
            if (sra->project.order_by)
            {
                printf("Order by ");
                Expression_print(sra->project.order_by);
                printf(sra->project.asc_desc == ORDER_BY_ASC ? " a" : " de");
                printf("scending");
            }
        }
        downInd();
        indent_print(")");
        break;
    case SRA_UNION:
        indent_print("Union(");
        upInd();
        SRA_print(sra->binary.sra1);
        indent_print(", ");
        SRA_print(sra->binary.sra2);
        downInd();
        indent_print(")");
        break;
    case SRA_EXCEPT:
        indent_print("Except(");
        upInd();
        SRA_print(sra->binary.sra1);
        indent_print(", ");
        SRA_print(sra->binary.sra2);
        downInd();
        indent_print(")");
        break;
    case SRA_INTERSECT:
        indent_print("Intersect(");
        upInd();
        SRA_print(sra->binary.sra1);
        indent_print(", ");
        SRA_print(sra->binary.sra2);
        downInd();
        indent_print(")");
        break;
    case SRA_JOIN:
        indent_print("Join(");
        upInd();
        SRA_print(sra->binary.sra1);
        printf(", \n");
        SRA_print(sra->binary.sra2);
        if (sra->join.opt_cond)
        {
            printf(",\n");
            indent_print("");
            JoinCondition_print(sra->join.opt_cond);
        }
        downInd();
        indent_print(")");
        break;
    case SRA_NATURAL_JOIN:
        indent_print("NaturalJoin(");
        upInd();
        SRA_print(sra->binary.sra1);
        printf(", \n");
        SRA_print(sra->binary.sra2);
        downInd();
        indent_print(")");
        break;
    case SRA_LEFT_OUTER_JOIN:
    case SRA_RIGHT_OUTER_JOIN:
    case SRA_FULL_OUTER_JOIN:
        if (sra->t == SRA_LEFT_OUTER_JOIN) indent_print("Left");
        else if (sra->t == SRA_RIGHT_OUTER_JOIN) indent_print("Right");
        else indent_print("Full");
        printf("OuterJoin(");
        upInd();
        SRA_print(sra->join.sra1);
        printf(",\n");
        SRA_print(sra->join.sra2);
        if (sra->join.opt_cond)
        {
            printf(",\n");
            indent_print("");
            JoinCondition_print(sra->join.opt_cond);
        }
        downInd();
        indent_print(")");
        break;
    default:
        puts("Unknown SRA type");
    }
}

void JoinCondition_print(JoinCondition_t *cond)
{
    if (cond->t == JOIN_COND_ON)
    {
        printf("On: ");
        Condition_print(cond->on);
    }
    else if (cond->t == JOIN_COND_USING)
    {
        printf("Using: ");
        StrList_print(cond->col_list);
    }
    else
    {
        printf("(Unknown JoinCondition type)");
    }
}

SRA_t *SRA_applyOption(SRA_t *sra, ProjectOption_t *option)
{
    if (sra->t != SRA_PROJECT)
    {
        fprintf(stderr, "Error: can't apply order by to anything except project.\n");
        exit(1);
    }
    else if (option != NULL)
    {
        if (option->order_by)
        {
            sra->project.order_by = option->order_by;
            sra->project.asc_desc = option->asc_desc;
        }
        if (option->group_by)
        {
            sra->project.group_by = option->group_by;
        }
    }
    return sra;
}

void ProjectOption_free(ProjectOption_t *opt)
{
    if (opt->group_by)
        Expression_free(opt->group_by);
    if (opt->order_by)
        Expression_free(opt->order_by);
    free(opt);
}

ProjectOption_t *OrderBy_make(Expression_t *expr, enum OrderBy asc_desc)
{
    ProjectOption_t *ob = (ProjectOption_t *)calloc(1, sizeof(ProjectOption_t));
    ob->asc_desc = asc_desc;
    ob->order_by = expr;
    return ob;
}

ProjectOption_t *GroupBy_make(Expression_t *expr)
{
    ProjectOption_t *gb = (ProjectOption_t *)calloc(1, sizeof(ProjectOption_t));
    gb->group_by = expr;
    return gb;
}

ProjectOption_t *ProjectOption_combine(ProjectOption_t *op1,
                                       ProjectOption_t *op2)
{
    if (op1->group_by && op2->group_by)
    {
        fprintf(stderr, "Error: can't combine two group_bys.\n");
        exit(1);
    }
    if (op1->order_by && op2->order_by)
    {
        fprintf(stderr, "Error: can't combine two order_bys.\n");
        exit(1);
    }
    if (op2->group_by)
    {
        op1->group_by = op2->group_by;
        op2->group_by = NULL;
        ProjectOption_free(op2);
        return op1;
    }
    else
    {
        op2->group_by = op1->group_by;
        op1->group_by = NULL;
        ProjectOption_free(op1);
        return op2;
    }
}

SRA_t *SRA_makeDistinct(SRA_t *sra)
{
    if (sra->t != SRA_PROJECT)
    {
        fprintf(stderr, "Error: distinct property only applies to Project\n");
    }
    else
    {
        sra->project.distinct = 1;
    }
    return sra;
}

JoinCondition_t *On(Condition_t *cond)
{
    JoinCondition_t *jc = (JoinCondition_t *)calloc(1, sizeof(JoinCondition_t));
    jc->t = JOIN_COND_ON;
    jc->on = cond;
    return jc;
}

JoinCondition_t *Using(StrList_t *col_list)
{
    JoinCondition_t *jc = (JoinCondition_t *)calloc(1, sizeof(JoinCondition_t));
    jc->t = JOIN_COND_USING;
    jc->col_list = col_list;
    return jc;
}

void ProjectOption_print(ProjectOption_t *op)
{
    if (op->order_by)
    {
        printf("Order by: (%p) ", op->order_by);
        Expression_print(op->order_by);
        printf(op->asc_desc == ORDER_BY_ASC ? " ascending" : " descending");
    }
    if (op->group_by)
    {
        printf("Group by: (%p) ", op->group_by);
        Expression_print(op->group_by);
    }
    if (!op->order_by && !op->group_by)
    {
        printf("Empty ProjectOption\n");
    }
}

void JoinCondition_free(JoinCondition_t *cond)
{
    switch (cond->t)
    {
    case JOIN_COND_ON:
        Condition_free(cond->on);
        break;
    case JOIN_COND_USING:
        StrList_free(cond->col_list);
        break;
    }
}

void SRA_free(SRA_t *sra)
{
    switch (sra->t)
    {
    case SRA_TABLE:
        TableReference_free(sra->table.ref);
        break;
    case SRA_PROJECT:
        SRA_free(sra->project.sra);
        Expression_freeList(sra->project.expr_list);
        Expression_free(sra->project.order_by);
        Expression_free(sra->project.group_by);
        break;
    case SRA_SELECT:
        SRA_free(sra->select.sra);
        Condition_free(sra->select.cond);
        break;
    case SRA_FULL_OUTER_JOIN:
    case SRA_LEFT_OUTER_JOIN:
    case SRA_RIGHT_OUTER_JOIN:
    case SRA_JOIN:
        SRA_free(sra->join.sra1);
        SRA_free(sra->join.sra2);
        if (sra->join.opt_cond)
            JoinCondition_free(sra->join.opt_cond);
        break;
    case SRA_NATURAL_JOIN:
    case SRA_UNION:
    case SRA_EXCEPT:
    case SRA_INTERSECT:
        SRA_free(sra->binary.sra1);
        SRA_free(sra->binary.sra2);
        break;
    }
    free(sra);
}

static RA_t *desugar_table(SRA_t *sra)
{
    /*
    an SRA table contains a TableReference_t, which can optionally
    have an alias. If there's no alias, we can simply generate
    an RA table with that name; otherwise, we'll apply a Rho operator
    to change its name.
    */
    if (!sra->table.ref->alias)
    {
        /* TODO: See comment in RA_Table */
        return RA_Table(sra->table.ref->table_name);
    }
    else
    {
        return
            RA_RhoTable(
                RA_Table(sra->table.ref->table_name),
                sra->table.ref->alias
            );
    }
}

static RA_t *desugar_project(SRA_t *sra)
{
    /*
    SRA project will have a list of expressions and a table to
    take them from, along with some things like order by,
    group by etc. We'll be ignoring all of these for now, as our
    RA machinery as-is doesn't support it. But it can be modified later.
    */
    return RA_Pi(SRA_desugar(sra->project.sra), sra->project.expr_list);
}

static RA_t *desugar_select(SRA_t *sra)
{
    /*
    This conversion is straightforward, since the structure is identical.
    */
    return RA_Sigma(SRA_desugar(sra->select.sra), sra->select.cond);
}

static RA_t *desugar_natural_join(SRA_t *sra)
{
    /*
    The natural join means joining on the condition that for all columns which
    have the same name, their values must also be equal.
    Consider tables t1, t2, with column sets cs1, cs2.
    Then let cs = cs1 intersect cs2, so cs is the set of columns they have in
    common.
    Then for every column c in cs, we want to make a condition: t1.c = t2.c.
    We apply this condition to the cross product of t1 and t2.
    Further, any pair of shared columns will be collapsed into one.
    Pseudocode:
       // start with cross product
       res = Cross(t1, t2)

       // find distinct and shared columns
       distinct1 = list of columns from t1 that are not in t2
       distinct2 = list of columns from t2 that are not in t1
       shared = list of columns that are shared

       // initial condition
       cond = Eq("t1." + shared[0], "t2." + shared[0])
       // all remaining columns ANDed on
       for i == 1 .. shared.length:
          cond = And(cond, Eq(t1.shared[i], t2.shared[i]))
       // update result with conditions
       res = Sigma(res, cond)

       // build columns to project
       to_project = []
       for col in distinct1:
          to_project += Term(col)
       for col in shared:
          to_project += Term(col)
       for col in distinct2:
          to_project += Term(col)
    */

    // /* start with cross product */
    // RA_t *res = RA_Cross(SRA_desugar(sra->binary.sra1),
    //                      SRA_desugar(sra->binary.sra2));
    // /* get t1\t2, t2\t1, t1 intersect t2 */
    // List_t distinct1 = columns_except(sra->binary.sra1->cols,
    //                                   sra->binary.sra2->cols);
    // List_t distinct2 = columns_except(sra->binary.sra2->cols,
    //                                   sra->binary.sra1->cols);
    // List_t shared = columns_intersect(sra->binary.sra1->cols,
    //                                   sra->binary.sra2->cols);
    // /* for later use in choosing which columns to project */
    // List_t to_project;
    // /* create a condition that shared columns have equal values */
    // Condition_t *cond = NULL;
    // int first = 1; /* see below for purpose */
    // while (shared.size > 0) {
    //    Column_t *shared_col = (Column_t *)list_removeFront(&shared);
    //    if (first) {
    //       cond = Eq(Term(shared_col->name)
    //    }
    // }
    return NULL;
}

RA_t *desugar_join(SRA_t *sra)
{
    return NULL;
}

RA_t *SRA_desugar(SRA_t *sra)
{
    RA_t *t1, *t2, *res;
    switch (sra->t)
    {
    case SRA_TABLE:
        res = desugar_table(sra);
        break;
    case SRA_PROJECT:
        res = desugar_project(sra);
        break;
    case SRA_SELECT:
        res = desugar_select(sra);
        break;
    case SRA_NATURAL_JOIN:
        res = desugar_natural_join(sra);
        break;
    case SRA_JOIN:
        res = desugar_join(sra);
        break;
    case SRA_FULL_OUTER_JOIN:
        res = desugar_join(sra);
        break;
    case SRA_LEFT_OUTER_JOIN:
        res = desugar_join(sra);
        break;
    case SRA_RIGHT_OUTER_JOIN:
        res = desugar_join(sra);
        break;
    case SRA_UNION:
        return RA_Union(SRA_desugar(sra->binary.sra1),
                        SRA_desugar(sra->binary.sra2));
    case SRA_EXCEPT:
        return RA_Union(SRA_desugar(sra->binary.sra1),
                        SRA_desugar(sra->binary.sra2));
    case SRA_INTERSECT:
        t1 = SRA_desugar(sra->binary.sra1);
        t2 = SRA_desugar(sra->binary.sra2);
        return RA_Difference(
                   RA_Union(t1, t2),
                   RA_Difference(
                       RA_Difference(t1, t2),
                       RA_Difference(t2, t1)));
    default:
        fprintf(stderr, "Error: unhandled SRA type\n");
        exit(1);
    }
    SRA_free(sra);
    return res;
}
/*
RA_t *SRA_desugar(SRA_t *sra) {
   List_t temp_tables;
   RA_t *res;
   list_init(&temp_tables, Table_free);
   res = SRA_desugar_r(sra, temp_tables);
   list_destroy(&temp_tables);
   return res;
}*/
