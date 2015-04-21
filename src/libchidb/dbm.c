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

/* Forward declaration of auxiliary functions. */
int realloc_ops(chidb_stmt *stmt, uint32_t size);
int realloc_reg(chidb_stmt *stmt, uint32_t size);
int realloc_cur(chidb_stmt *stmt, uint32_t size);



/* Initialize a DBM
 *
 * Creates an empty DBM with no instructions, no registers, and
 * no cursors. It does, however, allocate memory for a default
 * number of instructions, registers, and cursors (which can be
 * increased if necessary using the realloc_* instructions)
 *
 * Parameters
 * - stmt: Pointer to chidb_stmt to be initialized. Assumes it
 *         points to enough memory to contain a chidb_stmt struct.
 * - db: The chidb database on which this DBM program will be run
 *
 * Return
 * - CHIDB_OK: Operation successful
 * - CHIDB_ENOMEM: Could not allocate memory
 */
int chidb_stmt_init(chidb_stmt *stmt, chidb *db)
{
    assert(stmt != NULL);
    assert(db != NULL);

    int rc;

    stmt->db = db;
    stmt->sql = NULL;
    stmt->explain = false;

    /* The program starts running in instruction 0 */
    stmt->pc = 0;

    /* We allocate an array of chidb_dbm_op_t's with enough room for
     * DEFAULT_OPS_SIZE instructions. This is done with realloc_ops,
     * which initializes the instructions to Noop's. Note that realloc_ops
     * updates nOps to be DEFAULT_OPS_SIZE, but endOp remains 0, because
     * we haven't added any actual instructions. */
    stmt->ops = NULL;
    stmt->nOps = 0;
    stmt->endOp = 0;
    rc = realloc_ops(stmt, DEFAULT_OPS_SIZE);
    if(rc != CHIDB_OK)
        return rc;

    /* Same as above, but with registers. */
    stmt->reg = NULL;
    stmt->nReg = 0;
    rc = realloc_reg(stmt, DEFAULT_REG_SIZE);
    if(rc != CHIDB_OK)
        return rc;

    /* Same as above, but with cursors. */
    stmt->cursors = NULL;
    stmt->nCursors = 0;
    rc = realloc_cur(stmt, DEFAULT_CUR_SIZE);
    if(rc != CHIDB_OK)
        return rc;

    /* Initially, there is no Result Row */
    stmt->startRR = 0;
    stmt->nRR = 0;
    stmt->cols = NULL;
    stmt->nCols = 0;

    return CHIDB_OK;
}

/* Free a DBM's resources
 *
 * Frees the resources associated with a statement.
 *
 * Parameters
 * - stmt: DBM to free
 *
 * Return
 * - CHIDB_OK: Operation successful
 */
int chidb_stmt_free(chidb_stmt *stmt)
{
	free(stmt->ops);
	free(stmt->reg);
	free(stmt->cursors);
    return CHIDB_OK;
}


/* Set the value of a specific instruction
 *
 * Given an instruction (of type chidb_dbm_op_t, which includes
 * the opcode as well as all the parameters), set instruction "pos"
 * in the DBM program to be that instruction.
 *
 * Parameters
 * - stmt: Pointer to chidb_stmt to be initialized. Assumes it
 *         points to enough memory to contain a chidb_stmt struct.
 * - op: Pointe
 * - pos:
 *
 * Return
 * - CHIDB_OK: Operation successful
 * - CHIDB_ENOMEM: Could not allocate memory
 */
