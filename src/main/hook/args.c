#include <windows.h>

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "hook/args.h"

/* This does not handle escaped double quotes inside args correctly yet */

static HRESULT args_push(
        int *argc,
        char ***argv,
        const char *begin,
        const char *end)
{
    int tmp_argc;
    char **tmp_argv;
    size_t nchars;
    char *str;

    nchars = end - begin;
    str = malloc(nchars + 1);

    if (str == NULL) {
        goto fail;
    }

    memcpy(str, begin, nchars);
    str[nchars] = '\0';

    tmp_argc = *argc + 1;
    tmp_argv = realloc(*argv, tmp_argc * sizeof(char **));

    if (tmp_argv == NULL) {
        goto fail;
    }

    tmp_argv[tmp_argc - 1] = str;

    *argv = tmp_argv;
    *argc = tmp_argc;

    return S_OK;

fail:
    free(str);

    return E_OUTOFMEMORY;
}

HRESULT args_recover(int *argc_out, char ***argv_out)
{
    int argc;
    char **argv;
    const char *begin;
    const char *pos;
    bool quote;
    HRESULT hr;

    assert(argc_out != NULL);
    assert(argv_out != NULL);

    *argc_out = 0;
    *argv_out = NULL;

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
                    hr = args_push(&argc, &argv, begin, pos);

                    if (FAILED(hr)) {
                        goto fail;
                    }

                    quote = false;
                    begin = NULL;
                }

                break;

            case ' ':
                if (!quote && begin != NULL) {
                    args_push(&argc, &argv, begin, pos);
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
        hr = args_push(&argc, &argv, begin, pos);

        if (FAILED(hr)) {
            goto fail;
        }
    }

    *argc_out = argc;
    *argv_out = argv;

    return S_OK;

fail:
    args_free(argc, argv);

    return hr;
}

void args_free(int argc, char **argv)
{
    int i;

    for (i = 0 ; i < argc ; i++) {
        free(argv[i]);
    }

    free(argv);
}
