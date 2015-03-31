/*
 *  chidb - a didactic relational database management system
 *
 *  Database Machine
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

#include <assert.h>
#include <stdbool.h>
#include "dbm.h"


int realloc_ops(chidb_stmt *stmt, uint32_t size);
int realloc_reg(chidb_stmt *stmt, uint32_t size);
int realloc_cur(chidb_stmt *stmt, uint32_t size);

int chidb_stmt_init(chidb_stmt *stmt, chidb *db)
{
    assert(stmt != NULL);
    assert(db != NULL);

    int rc;

    stmt->db = db;
    stmt->sql = NULL;
    stmt->explain = false;

    stmt->pc = 0;

    stmt->ops = NULL;
    stmt->nOps = 0;
    stmt->endOp = 0;
    rc = realloc_ops(stmt, DEFAULT_OPS_SIZE);
    if(rc != CHIDB_OK)
        return rc;

    stmt->reg = NULL;
    stmt->nReg = 0;
    rc = realloc_reg(stmt, DEFAULT_REG_SIZE);
    if(rc != CHIDB_OK)
        return rc;

    stmt->cursors = NULL;
    stmt->nCursors = 0;
    rc = realloc_cur(stmt, DEFAULT_CUR_SIZE);
    if(rc != CHIDB_OK)
        return rc;

    return CHIDB_OK;
}

int chidb_stmt_set_op(chidb_stmt *stmt, chidb_dbm_op_t *op, uint32_t pos)
{
    if (pos >= stmt->nOps)
    {
        int rc = realloc_ops(stmt, pos + 1);
        if (rc != CHIDB_OK)
            return rc;
    }

    memcpy(&stmt->ops[pos], op, sizeof(chidb_dbm_op_t));

    if(op->p4 != NULL)
        stmt->ops[pos].p4 = strdup(op->p4);

    if(pos >= stmt->endOp)
        stmt->endOp = pos + 1;

    return CHIDB_OK;
}

int chidb_dbm_op_handle (chidb_stmt *stmt, chidb_dbm_op_t *op);

int chidb_stmt_exec(chidb_stmt *stmt)
{
    int rc = -1;

    while(stmt->pc < stmt->endOp)
    {
        chidb_dbm_op_t *op = &stmt->ops[stmt->pc++];
        rc = chidb_dbm_op_handle(stmt, op);

        if (rc != CHIDB_OK)
            break;
    }

    if (rc == CHIDB_OK)
        rc = CHIDB_DONE;

    return rc;
}

int chidb_stmt_op_print(chidb_dbm_op_t *op)
{
    char *p4;

    p4 = op->p4 == NULL ? "NULL" : op->p4;

    printf("%-15s %-6i %-6i %-6i ", opcode_to_str(op->opcode),
           op->p1,
           op->p2,
           op->p3);

    if (op->p4 == NULL)
        printf("NULL\n");
    else
        printf("\"%s\"\n", p4);

    return CHIDB_OK;
}

int chidb_stmt_reg_print(chidb_dbm_register_t *r)
{
    switch(r->type)
    {
    case REG_UNSPECIFIED:
        break;
    case REG_NULL:
        printf("NULL");
        break;
    case REG_INT32:
        printf("%i", r->value.i);
        break;
    case REG_STRING:
        printf("\"%s\"", r->value.s);
        break;
    case REG_BINARY:
        printf("(%i bytes)", r->value.bin.nbytes);
        break;
    }

    return CHIDB_OK;
}

int chidb_stmt_print(chidb_stmt *stmt)
{
    printf("opcode          P1     P2     P3     P4\n");
    printf("--------------- ------ ------ ------ ------\n");
    for(int i=0; i < stmt->endOp; i++)
    {
        chidb_stmt_op_print(&stmt->ops[i]);
    }
    printf("--------------- ------ ------ ------ ------\n\n");

    printf("REGISTERS\n");
    printf("---------\n");
    bool somevalue = false;
    for(int i=0; i < stmt->nReg; i++)
    {
        chidb_dbm_register_t *r = &stmt->reg[i];

        if(r->type != REG_UNSPECIFIED)
        {
            somevalue = true;
            printf("R_%i = ", i);
            chidb_stmt_reg_print(r);
            printf("\n");
        }
    }

    if(!somevalue)
        printf("None of the registers have values\n");

    return CHIDB_OK;
}

int realloc_ops(chidb_stmt *stmt, uint32_t size)
{
    stmt->ops = realloc(stmt->ops, sizeof(chidb_dbm_op_t) * size);
    if(stmt->ops == NULL)
        return CHIDB_ENOMEM;

    for(int i=stmt->nOps; i < size; i++)
    {
        stmt->ops[i].opcode = Op_Noop;
        stmt->ops[i].p1 = 0;
        stmt->ops[i].p2 = 0;
        stmt->ops[i].p3 = 0;
        stmt->ops[i].p4 = NULL;
    }

    stmt->nOps = size;

    return CHIDB_OK;
}

int realloc_reg(chidb_stmt *stmt, uint32_t size)
{
    stmt->reg = realloc(stmt->reg, sizeof(chidb_dbm_register_t) * size);
    if(stmt->reg == NULL)
        return CHIDB_ENOMEM;

    for(int i=stmt->nReg; i < size; i++)
    {
        stmt->reg[i].type = REG_UNSPECIFIED;
    }

    stmt->nReg = size;

    return CHIDB_OK;
}

int realloc_cur(chidb_stmt *stmt, uint32_t size)
{
    stmt->cursors = realloc(stmt->cursors, sizeof(chidb_dbm_cursor_t) * size);
    if(stmt->cursors == NULL)
        return CHIDB_ENOMEM;

    for(int i=stmt->nCursors; i < size; i++)
    {
        stmt->cursors[i].type = CURSOR_UNSPECIFIED;
    }

    stmt->nCursors = size;

    return CHIDB_OK;
}

