#include "util/cmdline.h"
#include "util/mem.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <windows.h>

static void push_argv(int *argc, char ***argv, const char *begin,
    const char *end)
{
    size_t nchars;
    char *str;

    (*argc)++;
    *argv = xrealloc(*argv, *argc * sizeof(char **));

    nchars = end - begin;
    str = xmalloc(nchars + 1);
    memcpy(str, begin, nchars);
    str[nchars] = '\0';

    (*argv)[*argc - 1] = str;
}

void args_recover(int *argc_out, char ***argv_out)
{
    int argc;
    char **argv;
    const char *begin;
    const char *pos;
    bool quote;

    argc = 0;
    argv = NULL;
    quote = false;

    for (begin = pos = GetCommandLine() ; *pos ; pos++) {
        switch (*pos) {
            case '"':
                if (!quote) {
                    quote = true;
                    begin = pos + 1;
                } else {
                    push_argv(&argc, &argv, begin, pos);

                    quote = false;
                    begin = NULL;
                }

                break;

            case ' ':
                if (!quote && begin != NULL) {
                    push_argv(&argc, &argv, begin, pos);
                    begin = NULL;
                }

                break;

            default:
                if (begin == NULL) {
                    begin = pos;
                }

                break;
        }
    }

    if (begin != NULL && !quote) {
        push_argv(&argc, &argv, begin, pos);
    }

    *argc_out = argc;
    *argv_out = argv;
}

void args_free(int argc, char **argv)
{
    int i;

    for (i = 0 ; i < argc ; i++) {
        free(argv[i]);
    }

    free(argv);
}

char *args_join(int argc, char **argv)
{
    char *pos;
    char *str;
    uint32_t nchars;
    uint32_t part_len;
    int i;

    nchars = 0;

    for (i = 0 ; i < argc ; i++) {
        /* 3 = leading space plus two surrounding quotes.
           The first element has no leading space, but this is counterbalanced
           by the fact that the string needs a NUL terminator. */
        nchars += 3 + strlen(argv[i]);
    }

    str = xmalloc(nchars);
    pos = str;

    for (i = 0 ; i < argc ; i++) {
        if (i != 0) {
            *pos++ = ' ';
        }

        *pos++ = '"';

        part_len = strlen(argv[i]);
        memcpy(pos, argv[i], part_len);
        pos += part_len;

        *pos++ = '"';
    }

    *pos = '\0';

    return str;
}

