/*
 * chisql.h
 *
 *  Created on: Mar 29, 2015
 *      Author: borja
 */

#ifndef CHISQL_H_
#define CHISQL_H_

/* Forward declaration */
typedef struct chisql_statement chisql_statement_t;


int chisql_parser(const char *sql, chisql_statement_t **stmt);

int chisql_stmt_print(chisql_statement_t *stmt);

#endif /* CHISQL_H_ */
