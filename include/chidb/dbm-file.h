/*
 * dbmfile.h
 *
 *  Created on: Mar 29, 2015
 *      Author: borja
 */

#ifndef DBMFILE_H_
#define DBMFILE_H_

#include <chidb/chidb.h>

/* Forward declaration */
typedef struct chidb_dbm_file chidb_dbm_file_t;

int chidb_dbm_file_load(const char* filename, chidb_dbm_file_t **dbmf, chidb *db);
int chidb_dbm_file_load2(const char* filename, chidb_dbm_file_t **dbmf, const char* dbfiledir, const char* genfiledir, bool copyOnUse);
int chidb_dbm_file_run(chidb_dbm_file_t *dbmf);
int chidb_dbm_file_print_rr(chidb_dbm_file_t *dbmf);
int chidb_dbm_file_print_program(chidb_dbm_file_t *dbmf);
int chidb_dbm_file_close(chidb_dbm_file_t *dbmf);


#endif /* DBMFILE_H_ */
