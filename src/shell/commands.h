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

struct handler_entry;

typedef int (*handler_function)(chidb_shell_ctx_t *ctx, struct handler_entry *e, const char **tokens, int ntokens);

struct handler_entry
{
    char *name;
    char *help;
    int name_len;
    handler_function func;
};

#define HANDLER_ENTRY(NAME,HELP) { #NAME, HELP, sizeof #NAME - 1, chidb_shell_handle_cmd_ ## NAME}
#define NULL_ENTRY               { NULL, NULL, 0, NULL }

int chidb_shell_handle_cmd_open(chidb_shell_ctx_t *ctx, struct handler_entry *e, const char **tokens, int ntokens);
int chidb_shell_handle_cmd_help(chidb_shell_ctx_t *ctx, struct handler_entry *e, const char **tokens, int ntokens);
int chidb_shell_handle_cmd_parse(chidb_shell_ctx_t *ctx, struct handler_entry *e, const char **tokens, int ntokens);
int chidb_shell_handle_cmd_opt(chidb_shell_ctx_t *ctx, struct handler_entry *e, const char **tokens, int ntokens);
int chidb_shell_handle_cmd_dbmrun(chidb_shell_ctx_t *ctx, struct handler_entry *e, const char **tokens, int ntokens);
int chidb_shell_handle_cmd_mode(chidb_shell_ctx_t *ctx, struct handler_entry *e, const char **tokens, int ntokens);
int chidb_shell_handle_cmd_headers(chidb_shell_ctx_t *ctx, struct handler_entry *e, const char **tokens, int ntokens);
int chidb_shell_handle_cmd_explain(chidb_shell_ctx_t *ctx, struct handler_entry *e, const char **tokens, int ntokens);

#endif /* COMMANDS_H_ */
