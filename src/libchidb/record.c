/*
 *  chidb - a didactic relational database management system
 *
 * Since chidb database records can be hard to use in their
 * raw format, this module provides functions that make it easier
 * to create and manipulate database records. This is done with a
 * DBRecord type that creates an abstraction over the raw database format.
 * For example, this function provides an "unpack" function that
 * takes a raw database record and produces a DBRecord variable, which
 * can then be accessed with more convenient get() functions.
 *
 * Additionally, the DBRecord type that can be used to create an
 * 'empty' database record (the number of fields must be known in advance,
 * though) and then append values to that record. Once the record is created,
 * a "pack" function can be used to generate the raw database record that can
 * be stored in a chidb file.
 *
 * The following is an example of how a database record could be created:
 *
 *   DBRecord *dbr;
 *   DBRecordBuffer dbrb;
 *   // DBRecordBuffer is an opaque type used to build a record one field at
 *   // a time; it is discarded once you're done creating the record.
 *   chidb_DBRecord_create_empty(&dbrb, 5); // 5 == number of fields
 *   chidb_DBRecord_appendInt8(&dbrb, 42);
 *   chidb_DBRecord_appendInt16(&dbrb, 42);
 *   chidb_DBRecord_appendInt32(&dbrb, 42);
 *   chidb_DBRecord_appendString(&dbrb, "foo!");
 *   chidb_DBRecord_appendNull(&dbrb);
 *   chidb_DBRecord_finalize(&dbrb, &dbr);
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
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

#include "chidbInt.h"

#include "record.h"
#include "util.h"


/* Create an empty record
 *
 * Note that this function uses a DBRecordBuffer. The actual DBRecord
 * isn't returned until the "finalize" function is called.
 *
 * Parameters
 * - dbrb: Pointer to an uninitialized DBRecordBuffer. After calling this
 *         function, the DBRecordBuffer will be initialized and the append*
 *         functions can be used to append values to the record.
 * - nfields: Number of fields in the record
 *
 * Return
 * - CHIDB_OK: Operation successful
 * - CHIDB_ENOMEM: Could not allocate memory
 */
int chidb_DBRecord_create_empty(DBRecordBuffer *dbrb, uint8_t nfields)
{
    dbrb->dbr = malloc(sizeof(DBRecord));
    dbrb->buf_size = 1024;
    dbrb->field = 0;
    dbrb->offset = 0;
    dbrb->header_size = 1;

    dbrb->dbr->nfields = nfields;
    dbrb->dbr->types = malloc(dbrb->dbr->nfields * sizeof(uint32_t));
    dbrb->dbr->offsets = malloc(dbrb->dbr->nfields * sizeof(uint32_t));
    dbrb->dbr->data = malloc(dbrb->buf_size);

    return CHIDB_OK;
}


/* Append a 1-byte integer to an initialized DBRecordBuffer
 *
 * Parameters
 * - dbrb: Initialized DBRecordBuffer
 * - v: Value to append
 *
 * Return
 * - CHIDB_OK: Operation successful
 * - CHIDB_ENOMEM: Could not allocate memory
 */
int chidb_DBRecord_appendInt8(DBRecordBuffer *dbrb, int8_t v)
{
    dbrb->dbr->offsets[dbrb->field] = dbrb->offset;

    dbrb->dbr->types[dbrb->field] = SQL_INTEGER_1BYTE;
    if (dbrb->offset + 1 > dbrb->buf_size)
    {
        dbrb->buf_size += 1024;
        dbrb->dbr->data = realloc(dbrb->dbr->data, dbrb->buf_size);
    }
    dbrb->dbr->data[dbrb->offset] = v;
    dbrb->offset += 1;
    dbrb->header_size++;
    dbrb->field++;

    return CHIDB_OK;
}


/* Append a 2-byte integer to an initialized DBRecordBuffer
 *
 * Parameters
 * - dbrb: Initialized DBRecordBuffer
 * - v: Value to append
 *
 * Return
 * - CHIDB_OK: Operation successful
 * - CHIDB_ENOMEM: Could not allocate memory
 */
int chidb_DBRecord_appendInt16(DBRecordBuffer *dbrb, int16_t v)
{
    dbrb->dbr->offsets[dbrb->field] = dbrb->offset;

    dbrb->dbr->types[dbrb->field] = SQL_INTEGER_2BYTE;
    if (dbrb->offset + 2 > dbrb->buf_size)
    {
        dbrb->buf_size += 1024;
        dbrb->dbr->data = realloc(dbrb->dbr->data, dbrb->buf_size);
    }
    put2byte(&dbrb->dbr->data[dbrb->offset], v);
    dbrb->offset += 2;
    dbrb->header_size++;
    dbrb->field++;

    return CHIDB_OK;
}


