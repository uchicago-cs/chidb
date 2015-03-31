#include <unistd.h>
#include <string.h>
#include <chidb/dbm-file.h>
#include "shell.h"
#include "commands.h"
#include <chisql/chisql.h>

#define COL_SEPARATOR "|"

struct handler_entry handlers[] =
{
    HANDLER_ENTRY (parse),
    HANDLER_ENTRY (dbmrun),

    NULL_ENTRY
};


int chidb_shell_handle_cmd(chidb_shell_ctx_t *ctx, const char *cmd)
{
    int rc = 0;

    if(cmd[0] == '.')
    {
        int h;

        for(h=0; handlers[h].name != NULL; h++)
        {
            if (!strncmp(cmd+1, handlers[h].name, handlers[h].name_len))
            {
                rc = handlers[h].func(ctx, cmd + handlers[h].name_len + 2);
                break;
            }
        }

        if (handlers[h].name == NULL)
        {
            fprintf(stderr, "ERROR: Unrecognized command: %s\n", cmd);
            return 1;
        }
        else
        {
            return rc;
        }
    }
    else
    {
        /* Anything that is not a command is a SQL statement,
         * which requires a DB file. */
        if(!ctx->db)
        {
            fprintf(stderr, "ERROR: No database is open.\n");
            return 1;
        }
        else
        {
            rc = chidb_shell_handle_sql(ctx, cmd);
            return rc;
        }
    }

    return 0;
}

int chidb_shell_handle_sql(chidb_shell_ctx_t *ctx, const char *sql)
{
    int rc;
    chidb_stmt *stmt;

    rc = chidb_prepare(ctx->db, sql, &stmt);

    if (rc == CHIDB_OK)
    {
        int numcol = chidb_column_count(stmt);
        for(int i = 0; i < numcol; i ++)
        {
            printf(i==0?"":COL_SEPARATOR);
            printf("%s", chidb_column_name(stmt,i));
        }
        printf("\n");

        while((rc = chidb_step(stmt)) == CHIDB_ROW)
        {
            for(int i = 0; i < numcol; i++)
            {
                printf(i==0?"":COL_SEPARATOR);
                switch(chidb_column_type(stmt,i))
                {
                case SQL_NULL:
                    break;
                case SQL_INTEGER_1BYTE:
                case SQL_INTEGER_2BYTE:
                case SQL_INTEGER_4BYTE:
                    printf("%i", chidb_column_int(stmt,i));
                    break;
                case SQL_TEXT:
                    printf("%s", chidb_column_text(stmt,i));
                    break;
                }
            }
            printf("\n");
        }

        switch(rc)
        {
        case CHIDB_ECONSTRAINT:
            printf("ERROR: SQL statement failed because of a constraint violation.\n");
            break;
        case CHIDB_EMISMATCH:
            printf("ERROR: Data type mismatch.\n");
            break;
        case CHIDB_EMISUSE:
            printf("ERROR: API used incorrectly.\n");
            break;
        case CHIDB_EIO:
            printf("ERROR: An I/O error has occurred when accessing the file.\n");
            break;
        }

        rc = chidb_finalize(stmt);
        if(rc == CHIDB_EMISUSE)
            printf("API used incorrectly.\n");
    }
    else if (rc == CHIDB_EINVALIDSQL)
        printf("SQL syntax error.\n");
    else if (rc == CHIDB_ENOMEM)
        printf("ERROR: Could not allocate memory.\n");

    return rc;
}


int chidb_shell_handle_cmd_parse(chidb_shell_ctx_t *ctx, const char *s)
{
    chisql_statement_t *sql_stmt;
    int rc;

    printf("Provided SQL: %s\n", s);

    rc = chisql_parser(s, &sql_stmt);

    if (rc != CHIDB_OK)
        return rc;

    printf("  Parsed SQL   \n");
    printf("---------------\n");
    chisql_stmt_print(sql_stmt);
    printf("\n");

    return 0;
}

int chidb_shell_handle_cmd_dbmrun(chidb_shell_ctx_t *ctx, const char *s)
{
    int rc;
    chidb_dbm_file_t *dbmf;

    if(access(s, F_OK) == -1)
    {
        fprintf(stderr, "ERROR: File does not exist: ##%s##\n", s);
        return 1;
    }

    rc = chidb_dbm_file_load(s, &dbmf, ctx->db);

    if(rc != CHIDB_OK)
    {
        fprintf(stderr, "ERROR: Could not load DBM file %s\n", s);
        return 1;
    }

    rc = chidb_dbm_file_run(dbmf);

    if(rc != CHIDB_OK)
    {
        fprintf(stderr, "ERROR: Could not run DBM file %s\n", s);
        return 1;
    }

    chidb_dbm_file_print_program(dbmf);

    rc = chidb_dbm_file_close(dbmf);

    return 0;
}


