/*****************************************************************************
 *
 *																 chidb
 *
 * This is the header for the chidb API.
 *
 * The chidb API comprises a set of functions that allows client software
 * to access and manipulate chidb files, including executing SQL statements
 * on them. See the chidb Architecture document for more details.
 *
 * 2009, 2010 Borja Sotomayor - http://people.cs.uchicago.edu/~borja/
 * Some modifications by CMSC 23500 class of Spring 2009
\*****************************************************************************/

#ifndef CHIDB_H_
#define CHIDB_H_

#include <chisql/chisql.h>

/* Forward declarations.
 * From the API's perspective's, these are opaque data types. */
typedef struct chidb_stmt chidb_stmt;
typedef struct chidb chidb;

/* API return codes */
#define CHIDB_OK (0)
#define CHIDB_EINVALIDSQL (1)
#define CHIDB_ENOMEM (2)
#define CHIDB_ECANTOPEN (3)
#define CHIDB_ECORRUPT (4)
#define CHIDB_ECONSTRAINT (5)
#define CHIDB_EMISMATCH (6)
#define CHIDB_EIO (7)
#define CHIDB_EMISUSE (8)

#define CHIDB_ROW (100)
#define CHIDB_DONE (101)

/* Opens a chidb file.
 *
 * If the file does not exist, it will be created
 *
 * Parameters
 * - file: Filename of the chidb file to open/create
 * - db: Out parameter. Returns a pointer to a chidb struct. The chidb
 *       struct is an opaque type representing a chidb database. In
 *       other words, an API user should not be concerned with what
 *       is contained in a variable of type chidb, and should simply
 *       use it as a representation of a chidb database to pass along
 *       to other API functions.
 *
 * Return
 * - CHIDB_OK: Operation successful
 * - CHIDB_ENOMEM: Could not allocate memory
 * - CHIDB_ECANTOPEN: Unable to open the database file
 * - CHIDB_ECORRUPT: The database file is not well formed
 * - CHIDB_EIO: An I/O error has occurred when accessing the file
 */
int chidb_open(const char *file, chidb **db); 


/* Prepares a SQL statement for execution
 *
 * Parameters
 * - db: chidb database
 * - sql: SQL statement
 * - stmt: Out parameter. Returns a pointer to a chidb_stmt. The chidb_stmt
 *         type is an opaque type representing a prepared SQL statement.
 *
 * Return
 * - CHIDB_OK: Operation successful
 * - CHIDB_EINVALIDSQL: Invalid SQL
 * - CHIDB_ENOMEM: Could not allocate memory
 */
int chidb_prepare(chidb *db, const char *sql, chidb_stmt **stmt);


/* Steps through a prepared SQL statement
 *
 * This function will run the SQL statement until a result row is available
 * or just runs the SQL statement to completion if it is not meant to
 * produce a result row (such as an INSERT statement)
 *
 * If the statement is a SELECT statement, this function returns
 * CHIDB_ROW each time a result row is produced. The values of the
 * result row can be accessed using the column access functions
 * (chidb_column_*). Thus, chidb_step has to be called repeatedly
 * to access all the rows returned by the query. Once there are no
 * more rows left, or if the statement is not meant to produce any
 * results, then CHIDB_DONE is returned (note that this function does
 * not return CHIDB_OK).
 *
 * Parameters
 * - stmt: Prepared SQL statement
 *
 * Return
 * - CHIDB_ROW: Statement returned a row.
 * - CHIDB_DONE: Statement has finished executing.
 */
int chidb_step(chidb_stmt *stmt);


/* Finalizes a SQL statement, freeing all resources associated with it.
 *
 * Parameters
 * - stmt: Prepared SQL statement
 *
 * Return
 * - CHIDB_OK: Operation successful
 * - CHIDB_EMISUSE: Statement was already finalized
 */
int chidb_finalize(chidb_stmt *stmt);


/* Returns the number of columns returned by a SQL statement
 *
 * Parameters
 * - stmt: Prepared SQL statement
 *
 * Return
 * - Number of columns in the result rows. If the SQL statement is not
 *   meant to produce any results (such as an INSERT statement), then 0
 *   is returned.
 */
int chidb_column_count(chidb_stmt *stmt);


/* Returns the type of a column
 *
 * Parameters
 * - stmt: Prepared SQL statement
 * - col: Column (columns are numbered from 0)
 *
 * Return
 * - Column type (see chidb Architecture document for valid types)
 */
int chidb_column_type(chidb_stmt *stmt, int col);


/* Returns the name of a column
 *
 * Parameters
 * - stmt: Prepared SQL statement
 * - col: Column (columns are numbered from 0)
 *
 * Return
 * - Pointer to a null-terminated string with the name of column. The API
 *   client does not have to free() the returned string. It is the API's
 *   responsibility to allocate and free the memory for this string.
 */
const char *chidb_column_name(chidb_stmt* stmt, int col);


/* Returns the value of a column of integer type
 *
 * Parameters
 * - stmt: Prepared SQL statement
 * - col: Column (columns are numbered from 0)
 *
 * Return
 * - Integer value
 */
int chidb_column_int(chidb_stmt *stmt, int col);


/* Returns the value of a column of string type
 *
 * Parameters
 * - stmt: Prepared SQL statement
 * - col: Column (columns are numbered from 0)
 *
 * Return
 * - Pointer to a null-terminated string with the value. The API client
 *   does not have to free() the returned string. It is the API's
 *   responsibility to allocate and free the memory for this string
 *   (note that this may happen after chidb_step is called again)
 */
const char *chidb_column_text(chidb_stmt *stmt, int col);


/* Closes a chidb database
 *
 * Parameters
 * - db: chidb database
 *
 * Return
 * - CHIDB_OK: Operation successful
 * - CHIDB_EMISUSE: Database that is already closed
 */
int chidb_close(chidb *db); 

#endif /*CHIDB_H_*/
