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
#include "btree.h"

/* This file currently provides only a very basic implementation
 * of chidb_open and chidb_close that are sufficient to test
 * the B Tree module. The rest of the functions are left
 * as an exercise to the student. chidb_open itself may need to be
 * modified if other chidb modules are implemented.
 */

int chidb_open(const char *file, chidb **db)
{
    *db = malloc(sizeof(chidb));
    if (*db == NULL)
        return CHIDB_ENOMEM;
    chidb_Btree_open(file, *db, &(*db)->bt);

    return CHIDB_OK;
}

int chidb_close(chidb *db)
{
    chidb_Btree_close(db->bt);
    free(db);
    return CHIDB_OK;
}

int chidb_prepare(chidb *db, const char *sql, chidb_stmt **stmt)
{
    /* Your code goes here */

    return CHIDB_OK;
}

int chidb_step(chidb_stmt *stmt)
{
    /* Your code goes here */

    return CHIDB_OK;
}

int chidb_finalize(chidb_stmt *stmt)
{
    /* Your code goes here */

    return CHIDB_OK;
}

int chidb_column_count(chidb_stmt *stmt)
{
    /* Your code goes here */

    return 0;
}

int chidb_column_type(chidb_stmt *stmt, int col)
{
    /* Your code goes here */

    return SQL_NOTVALID;
}

const char *chidb_column_name(chidb_stmt* stmt, int col)
{
    /* Your code goes here */

    return "";
}

int chidb_column_int(chidb_stmt *stmt, int col)
{
    /* Your code goes here */

    return 0;
}

const char *chidb_column_text(chidb_stmt *stmt, int col)
{
    /* Your code goes here */

    return "";
}
