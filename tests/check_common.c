#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <check.h>
#include "check_common.h"

char *database_file_path(const char *fname)
{
    char *f;

    f = malloc(strlen(DATABASES_DIR) + strlen(fname) + 1);
    sprintf(f, "%s%s", DATABASES_DIR, fname);

    return f;
}


char *generated_file_path(const char *fname)
{
    char *f;

    f = malloc(strlen(GENERATED_DIR) + strlen(fname) + 1);
    sprintf(f, "%s%s", GENERATED_DIR, fname);

    return f;
}

char* create_copy(const char *src, const char *dst)
{
    char *srcfile = database_file_path(src);
    char *dstfile = generated_file_path(dst);

    remove(dstfile);
    if(copy(srcfile, dstfile) == NULL)
        ck_abort_msg("Could not create copy of file.");

    free(srcfile);

    return dstfile;
}

void delete_copy(char *f)
{
    remove(f);
    free(f);
}

char* create_tmp_file()
{
    char *template = strdup(TMPFILE_TEMPLATE);

    int tmpfile = mkstemp(template);
    close(tmpfile);

    return template;
}

void delete_tmp_file(char *f)
{
    remove(f);
    free(f);
}
