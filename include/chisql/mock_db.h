#ifndef __MOCK_DB_H__
#define __MOCK_DB_H__

#include "common.h"
#include "create.h"
#include "list.h"

void mock_db_init(void);
void add_table(Table_t *table);
void remove_table(Table_t *table);
Table_t *table_by_name(const char *name);
void show_tables(void);
/* puts all of table's columns in a List_t */
List_t column_list(Table_t *table);
/* Creates a List_t of all columns shared by table1 and table2 */
List_t columns_in_common(Table_t *table1, Table_t *table2);
/* Same as above methods, but searches through existing tables */
List_t column_list_str(const char *table_name);
List_t columns_in_common_str(const char *table1, const char *table2);

#endif