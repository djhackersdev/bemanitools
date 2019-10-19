#define LOG_MODULE "log-gftools"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "hook/table.h"

#include "util/log.h"

static int CDECL my_GFReportPuts(
    int level,
    const char *source_file,
    int a3,
    const char *func,
    const char *msg);

static int(CDECL *real_GFReportPuts)(
    int level,
    const char *source_file,
    int a3,
    const char *func,
    const char *msg);

static const struct hook_symbol jbhook1_log_gftools_hook_syms[] = {
    {.name = "GFReportPuts",
     .patch = my_GFReportPuts,
     .link = (void **) &real_GFReportPuts},
};

static int CDECL my_GFReportPuts(
    int level,
    const char *source_file,
    int a3,
    const char *func,
    const char *msg)
{
    log_misc("[%d][%s][%d][%s]: %s", level, source_file, a3, func, msg);

    return real_GFReportPuts(level, source_file, a3, func, msg);
}

void jbhook1_log_gftools_init(void)
{
    hook_table_apply(
        NULL,
        "gftools.dll",
        jbhook1_log_gftools_hook_syms,
        lengthof(jbhook1_log_gftools_hook_syms));

    log_info("Inserted gftools log hooks");
}