/* Append a 4-byte integer to an initialized DBRecordBuffer
 *
 * Parameters
 * - dbrb: Initialized DBRecordBuffer
 * - v: Value to append
 *
 * Return
 * - CHIDB_OK: Operation successful
 * - CHIDB_ENOMEM: Could not allocate memory
 */
int chidb_DBRecord_appendInt32(DBRecordBuffer *dbrb, int32_t v)
{
    dbrb->dbr->offsets[dbrb->field] = dbrb->offset;

    dbrb->dbr->types[dbrb->field] = SQL_INTEGER_4BYTE;
    if (dbrb->offset + 4 > dbrb->buf_size)
    {
        dbrb->buf_size += 1024;
        dbrb->dbr->data = realloc(dbrb->dbr->data, dbrb->buf_size);
    }
    put4byte(&dbrb->dbr->data[dbrb->offset], v);
    dbrb->offset += 4;
    dbrb->header_size++;
    dbrb->field++;

    return CHIDB_OK;
}

/* Append a NULL value to an initialized DBRecordBuffer
 *
 * Parameters
 * - dbrb: Initialized DBRecordBuffer
 *
 * Return
 * - CHIDB_OK: Operation successful
 */
int chidb_DBRecord_appendNull(DBRecordBuffer *dbrb)
{
    dbrb->dbr->offsets[dbrb->field] = 0;

    dbrb->dbr->types[dbrb->field] = SQL_NULL;
    dbrb->field++;
    dbrb->header_size++;

    return CHIDB_OK;
}

/* Append a string to an initialized DBRecordBuffer
 *
 * Parameters
 * - dbrb: Initialized DBRecordBuffer
 * - v: Value to append
 *
 * Return
 * - CHIDB_OK: Operation successful
 * - CHIDB_ENOMEM: Could not allocate memory
 */
int chidb_DBRecord_appendString(DBRecordBuffer *dbrb,  char *v)
{
    int len;

    dbrb->dbr->offsets[dbrb->field] = dbrb->offset;

    len = strlen(v);
    if (dbrb->offset + len > dbrb->buf_size)
    {
        dbrb->buf_size += 1024;
        dbrb->dbr->data = realloc(dbrb->dbr->data, dbrb->buf_size);
    }
    memcpy(&dbrb->dbr->data[dbrb->offset], v, len);
    dbrb->offset += len;
    dbrb->dbr->types[dbrb->field] = len * 2 + SQL_TEXT;
    dbrb->field++;
    dbrb->header_size += 4;

    return CHIDB_OK;
}


/* Finalizes an initialized DBRecordBuffer
 *
 * This function must be called on an initialized DBRecordBuffer once
 * all the values in that record have been appended to it.
 *
 * Parameters
 * - dbrb: Initialized DBRecordBuffer
 * - dbr: Out parameter used to return a pointer to a DBRecord.
 *
 * Return
 * - CHIDB_OK: Operation successful
 * - CHIDB_ENOMEM: Could not allocate memory
 */
int chidb_DBRecord_finalize(DBRecordBuffer *dbrb, DBRecord **dbr)
{
    dbrb->dbr->nfields = dbrb->field;
    dbrb->dbr->data = realloc(dbrb->dbr->data, dbrb->offset);
    dbrb->dbr->data_len = dbrb->offset;
    dbrb->dbr->packed_len = dbrb->header_size + dbrb->offset;

    *dbr = dbrb->dbr;

    return CHIDB_OK;
}


/* Create a DBRecord from a raw binary database record
 *
 * Parameters
 * - dbr: Out paremeter used to return a pointer to a DBRecord.
 * - raw: Pointer to first byte of raw binary database record
 *
 * Return
 * - CHIDB_OK: Operation successful
 * - CHIDB_ENOMEM: Could not allocate memory
 */
