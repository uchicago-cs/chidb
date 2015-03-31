/*
 * commands.h
 *
 *  Created on: Jul 31, 2014
 *      Author: borja
 */

#ifndef COMMANDS_H_
#define COMMANDS_H_

int chidb_shell_handle_cmd(chidb_shell_ctx_t *ctx, const char *cmd);
int chidb_shell_handle_sql(chidb_shell_ctx_t *ctx, const char *sql);

typedef int (*handler_function)(chidb_shell_ctx_t *ctx, const char *s);

struct handler_entry
{
    char *name;
    int name_len;
    handler_function func;
};

#define HANDLER_ENTRY(NAME) { #NAME, sizeof #NAME - 1, chidb_shell_handle_cmd_ ## NAME}
#define NULL_ENTRY          { NULL, 0, NULL }

int chidb_shell_handle_cmd_parse(chidb_shell_ctx_t *ctx, const char *s);
int chidb_shell_handle_cmd_dbmrun(chidb_shell_ctx_t *ctx, const char *s);


#endif /* COMMANDS_H_ */