int chidb_stmt_set_op(chidb_stmt *stmt, chidb_dbm_op_t *op, uint32_t pos)
{
	/* Is the array of instructions large enough for instruction "pos"?
	 * If not, reallocate the instruction array */
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

/* Forward declaration of instruction handler. See dbm-ops.c for details */
int chidb_dbm_op_handle (chidb_stmt *stmt, chidb_dbm_op_t *op);


/* Run the DBM
 *
 * This function will run the DBM until one of the following happens:
 *
 *  - A ResultRow instruction is encountered. startRR and nRR are set
 *    to the appropriate values, and the function returns CHIDB_ROW
 *  - A Halt instruction is encountered, and CHIDB_DONE is returned.
 *  - The end of the program is reached (the program counter goes
 *    beyond endOp), and CHIDB_DONE is returned.
 *  - Any of the instructions returns anything other than CHIDB_OK
 *    or CHIDB_ROW. The program stops executing and and the return
 *    value of the instruction handler is returned.
 *
 * Parameters
 * - stmt: DBM to run.
 *
 * Returns
 * - CHIDB_ROW: Statement returned a row.
 * - CHIDB_DONE: Statement has finished executing.
 * - Any error code returned by an individual instruction handler.
 */
int chidb_stmt_exec(chidb_stmt *stmt)
{
    int rc = CHIDB_OK;

    while(stmt->pc < stmt->endOp)
    {
        chidb_dbm_op_t *op = &stmt->ops[stmt->pc++];
        rc = chidb_dbm_op_handle(stmt, op);

        if (rc != CHIDB_OK)
            break;
    }

    assert(stmt->nRR == stmt->nCols);

    if (rc == CHIDB_OK || rc == CHIDB_DONE)
        rc = CHIDB_DONE;

    return rc;
}

/* Prints a human-readable representation of an instruction */
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

/* Returns a human-readable string representation of a register */
char* chidb_stmt_reg_str(chidb_dbm_register_t *r)
{
    char s[MAX_STR_LEN + 1];

    switch(r->type)
    {
    case REG_UNSPECIFIED:
        strcpy(s, "");
        break;
    case REG_NULL:
        strcpy(s, "NULL");
        break;
    case REG_INT32:
        snprintf(s, MAX_STR_LEN, "%i", r->value.i);
        break;
    case REG_STRING:
        snprintf(s, MAX_STR_LEN, "\"%s\"", r->value.s);
        break;
    case REG_BINARY:
        snprintf(s, MAX_STR_LEN, "(%i bytes)", r->value.bin.nbytes);
        break;
    }

    return strdup(s);
}

/* Prints a human-readable representation of a register */
int chidb_stmt_reg_print(chidb_dbm_register_t *r)
{
    char *s = chidb_stmt_reg_str(r);

    printf("%s", s);

    free(s);

    return CHIDB_OK;
}

/* Returns a string representation of the current Result Row, with each
 * column separated by "sep"  */
char* chidb_stmt_rr_str(chidb_stmt *stmt, char sep)
{
    char s[MAX_STR_LEN + 1], *rstr, sepstr[2];
    bool first = true;

    s[0]='\0';

    sepstr[0] = sep;
    sepstr[1] = '\0';

    for(int i=stmt->startRR; i < stmt->startRR + stmt->nRR; i++)
    {
        chidb_dbm_register_t *r = &stmt->reg[i];

        if(r->type != REG_UNSPECIFIED)
        {
            if(!first)
                strcat(s, sepstr);
            else
                first = false;

            rstr = chidb_stmt_reg_str(r);
            strcat(s, rstr);
            free(rstr);
        }
    }

    return strdup(s);
}


/* Prints the current Result Row, with each column separated by "sep"  */
int chidb_stmt_rr_print(chidb_stmt *stmt, char sep)
{
    char *s = chidb_stmt_rr_str(stmt, sep);

    printf("%s", s);

    free(s);

    return CHIDB_OK;
}

/* Prints a human-readable representation of the entire DBM, including
 * the program and current values of the registers */
int chidb_stmt_print(chidb_stmt *stmt)
{
    printf("     opcode          P1     P2     P3     P4\n");
    printf("     --------------- ------ ------ ------ ------\n");
    for(int i=0; i < stmt->endOp; i++)
    {
        printf("%3i: ", i);
        chidb_stmt_op_print(&stmt->ops[i]);
    }
    printf("     --------------- ------ ------ ------ ------\n\n");

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

/* Reallocates the size of the program (i.e., the number of possible instructions)
 * to be "size" instructions. All new instructions are initialized to be the Noop
 * instruction.  */
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

/* Reallocates the number of registers in the DBM to be
 * to be "size" registers. All new registers are set to type REG_UNSPECIFIED */
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

/* Reallocates the number of cursors in the DBM to be
 * to be "size" cursors. All new cursors are set to type CURSOR_UNSPECIFIED */
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

