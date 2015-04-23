/*
 *  chidb - a didactic relational database management system
 *
 *  Type definitions for the Database Machine
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


#ifndef DBM_TYPES_H_
#define DBM_TYPES_H_

#include <stdbool.h>
#include <chidb/chisql.h>
#include "chidbInt.h"
#include "dbm-cursor.h"

#define DEFAULT_OPS_SIZE (50)
#define DEFAULT_REG_SIZE (10)
#define DEFAULT_CUR_SIZE (10)

/* We define a "for each" macro to generate the various portions
 * of code that relate to opcodes. This is based on the solution
 * shown at http://stackoverflow.com/questions/9907160/how-to-convert-enum-names-to-string-in-c
 */

#define FOREACH_OP(OP)  \
        OP(Noop)        \
        OP(OpenRead)    \
        OP(OpenWrite)   \
        OP(Close)       \
        OP(Rewind)      \
        OP(Next)        \
        OP(Prev)        \
        OP(Seek)        \
        OP(SeekGt)      \
        OP(SeekGe)      \
        OP(SeekLt)      \
        OP(SeekLe)      \
        OP(Column)      \
        OP(Key)         \
        OP(Integer)     \
        OP(String)      \
        OP(Null)        \
        OP(ResultRow)   \
        OP(MakeRecord)  \
        OP(Insert)      \
        OP(Eq)          \
        OP(Ne)          \
        OP(Lt)          \
        OP(Le)          \
        OP(Gt)          \
        OP(Ge)          \
        OP(IdxGt)       \
        OP(IdxGe)       \
        OP(IdxLt)       \
        OP(IdxLe)       \
        OP(IdxPKey)     \
        OP(IdxInsert)   \
        OP(CreateTable) \
        OP(CreateIndex) \
        OP(Copy)        \
        OP(SCopy)       \
        OP(Halt)

/* The following generates an enum type for the opcode. It expands to:
 *
 * typedef enum opcode
 * {
 *    Op_OpenRead,
 *    Op_OpenWrite,
 *    Op_Close,
 *    ...
 *    Op_Halt
 * } opcode_t;
 */

#define GENERATE_ENUM(ENUM) Op_ ## ENUM,
typedef enum opcode
{
    FOREACH_OP(GENERATE_ENUM)
} opcode_t;


/* The following generates an array of strings mapping opcodes to
 * opcode names. It expands to:
 *
 * static const char* op_str[] = {
 * {
 *    [Op_OpenRead] = "OpenRead",
 *    [Op_OpenWrite] = "OpenWrite",
 *    ...
 *    [Op_Halt] = "Halt",
 * } opcode_t;
 */

#define GENERATE_ENUM_STR(ENUM) [Op_ ## ENUM] = #ENUM,
static const char* op_str[] =
{
    FOREACH_OP(GENERATE_ENUM_STR)
};

/* Functions to map opcodes to strings, and viceverse */

static inline const char* opcode_to_str(int opcode)
{
    return (opcode >=0 && opcode <= Op_Halt)? op_str[opcode] : NULL;
}

static inline int str_to_opcode(const char *s)
{
    for(int i=0; i<=Op_Halt; i++)
        if (strcmp(s, op_str[i]) == 0)
            return i;

    return -1;
}

/* A single DBM instruction */
typedef struct chidb_dbm_op
{
    opcode_t opcode;
    int32_t p1;
    int32_t p2;
    int32_t p3;
    char *p4;
} chidb_dbm_op_t;


/* A register can be of type integer, string, null or binary.
 * Additionally we define a REG_UNSPECIFIED type, which is
 * the type of any new register than hasn't been assigned a value. */
typedef enum register_type
{
    REG_UNSPECIFIED    = 0,
    REG_NULL           = 1,
    REG_INT32          = 2,
    REG_STRING         = 3,
    REG_BINARY         = 4
} register_type_t;

static inline const char* regtype_to_str(register_type_t regtype)
{
    switch(regtype)
    {
    case REG_UNSPECIFIED:
        return "unspecified";
    case REG_NULL:
        return "null";
    case REG_INT32:
        return "integer";
    case REG_STRING:
        return "string";
    case REG_BINARY:
        return "binary";
    default:
        return "unknown";
    }
}

/* A type representing a single register */
typedef struct chidb_dbm_register
{
    register_type_t type;

    union
    {
        int32_t i;
        char* s;
        struct
        {
            uint8_t* bytes;
            uint32_t nbytes;
        } bin;
    } value;

} chidb_dbm_register_t;

/*  This is the struct that represents a single DBM program.
 *
 *  Notice how a single DBM program has its own registers and cursors;
 *  unlike the type of programs you may be accustomed to, the DBM programs
 *  don't share registers and other resources. Each program is, in essence,
 *  a self-contained machine.
 *
 *  The reason why this type is called stmt (short for “statement”) is
 *  because a SQL statement is compiled into a DBM. So, in essence,
 *  a compiled SQL statement *is* a DBM.
 *
 */
struct chidb_stmt
{
    /* Database associated with this statement */
    chidb *db;

    /* SQL statement from which this DBM program was created */
    chisql_statement_t *sql;

    /* Program counter and instructions */
    /* Instructions are stored in a dynamically allocated array of chidb_dbm_op_t's */
    uint32_t pc;
    chidb_dbm_op_t *ops;
    uint32_t nOps;  /* Size of the array */
    uint32_t endOp; /* Last actual operation (endOp < nOps) */

    /* Registers */
    /* Registers are stored in a dynamically allocated array of chidb_dbm_register_t's */
    chidb_dbm_register_t *reg;
    uint32_t nReg;

    /* Cursors */
    /* Cursors are stored in a dynamically allocated array of chidb_dbm_cursor_t's */
    chidb_dbm_cursor_t *cursors;
    uint32_t nCursors;

    /* Result Row: First register and number of registers */
    uint32_t startRR;
    uint32_t nRR;

    /* Result row: column names */
    /* nCols is determined by the SQL statement. nRR should always
     * match this number of columns. */
    char **cols;
    uint32_t nCols;

    /* Is this an "EXPLAIN" statement? If so, "running" this
     * statement will yield the program itself, with one row
     * per operation */
    bool explain;

    /* Additional fields go here */
};

/* Handy macros for checking whether we're accessing a correct register, cursor, or DBM address */

#define EXISTS_REGISTER(stmt, r) ((r) >= 0 && (r) < (stmt)->nReg)
#define IS_VALID_REGISTER(stmt, r) (EXISTS_REGISTER(stmt, r) && (stmt)->reg[r].type != REG_UNSPECIFIED)

#define EXISTS_CURSOR(stmt, c) ((c) >= 0 && (c) < (stmt)->nCursors)
#define IS_VALID_CURSOR(stmt, c) (EXISTS_CURSOR(stmt, c) && (stmt)->cursors[c].type != CURSOR_UNSPECIFIED)

#define IS_VALID_ADDRESS(stmt, a) ((a) >= 0 && (a) < (stmt)->endOp)


#endif /* DBM_TYPES_H_ */
