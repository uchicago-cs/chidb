#include <stdio.h>
#include <check.h>
#include <ctype.h>
#include <stdlib.h>
#include "chidb.h"
#include "check_common.h"
#include "libchidb/vdbe.h"

#define MAX_LINE_LEN (256)

#define LINE_OK          (0)
#define LINE_SKIP        (1)
#define LINE_EOF         (2)
#define LINE_NEW_SECTION (3)

typedef enum vdbe_file_sections
{
    CHIDB_FILE = 0,
    PROGRAM = 1,
    QUERY_RESULT = 2,
    REGISTERS = 3
} vdbe_file_sections_t;

int read_line(FILE *f, char* line)
{
    char* rc;

    rc = fgets(line, MAX_LINE_LEN, f);

    if(rc == NULL)
        return LINE_EOF;

    line[strcspn(line, "\n")] = '\0';

    while(isspace(*line)) line++;

    if(line[0] == '#')
        return LINE_SKIP;

    if(line[0] == '\0')
        return LINE_SKIP;

    if(line[0] == '%' && line[1] == '%')
        return LINE_NEW_SECTION;

    return LINE_OK;

}

int tokenize(char *str, char ***tokens)
{
    char *s;
    int ntokens = 0;

    s = str;
    if (s==NULL)
        return CHIDB_ENOMEM;

    /* First pass: Add \0 at the end of each token
     * and count the number of tokens */
    while(isspace(*s)) s++;

    while(*s != '\0')
    {
        ntokens++;
        if (*s == '"')
        {
            s++;
            while(*s && *s != '"') s++;
        }
        else
            while(*s && !isspace(*s)) s++;

        if(*s != '\0')
        {
            *s++ = '\0';
            while(*s && isspace(*s)) s++;
        }
    }


    /* Second pass: Create the array of tokens */
    *tokens = malloc(sizeof(char**) * ntokens);

    s = str;
    while(isspace(*s)) s++;
    for(int i=0; i<ntokens; i++)
    {
        if (*s == '"')
        {
            s++;
            (*tokens)[i] = s;
            while(*s && *s != '"') s++;
        }
        else
        {
            (*tokens)[i] = s;
            while(*s && !isspace(*s)) s++;
        }

        s++;
        while(*s && isspace(*s)) s++;
    }

    return ntokens;
}


int prepare_file(char *line, chidb **db, char **fname, bool *delete)
{
    char *linedup;
    char **tokens;
    int ntokens;
    int rc;

    linedup = strdup(line);
    if (linedup==NULL)
        return CHIDB_ENOMEM;

    ntokens = tokenize(linedup, &tokens);

    if(ntokens != 2)
        return 1;

    if (strcmp(tokens[0], "NO") == 0 && strcmp(tokens[0], "DBFILE"))
    {
        *fname = create_tmp_file();
        *delete = true;
    }
    else if (strcmp(tokens[0], "CREATE") == 0)
    {
        *fname = generated_file_path(tokens[1]);
        remove(*fname);
        *delete = false;
    }
    else if (strcmp(tokens[0], "USE") == 0)
    {
        *fname = create_copy(tokens[1], tokens[1]);
        *delete = true;
    }
    else
        return 1;

    rc = chidb_open(*fname, db);
    ck_assert_msg(rc == CHIDB_OK, "Could not open chidb file %s", line);


    free(linedup);
    return 0;
}

int parse_instruction(char *line, chidb_vdbe_op_t *op)
{
    char *linedup;
    char **tokens;
    int ntokens;

    linedup = strdup(line);
    if (linedup==NULL)
        return CHIDB_ENOMEM;

    ntokens = tokenize(linedup, &tokens);

    if(ntokens != 5)
        return 1;

    int opcode = str_to_opcode(tokens[0]);

    if(opcode == -1)
        return 1;

    op->opcode = opcode;
    op->p1 = tokens[1][0]=='_' ? 0 : atoi(tokens[1]);
    op->p2 = tokens[2][0]=='_' ? 0 : atoi(tokens[2]);
    op->p3 = tokens[3][0]=='_' ? 0 : atoi(tokens[3]);
    op->p4 = tokens[4][0]=='_' ? NULL : strdup(tokens[4]);

    free(linedup);
    return 0;
}

