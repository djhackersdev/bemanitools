#include <windows.h>

#include <ctype.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "config/resource.h"

#include "util/array.h"
#include "util/log.h"
#include "util/str.h"
#include "util/winres.h"

struct usage {
    uint16_t id;
    char name[128];
};

struct usage_page {
    uint16_t id;
    struct array usages;
    char wildcard[128];
};

static struct array usages;

static bool usages_begin_page(const char *line, struct usage_page *page);
static bool usages_read_usage(const char *line, struct usage_page *page);

void usages_init(HINSTANCE inst)
{
    struct resource res;
    struct usage_page *cur_page;
    struct usage_page tmp_page;
    char line[512];
    size_t line_no;
    size_t nchars;

    resource_open(&res, inst, "USAGES", IDR_USAGES);
    array_init(&usages);

    cur_page = NULL;
    line_no = 0;

    while (resource_fgets(&res, line, sizeof(line))) {
        line_no++;

        nchars = strlen(line);

        if (nchars == 0 || line[0] == '#'
                || strspn(line, " \t\r\n") == nchars) {
            continue;
        }

        str_trim(line);

        if (!isspace(line[0])) {
            if (!usages_begin_page(line, &tmp_page)) {
                log_fatal("IDR_USAGES:%u: Invalid usage page",
                        (unsigned int) line_no);

                break;
            }

            cur_page = array_append(struct usage_page, &usages);
            memcpy(cur_page, &tmp_page, sizeof(tmp_page));
        } else {
            if (cur_page == NULL) {
                log_fatal("IDR_USAGES:%u: Usage before first page",
                        (unsigned int) line_no);

                break;
            }

            if (!usages_read_usage(line, cur_page)) {
                log_fatal("IDR_USAGES:%u: Invalid usage",
                        (unsigned int) line_no);

                break;
            }
        }
    }
}

static bool usages_begin_page(const char *line, struct usage_page *page)
{
    int id;

    if (sscanf(line, "%i", &id) != 1) {
        return false;
    }

    page->id = (uint16_t) id;
    array_init(&page->usages);

    return true;
}

static bool usages_read_usage(const char *line, struct usage_page *page)
{
    struct usage *usage;
    int id;
    int offset;
    char c;

    if (sscanf(line, " %c %n", &c, &offset) == 1 && c == '*') {
        str_cpy(page->wildcard, sizeof(page->wildcard), &line[offset]);
        page->wildcard[sizeof(page->wildcard) - 1] = '\0';

        return true;
    } else if (sscanf(line, " %i %n", &id, &offset) == 1) {
        usage = array_append(struct usage, &page->usages);

        usage->id = (uint16_t) id;
        str_cpy(usage->name, sizeof(usage->name), &line[offset]);
        usage->name[sizeof(usage->name) - 1] = '\0';

        return true;
    } else {
        return false;
    }
}

void usages_get(char *chars, size_t nchars, uint32_t usage_id)
{
    const struct usage_page *page;
    const struct usage *usage;
    uint16_t hi;
    uint16_t lo;
    size_t i;
    size_t j;

    hi = ((usage_id >> 16) & 0xFFFF);
    lo = ((usage_id >>  0) & 0xFFFF);

    for (i = 0 ; i < usages.nitems ; i++) {
        page = array_item(struct usage_page, &usages, i);

        if (page->id != hi) {
            continue;
        }

        for (j = 0 ; j < page->usages.nitems ; j++) {
            usage = array_item(struct usage, &page->usages, j);

            if (usage->id == lo) {
                str_cpy(chars, nchars, usage->name);
                chars[nchars - 1] = '\0';

                return;
            }
        }

        if (page->wildcard[0]) {
            str_format(chars, nchars, page->wildcard, lo);

            return;
        }
    }

    str_format(chars, nchars, "%#08x", usage_id);
}

void usages_fini(void)
{
    size_t i;

    for (i = 0 ; i < usages.nitems ; i++) {
        array_fini(&array_item(struct usage_page, &usages, i)->usages);
    }

    array_fini(&usages);
}

