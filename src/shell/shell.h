#ifndef SHELL_H_
#define SHELL_H_

#include <chidb/chidb.h>

typedef struct chidb_shell_ctx
{
    char *dbfile;
    chidb *db ;

} chidb_shell_ctx_t;

void chidb_shell_init_ctx(chidb_shell_ctx_t *ctx);
int chidb_shell_open_db(chidb_shell_ctx_t *ctx, char *file);

#endif
