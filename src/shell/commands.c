#include <unistd.h>
#include <string.h>
#include <chidb/dbm-file.h>
#include "shell.h"
#include "commands.h"
#include <chisql/chisql.h>
#include <chidb/utils.h>


#define COL_SEPARATOR "|"

struct handler_entry handlers[] =
{
    HANDLER_ENTRY (open,      ".open FILENAME     Close existing database (if any) and open FILENAME"),
    HANDLER_ENTRY (parse,     ".parse \"SQL\"       Show parse tree for statement SQL"),
    HANDLER_ENTRY (opt,       ".opt \"SQL\"       Show parse tree and optimized parse tree for statement SQL"),
    HANDLER_ENTRY (dbmrun,    ".dbmrun DBMFILE    Run DBM program in DBMFILE"),
    HANDLER_ENTRY (headers,   ".headers on|off    Switch display of headers on or off in query results"),
    HANDLER_ENTRY (mode,      ".mode MODE         Switch display mode. MODE is one of:\n"
    		                  "                     column  Left-aligned columns\n"
    		                  "                     list    Values delimited by | (default)"),
    HANDLER_ENTRY (explain,   ".explain on|off    Turn output mode suitable for EXPLAIN on or off."),
    HANDLER_ENTRY (help,      ".help              Show this message"),

    NULL_ENTRY
};


void usage_error(struct handler_entry *e, const char *msg)
{
    fprintf(stderr, "ERROR: %s\n", msg);
    fprintf(stderr, "%s\n", e->help);
}

