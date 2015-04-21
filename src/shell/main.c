/*****************************************************************************
 *
 *																 chidb
 *
 * This module provides a simple chidb shell.
 *
 * This shell assumes a complete implementation of the chidb API
 * is available. If so, provides a basic SQL shell.
 *
 * 2009, 2010 Borja Sotomayor - http://people.cs.uchicago.edu/~borja/
\*****************************************************************************/
#include <unistd.h>
#include <stdlib.h>
#include <histedit.h>
#include <string.h>
#include <chidb/chidb.h>
#include <chidb/log.h>
#include "shell.h"
#include "commands.h"


char *prompt(EditLine *e)
{
    return "chidb> ";
}

int main(int argc, char *argv[])
{
    EditLine *el;
    History *hist;
    int opt;
    int rc;
    int verbosity = 0;
    char *command = NULL;
    chidb_shell_ctx_t shell_ctx;

    chidb_shell_init_ctx(&shell_ctx);

    /* Process command-line arguments */
    while ((opt = getopt(argc, argv, "c:vh")) != -1)
        switch (opt)
        {
        case 'c':
            command = strdup(optarg);
            break;
        case 'v':
            verbosity++;
            break;
        case 'h':
            printf("Usage: chidb [-c COMMAND] [DATABASE]\n");
            exit(0);
        default:
            printf("ERROR: Unknown option -%c\n", opt);
            exit(-1);
        }

    /* Set logging level based on verbosity */
    switch(verbosity)
    {
    case 0:
        chilog_setloglevel(CRITICAL);
        break;
    case 1:
        chilog_setloglevel(INFO);
        break;
    case 2:
        chilog_setloglevel(DEBUG);
        break;
    case 3:
        chilog_setloglevel(TRACE);
        break;
    default:
        chilog_setloglevel(TRACE);
        break;
    }

    if (optind < argc)
    {
        rc = chidb_shell_open_db(&shell_ctx, argv[optind]);
        if(rc)
        {
            fprintf(stderr, "ERROR: Could not open file %s or file is not well formed.\n", argv[optind]);
            exit(1);
        }
    }

    /* If a command was specified as an argument, we just run that.
     * Otherwise, we start the shell. */
    if (command)
    {
        chidb_shell_handle_cmd(&shell_ctx, command);
    }
    else
    {
        HistEvent ev;

        /* Initialize EditLine */
        el = el_init(argv[0], stdin, stdout, stderr);
        el_set(el, EL_PROMPT, &prompt);
        el_set(el, EL_EDITOR, "emacs");

        /* Initialize the history */
        hist = history_init();
        if (hist == 0)
        {
            fprintf(stderr, "ERROR: Could not initialize history.\n");
            return 1;
        }
        history(hist, &ev, H_SETSIZE, 100); // 100 elements in history
        el_set(el, EL_HIST, history, hist); // history callback

        while (1)
        {
            int count;
            const char *cmd;
            char *cmd2;

            cmd = el_gets(el, &count);

            if (count == 0)
            {
                putchar('\n');
                break;
            }
            else if (count == 1)
            {
                continue;
            }
            else
            {
                cmd2 = strdup(cmd);

                /* TODO: Do better whitespace stripping */
                if(cmd2[strlen(cmd2)-1] == '\n')
                    cmd2[strlen(cmd2)-1] = '\0';

                history(hist, &ev, H_ENTER, cmd2); // Add to history

                chidb_shell_handle_cmd(&shell_ctx, cmd2);
                free(cmd2);
            }

        }


        history_end(hist);
        el_end(el);
    }

    return 0;
}

