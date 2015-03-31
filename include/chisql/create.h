#ifndef __CREATE_H_
#define __CREATE_H_

#include "common.h"
#include "column.h"

typedef struct Table_s {
   char *name;
   Column_t *columns;
} Table_t;

enum key_dec_type {KEY_DEC_PRIMARY, KEY_DEC_FOREIGN};

typedef struct KeyDec_s {
   enum key_dec_type t; 
   union {
      StrList_t *primary_keys;
      ForeignKeyRef_t fkey;
   } dec;
   struct KeyDec_s *next;
} KeyDec_t;

typedef struct TableReference_s {
   char *table_name, *alias;
} TableReference_t;

typedef struct Index_s {
   char *name, *table_name, *column_name;
   int unique;
} Index_t;

enum CreateType { CREATE_TABLE, CREATE_INDEX };

typedef struct Create_s {
   enum CreateType t;
   union {
      Table_t *table;
      Index_t *index;
   };
} Create_t;

Table_t *   Table_make(char *name, Column_t *columns, KeyDec_t *decs);
void        Table_print(Table_t *table);
void        Table_free(void *table); /* void for generic */
Table_t *   Table_addKeyDecs(Table_t *table, KeyDec_t *decs);

KeyDec_t *  KeyDec_append(KeyDec_t *decs, KeyDec_t *dec);
KeyDec_t *  ForeignKeyDec(ForeignKeyRef_t fkr);
KeyDec_t *  PrimaryKeyDec(StrList_t *col_names);

TableReference_t *TableReference_make(char *table_name, char *alias);
void        TableReference_free(TableReference_t *tref);

Index_t *   Index_make(char *name, char *table_name, char *column_name);
Index_t *   Index_makeUnique(Index_t *idx);
void        Index_print(Index_t *idx);
void        Index_free(Index_t *idx);

Create_t *  Create_fromTable(Table_t *table);
Create_t *  Create_fromIndex(Index_t *idx);
void        Create_print(Create_t *cre);
void        Create_free(Create_t *cre);

#endif
