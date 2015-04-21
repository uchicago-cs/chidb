#include <string.h>
#include "shell.h"

void chidb_shell_init_ctx(chidb_shell_ctx_t *ctx)
{
    ctx->db = NULL;
    ctx->dbfile = NULL;

    ctx->header = false;
    ctx->mode = MODE_LIST;
}

int chidb_shell_open_db(chidb_shell_ctx_t *ctx, char *file)
{
    int rc;

    rc = chidb_open(file, &ctx->db);

    if (rc != CHIDB_OK)
        return 1;

    ctx->dbfile = strdup(file);

    return 0;
}