int chidb_DBRecord_unpack(DBRecord **dbr, uint8_t *raw)
{
    *dbr = malloc(sizeof(DBRecord));
    if (dbr == NULL)
        return CHIDB_ENOMEM;

    (*dbr)->nfields = 0;

    uint8_t header_size = raw[0];
    uint8_t header_pos = 1;
    (*dbr)->types = malloc(0xFF * sizeof(uint32_t));
    while(header_pos < header_size)
    {
        if (raw[header_pos] & 0x80)
        {
            getVarint32(&raw[header_pos], &(*dbr)->types[(*dbr)->nfields]);
            header_pos += 4;
        }
        else
        {
            (*dbr)->types[(*dbr)->nfields] = raw[header_pos];
            header_pos += 1;
        }

        (*dbr)->nfields++;
    }
    (*dbr)->types = realloc((*dbr)->types, (*dbr)->nfields * sizeof(uint32_t));

    uint32_t offset = 0;
    (*dbr)->offsets = malloc((*dbr)->nfields * sizeof(uint32_t));
    if ((*dbr)->offsets == NULL)
        return CHIDB_ENOMEM;
    for(int i=0; i<(*dbr)->nfields; i++)
    {
        (*dbr)->offsets[i] = offset;
        int type = chidb_DBRecord_getType(*dbr, i);
        if (type == SQL_NULL)
            offset += 0;
        else if (type == SQL_INTEGER_1BYTE)
            offset += 1;
        else if (type == SQL_INTEGER_2BYTE)
            offset += 2;
        else if (type == SQL_INTEGER_4BYTE)
            offset += 4;
        else if (type == SQL_TEXT)
        {
            int len;
            chidb_DBRecord_getStringLength(*dbr, i, &len);
            offset += len;
        }
    }

    (*dbr)->data_len = offset;
    (*dbr)->packed_len = header_size + offset;
    (*dbr)->data = malloc(offset);
    if ((*dbr)->data == NULL)
        return CHIDB_ENOMEM;
    memcpy((*dbr)->data, raw + header_size, (*dbr)->data_len);

    return CHIDB_OK;
}


/* Create a raw binary database record from a DBRecord
 *
 * Parameters
 * - dbr: The DBRecord
 * - p: Out paremeter used to return a pointer to a raw binary representation
 *      of the record
 *
 * Return
 * - CHIDB_OK: Operation successful
 * - CHIDB_ENOMEM: Could not allocate memory
 */
int chidb_DBRecord_pack(DBRecord *dbr, uint8_t **p)
{
    *p = malloc(dbr->packed_len);
    if (p == NULL)
        return CHIDB_ENOMEM;
    (*p)[0] = dbr->packed_len - dbr->data_len;

    uint8_t header_pos = 1;
    for(int i=0; i < dbr->nfields; i++)
    {
        if (chidb_DBRecord_getType(dbr, i) == SQL_TEXT)
        {
            putVarint32(*p + header_pos, dbr->types[i]);
            header_pos +=4;
        }
        else
        {
            (*p)[header_pos] = dbr->types[i];
            header_pos +=1;
        }
    }
    memcpy(*p + (*p)[0], dbr->data, dbr->data_len);

    return CHIDB_OK;
}


/* Returns the type of a field
 *
 * Parameters
 * - dbr: The DBRecord
 * - field: Index of the field
 *
 * Return
 * - SQL_NULL, SQL_INTEGER_1BYTE, SQL_INTEGER_2BYTE, SQL_INTEGER_4BYTE,
 *   or SQL_TEXT depending on the field type.
 * - SQL_NOTVALID if the specified field has an invalid field type.
 */
int chidb_DBRecord_getType(DBRecord *dbr, uint8_t field)
{
    if(dbr->types[field] == SQL_NULL || dbr->types[field] == SQL_INTEGER_1BYTE ||
            dbr->types[field] == SQL_INTEGER_2BYTE || dbr->types[field] == SQL_INTEGER_4BYTE)
        return dbr->types[field];
    else if ((dbr->types[field] - SQL_TEXT) % 2 == 0)
        return SQL_TEXT;
    else
        return SQL_NOTVALID;
}


/* Returns the value of a 1-byte integer field
 *
 * Parameters
 * - dbr: The DBRecord
 * - field: Index of the field
 * - v: Out parameter used to return the value
 *
 * Return
 * - CHIDB_OK: Operation successful
 */
int chidb_DBRecord_getInt8(DBRecord *dbr, uint8_t field, int8_t *v)
{
    *v = dbr->data[dbr->offsets[field]];

    return CHIDB_OK;
}


/* Returns the value of a 2-byte integer field
 *
 * Parameters
 * - dbr: The DBRecord
 * - field: Index of the field
 * - v: Out parameter used to return the value
 *
 * Return
 * - CHIDB_OK: Operation successful
 */
int chidb_DBRecord_getInt16(DBRecord *dbr, uint8_t field, int16_t *v)
{
    *v = get2byte(&dbr->data[dbr->offsets[field]]);

    return CHIDB_OK;
}


/* Returns the value of a 4-byte integer field
 *
 * Parameters
 * - dbr: The DBRecord
 * - field: Index of the field
 * - v: Out parameter used to return the value
 *
 * Return
 * - CHIDB_OK: Operation successful
 */
int chidb_DBRecord_getInt32(DBRecord *dbr, uint8_t field, int32_t *v)
{
    *v = get4byte(&dbr->data[dbr->offsets[field]]);

    return CHIDB_OK;
}


/* Returns the value of a string field
 *
 * Parameters
 * - dbr: The DBRecord
 * - field: Index of the field
 * - v: Out parameter used to return the value
 *
 * Return
 * - CHIDB_OK: Operation successful
 */