int chidb_shell_handle_cmd(chidb_shell_ctx_t *ctx, const char *cmd)
{
    int rc = 0;

    if(cmd[0] == '.')
    {
        int h, ntokens;
        char *cmddup = strdup(cmd), **tokens;

        ntokens = chidb_tokenize(cmddup, &tokens);

        for(h=0; handlers[h].name != NULL; h++)
        {
            if (!strncmp(tokens[0]+1, handlers[h].name, handlers[h].name_len))
            {
                rc = handlers[h].func(ctx, &handlers[h], (const char**) tokens, ntokens);
                break;
            }
        }

        if (handlers[h].name == NULL)
        {
            fprintf(stderr, "ERROR: Unrecognized command: %s\n", tokens[0]);
            free(tokens);
            free(cmddup);
            return 1;
        }
        else
        {
            free(tokens);
            free(cmddup);
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

        if(ctx->header)
        {
            for(int i = 0; i < numcol; i ++)
            {
                if(ctx->mode == MODE_LIST)
                {
                    printf(i==0?"":COL_SEPARATOR);
                    printf("%s", chidb_column_name(stmt,i));
                }
                else if(ctx->mode == MODE_COLUMN)
                {
                    printf(i==0?"":" ");
                    printf("%-10.10s", chidb_column_name(stmt,i));
                }
            }
            printf("\n");

            if(ctx->mode == MODE_COLUMN)
            {
                for(int i = 0; i < numcol; i ++)
                {
                   printf(i==0?"":" ");
                   printf("----------");
                }
                printf("\n");
            }
        }

        while((rc = chidb_step(stmt)) == CHIDB_ROW)
        {
            for(int i = 0; i < numcol; i++)
            {
                int coltype;

                if(ctx->mode == MODE_LIST)
                    printf(i==0?"":COL_SEPARATOR);
                else if (ctx->mode == MODE_COLUMN)
                    printf(i==0?"":" ");


                coltype = chidb_column_type(stmt,i);

                if(coltype == SQL_NOTVALID)
                {
                    printf("ERROR: Column %i return an invalid type.\n", coltype);
                    break;
                }
                else if(coltype == SQL_INTEGER_1BYTE || coltype == SQL_INTEGER_2BYTE || coltype == SQL_INTEGER_4BYTE)
                {
                    if(ctx->mode == MODE_LIST)
                        printf("%i", chidb_column_int(stmt,i));
                    else if (ctx->mode == MODE_COLUMN)
                        printf("%10i", chidb_column_int(stmt,i));
                }
                else if(coltype == SQL_NULL)
                {
                    /* Print nothing */
                    if (ctx->mode == MODE_COLUMN)
                        printf("          ");
                }
                else
                {
                    int len;
                    if((coltype - 13) % 2 != 0)
                    {
                        printf("ERROR: Column %i returned an invalid type.\n", i);
                        break;
                    }
                    const char *text = chidb_column_text(stmt,i);
                    len = strlen(text);
                    if(len != (coltype-13)/2)
                    {
                        printf("ERROR: THe length (%i) of the text in column %i does not match its type (%i).\n", len, i, coltype);
                        break;
                    }

                    if(ctx->mode == MODE_LIST)
                        printf("%s", text);
                    else if (ctx->mode == MODE_COLUMN)
                        printf("%-10.10s", text);
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


int chidb_shell_handle_cmd_open(chidb_shell_ctx_t *ctx, struct handler_entry *e, const char **tokens, int ntokens)
{
    int rc;
    chidb *newdb;

    if(ntokens != 2)
    {
    	usage_error(e, "Invalid arguments");
    	return 1;
    }

    rc = chidb_open(tokens[1], &newdb);

	if (rc != CHIDB_OK)
    {
        fprintf(stderr, "ERROR: Could not open file %s or file is not well formed.\n", tokens[1]);
        return rc;
    }


    if(ctx->db)
    {
    	chidb_close(ctx->db);
    	free(ctx->dbfile);
    }

    ctx->db = newdb;
    ctx->dbfile = strdup(tokens[1]);

    return CHIDB_OK;
}


int chidb_shell_handle_cmd_parse(chidb_shell_ctx_t *ctx, struct handler_entry *e, const char **tokens, int ntokens)
{
    chisql_statement_t *sql_stmt;
    int rc;

    if(ntokens != 2)
    {
    	usage_error(e, "Invalid arguments");
    	return 1;
    }

    rc = chisql_parser(tokens[1], &sql_stmt);

    if (rc != CHIDB_OK)
    {
        return rc;
    }

    chisql_stmt_print(sql_stmt);
    printf("\n");

    return CHIDB_OK;
}

/* Implemented in optimizer.c */
int chidb_stmt_optimize(chidb *db,
            chisql_statement_t *sql_stmt,
            chisql_statement_t **sql_stmt_opt);

int chidb_shell_handle_cmd_opt(chidb_shell_ctx_t *ctx, struct handler_entry *e, const char **tokens, int ntokens)
{
    chisql_statement_t *sql_stmt, *sql_stmt_opt;
    int rc;

    if(ntokens != 2)
    {
        usage_error(e, "Invalid arguments");
        return 1;
    }

    if(!ctx->db)
    {
        fprintf(stderr, "ERROR: No database is open.\n");
        return 1;
    }

    rc = chisql_parser(tokens[1], &sql_stmt);

    if (rc != CHIDB_OK)
    {
        return rc;
    }

    chisql_stmt_print(sql_stmt);
    printf("\n\n");

    rc = chidb_stmt_optimize(ctx->db, sql_stmt, &sql_stmt_opt);

    if(rc != CHIDB_OK)
    {
        return rc;
    }

    chisql_stmt_print(sql_stmt_opt);
    printf("\n");

    return CHIDB_OK;
}

int chidb_shell_handle_cmd_dbmrun(chidb_shell_ctx_t *ctx, struct handler_entry *e, const char **tokens, int ntokens)
{
    int rc;
    chidb_dbm_file_t *dbmf;
    bool results = false;

    if(ntokens != 2)
    {
    	usage_error(e, "Invalid arguments");
    	return 1;
    }

    if(access(tokens[1], F_OK) == -1)
    {
        fprintf(stderr, "ERROR: File does not exist: %s\n", tokens[1]);
        return 1;
    }

    rc = chidb_dbm_file_load(tokens[1], &dbmf, ctx->db);

    if(rc != CHIDB_OK)
    {
        fprintf(stderr, "ERROR: Could not load DBM file %s\n", tokens[1]);
        return 1;
    }

    do
    {
        rc = chidb_dbm_file_run(dbmf);

        if(rc == CHIDB_ROW)
        {
            if(!results)
            {
                printf("RESULT ROWS\n");
                printf("-----------\n");
                results = true;
            }

            chidb_dbm_file_print_rr(dbmf);
            printf("\n");
        }
        else if (rc != CHIDB_DONE)
        {
            fprintf(stderr, "ERROR: Error while running DBM file %s\n", tokens[1]);
            return 1;
        }
    } while (rc != CHIDB_DONE);

    if(results)
        printf("\n");
    else
    {
        printf("This program produced no result rows.\n\n");
    }

    chidb_dbm_file_print_program(dbmf);

    rc = chidb_dbm_file_close(dbmf);

    return 0;
}

int chidb_shell_handle_cmd_headers(chidb_shell_ctx_t *ctx, struct handler_entry *e, const char **tokens, int ntokens)
{
    if(ntokens != 2)
    {
    	usage_error(e, "Invalid arguments");
    	return 1;
    }

    if(strcmp(tokens[1],"on")==0)
        ctx->header = true;
    else if(strcmp(tokens[1],"off")==0)
        ctx->header = false;
    else
    {
    	usage_error(e, "Invalid argument");
    	return 1;
    }

    return CHIDB_OK;
}

int chidb_shell_handle_cmd_mode(chidb_shell_ctx_t *ctx, struct handler_entry *e, const char **tokens, int ntokens)
{
    if(ntokens != 2)
    {
    	usage_error(e, "Invalid arguments");
    	return 1;
    }

    if(strcmp(tokens[1],"list")==0)
        ctx->mode = MODE_LIST;
    else if(strcmp(tokens[1],"column")==0)
        ctx->mode = MODE_COLUMN;
    else
    {
    	usage_error(e, "Invalid argument");
    	return 1;
    }

    return CHIDB_OK;
}

int chidb_shell_handle_cmd_explain(chidb_shell_ctx_t *ctx, struct handler_entry *e, const char **tokens, int ntokens)
{
    if(ntokens != 2)
    {
    	usage_error(e, "Invalid arguments");
    	return 1;
    }

    if(strcmp(tokens[1],"on")==0)
    {
        ctx->header = true;
        ctx->mode = MODE_COLUMN;
    }
    else if(strcmp(tokens[1],"off")==0)
    {
        ctx->header = false;
        ctx->mode = MODE_LIST;
    }
    else
    {
    	usage_error(e, "Invalid argument");
    	return 1;
    }

    return CHIDB_OK;
}

int chidb_shell_handle_cmd_help(chidb_shell_ctx_t *ctx, struct handler_entry *e, const char **tokens, int ntokens)
{
    for(int h=0; handlers[h].name != NULL; h++)
    {
    	fprintf(stderr, "%s\n", handlers[h].help);
    }

    return CHIDB_OK;
}

