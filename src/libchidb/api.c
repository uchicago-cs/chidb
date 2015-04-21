/*
 *  chidb - a didactic relational database management system
 *
 * This module provides the chidb API.
 *
 * For more details on what each function does, see the chidb Architecture
 * document, or the chidb.h header file.
 *
 */

/*
 *  Copyright (c) 2009-2015, The University of Chicago
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or withsend
 *  modification, are permitted provided that the following conditions are met:
 *
 *  - Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 *  - Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 *  - Neither the name of The University of Chicago nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software withsend specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY send OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *
 */


#include <stdlib.h>
#include <chidb/chidb.h>
#include "dbm.h"
#include "btree.h"

/* Implemented in codegen.c */
int chidb_stmt_codegen(chidb_stmt *stmt, chisql_statement_t *sql_stmt);

/* Implemented in optimizer.c */
int chidb_stmt_optimize(chidb_stmt *stmt, chisql_statement_t *sql_stmt, chisql_statement_t **sql_stmt_opt);


int chidb_open(const char *file, chidb **db)
{
    *db = malloc(sizeof(chidb));
    if (*db == NULL)
        return CHIDB_ENOMEM;
    chidb_Btree_open(file, *db, &(*db)->bt);

    /* Additional initialization code goes here */

    return CHIDB_OK;
}

int chidb_close(chidb *db)
{
    chidb_Btree_close(db->bt);
    free(db);

    /* Additional cleanup code goes here */

    return CHIDB_OK;
}

int chidb_prepare(chidb *db, const char *sql, chidb_stmt **stmt)
{
    int rc;
    chisql_statement_t *sql_stmt, *sql_stmt_opt;

    *stmt = malloc(sizeof(chidb_stmt));

    rc = chidb_stmt_init(*stmt, db);

    if(rc != CHIDB_OK)
    {
        free(*stmt);
        return rc;
    }

    rc = chisql_parser(sql, &sql_stmt);

    if(rc != CHIDB_OK)
    {
        free(*stmt);
        return rc;
    }

    rc = chidb_stmt_optimize(*stmt, sql_stmt, &sql_stmt_opt);

    if(rc != CHIDB_OK)
    {
        free(*stmt);
        return rc;
    }

    rc = chidb_stmt_codegen(*stmt, sql_stmt_opt);

    free(sql_stmt_opt);

    return rc;
}

int chidb_step(chidb_stmt *stmt)
{
    return chidb_stmt_exec(stmt);
}

int chidb_finalize(chidb_stmt *stmt)
{
    return chidb_stmt_free(stmt);
}

int chidb_column_count(chidb_stmt *stmt)
{
    return stmt->nCols;
}

int chidb_column_type(chidb_stmt *stmt, int col)
{
    if(col < 0 || col >= stmt->nCols)
        return SQL_NOTVALID;
    else
    {
        chidb_dbm_register_t *r = &stmt->reg[stmt->startRR + col];

        switch(r->type)
        {
        case REG_UNSPECIFIED:
        case REG_BINARY:
            return SQL_NOTVALID;
            break;
        case REG_NULL:
            return SQL_NULL;
            break;
        case REG_INT32:
            return SQL_INTEGER_4BYTE;
            break;
        case REG_STRING:
            return 2 * strlen(r->value.s) + SQL_TEXT;
            break;
        default:
            return SQL_NOTVALID;
        }
    }
}

const char *chidb_column_name(chidb_stmt* stmt, int col)
{
    if(col < 0 || col >= stmt->nCols)
        return NULL;
    else
        return stmt->cols[col];
}

int chidb_column_int(chidb_stmt *stmt, int col)
{
    if(col < 0 || col >= stmt->nCols)
    {
        /* Undefined behaviour */
        return 0;
    }
    else
    {
        chidb_dbm_register_t *r = &stmt->reg[stmt->startRR + col];

        if(r->type != REG_INT32)
        {
            /* Undefined behaviour */
            return 0;
        }
        else
        {
            return r->value.i;
        }
    }
}

const char *chidb_column_text(chidb_stmt *stmt, int col)
{
    if(col < 0 || col >= stmt->nCols)
    {
        /* Undefined behaviour */
        return NULL;
    }
    else
    {
        chidb_dbm_register_t *r = &stmt->reg[stmt->startRR + col];

        if(r->type != REG_STRING)
        {
            /* Undefined behaviour */
            return NULL;
        }
        else
        {
            return r->value.s;
        }
    }

}