int chidb_DBRecord_getString(DBRecord *dbr, uint8_t field, char **v)
{
    int len ;
    chidb_DBRecord_getStringLength(dbr, field, &len);
    *v = malloc(len+1);
    if (v == NULL)
        return CHIDB_ENOMEM;
    memcpy(*v, &dbr->data[dbr->offsets[field]], len);
    (*v)[len] = '\0';

    return CHIDB_OK;
}


/* Returns the length of a string field
 *
 * Parameters
 * - dbr: The DBRecord
 * - field: Index of the field
 * - len: Out parameter used to return the length
 *
 * Return
 * - CHIDB_OK: Operation successful
 */
int chidb_DBRecord_getStringLength(DBRecord *dbr, uint8_t field, int *len)
{
    *len = (dbr->types[field] - 13) / 2;

    return CHIDB_OK;
}


/* Prints a string representation of a database record to stdout
 *
 * Parameters
 * - dbr: The DBRecord
 *
 * Return
 * - CHIDB_OK: Operation successful
 */
int chidb_DBRecord_print(DBRecord *dbr)
{
    for(int i=0; i<dbr->nfields; i++)
    {
        int type = chidb_DBRecord_getType(dbr, i);
        if (type == SQL_NULL)
            printf("|");
        else if (type == SQL_INTEGER_1BYTE)
        {
            uint8_t i8;
            chidb_DBRecord_getInt8(dbr, i, (int8_t *) &i8);
            printf("| %i ", i8);
        }
        else if (type == SQL_INTEGER_2BYTE)
        {
            uint16_t i16;
            chidb_DBRecord_getInt16(dbr, i, (int16_t *) &i16);
            printf("| %i ", i16);
        }
        else if (type == SQL_INTEGER_4BYTE)
        {
            uint32_t i32;
            chidb_DBRecord_getInt32(dbr, i, (int32_t *) &i32);
            printf("| %i ", i32);
        }
        else if (type == SQL_TEXT)
        {
            char *s;
            chidb_DBRecord_getString(dbr, i, &s);
            printf("| %s ", s);
            free(s);
        }
    }
    printf("|");

    return CHIDB_OK;
}


/* Creates a DBRecord based on a specification string and all the values
 * in the record.
 *
 * This function is useful for testing, and when all the values are
 * known beforehand.
 *
 * The specification string has the following format is a pipe-delimited
 * list of fields. The following field specifiers are recognized:
 *
 * - s: A string
 * - 0: A null value
 * - i1: A 1-byte integer
 * - i2: A 2-byte integer
 * . i4: A 4-byte integer
 *
 * For example, "|s|0|i1|i2|i4|".
 *
 * Parameters
 * - dbr: Out parameter to return the DBRecord
 * - spec: The specification string.
 * - As many parameters as fields in the record.
 *
 * Return
 * - CHIDB_OK: Operation successful
 */
int chidb_DBRecord_create(DBRecord **dbr, const char *spec, ...)
{
    va_list args;
    va_start(args, spec);
    uint8_t nfields;
    DBRecordBuffer dbrb;
    char *aux = (char*) spec-1;

    *dbr = malloc(sizeof(DBRecord));

    nfields = 0;
    while ( (aux = strchr(aux+1, '|')) ) nfields++;
    nfields--;

    chidb_DBRecord_create_empty(&dbrb, nfields);

    aux = (char*) spec;
    while (*aux)
    {
        char intsize;
        char *s;
        uint8_t i8;
        uint16_t i16;
        uint32_t i32;

        switch(*aux++)
        {
        case 'i':
            intsize = *aux++;
            switch(intsize)
            {
            case '1':
                i8 = va_arg(args, int);
                chidb_DBRecord_appendInt8(&dbrb, i8);
                break;
            case '2':
                i16 = va_arg(args, int);
                chidb_DBRecord_appendInt16(&dbrb, i16);
                break;
            case '4':
                i32 = va_arg(args, int);
                chidb_DBRecord_appendInt32(&dbrb, i32);
                break;
            }

            break;

        case '0':
            chidb_DBRecord_appendNull(&dbrb);
            break;
        case 's':
            s = va_arg(args, char *);
            chidb_DBRecord_appendString(&dbrb, s);
            break;
        }
    }
    va_end(args);

    chidb_DBRecord_finalize(&dbrb, dbr);

    return CHIDB_OK;
}


/* Destroys a DBRecord and frees resources associated with it.
 *
 * Parameters
 * - dbr: The DBRecord
 *
 * Return
 * - CHIDB_OK: Operation successful
 */
int chidb_DBRecord_destroy(DBRecord *dbr)
{
    free(dbr->data);
    free(dbr->types);
    free(dbr->offsets);
    free(dbr);

    return CHIDB_OK;
}