int check_register(char *line, chidb_stmt *stmt)
{
    char *linedup;
    char **tokens;
    int ntokens, regN;
    chidb_vdbe_register_t *reg;

    linedup = strdup(line);
    if (linedup==NULL)
        return CHIDB_ENOMEM;

    ntokens = tokenize(linedup, &tokens);

    if(!(ntokens == 2 || ntokens == 3))
        return 1;

    if (tokens[0][0] != 'R' && tokens[0][1] != '_')
        return 1;

    /* TODO: Validate register index */
    regN = atoi(&tokens[0][2]);
    reg = &stmt->reg[regN];

    ck_assert_msg(strcmp(tokens[1], "unspecified") == 0 ||
                  strcmp(tokens[1], "null") == 0 ||
                  strcmp(tokens[1], "integer") == 0 ||
                  strcmp(tokens[1], "string") == 0 ||
                  strcmp(tokens[1], "binary") == 0,
                  "Unknown register type: %s", tokens[1]);

    switch(reg->type)
    {
    case REG_UNSPECIFIED:
        ck_assert_msg(strcmp(tokens[1], "unspecified") == 0,
                "Expected register %i to be of type %s, but it is of type unspecified", regN, tokens[1]);
        break;
    case REG_NULL:
        ck_assert_msg(strcmp(tokens[1], "null") == 0,
                "Expected register %i to be of type %s, but it is of type null", regN, tokens[1]);
        break;
    case REG_INT32:
        ck_assert_msg(strcmp(tokens[1], "integer") == 0,
                "Expected register %i to be of type %s, but it is of type integer", regN, tokens[1]);
        if(ntokens == 3)
            ck_assert_msg(atoi(tokens[2]) == reg->value.i,
                    "Expected register %i to have value %s but it has value %i", regN, tokens[2], reg->value.i);
        break;
    case REG_STRING:
        ck_assert_msg(strcmp(tokens[1], "string") == 0,
                "Expected register %i to be of type %s, but it is of type string", regN, tokens[1]);
        if(ntokens == 3)
            ck_assert_msg(strcmp(tokens[2], reg->value.s) == 0,
                    "Expected register %i to have value '%s' but it has value '%s'", regN, tokens[2], reg->value.s);
        break;
    case REG_BINARY:
        ck_assert_msg(strcmp(tokens[1], "binary") == 0,
                "Expected register %i to be of type %s, but it is of type binary", regN, tokens[1]);
        /* TODO: Check value too */
        break;
    }


    return 0;
}

void run_vdbe_file(const char* filename)
{
    FILE *f;
    chidb *db = NULL;
    chidb_stmt stmt;
    chidb_vdbe_op_t op;
    char *fname;
    bool delete;
    char line[MAX_LINE_LEN + 1];
    int rc, exec_rc, opnum = 0;
    vdbe_file_sections_t section = CHIDB_FILE;

    printf("filename: %s\n", filename);
    f = fopen(filename, "r");

    ck_assert_msg(f != NULL, "Could not open file %s", filename);

    while(1)
    {
        while((rc = read_line(f, line)) == LINE_SKIP)
            ;

        if(rc == LINE_EOF)
            break;

        if(rc == LINE_NEW_SECTION)
        {
            if (section == PROGRAM)
            {
                exec_rc = chidb_stmt_exec(&stmt);
                ck_assert_msg(exec_rc == CHIDB_DONE || exec_rc == CHIDB_ROW, "Error executing statement (rc=%i)", rc);
            }
            section++;
            continue;
        }

        switch(section)
        {
        case CHIDB_FILE:
            rc = prepare_file(line, &db, &fname, &delete);
            ck_assert_msg(rc == 0, "Error parsing line: '%s'", line);
            rc = chidb_stmt_init(&stmt, db);
            ck_assert_msg(rc == CHIDB_OK, "Could not create VDBE");
            break;
        case PROGRAM:
            rc = parse_instruction(line, &op);
            ck_assert_msg(rc == 0, "Error parsing line: '%s'", line);
            chidb_stmt_set_op(&stmt, &op, opnum++);
            break;
        case QUERY_RESULT:
            ck_assert_msg(exec_rc == CHIDB_ROW, "VDBE file include query results, but program produced no rows.");
            break;
        case REGISTERS:
            rc = check_register(line, &stmt);
            ck_assert_msg(rc == 0, "Error parsing line: '%s'", line);
            break;

        }
    }

    chidb_stmt_print(&stmt);

    if(delete && fname)
        remove(fname);

    if(db)
        free(db);
}
