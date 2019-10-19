#include <string.h>

#include "cconfig/cconfig.h"

#include "util/log.h"
#include "util/mem.h"
#include "util/str.h"

static struct cconfig_entry *cconfig_extend_config(struct cconfig *config)
{
    config->nentries++;

    config->entries = xrealloc(
        config->entries, config->nentries * sizeof(struct cconfig_entry));
    memset(
        &config->entries[config->nentries - 1],
        0,
        sizeof(struct cconfig_entry));

    return &config->entries[config->nentries - 1];
}

struct cconfig *cconfig_init()
{
    struct cconfig *config;

    config = xmalloc(sizeof(struct cconfig));
    memset(config, 0, sizeof(struct cconfig));

    return config;
}

struct cconfig_entry *cconfig_get(struct cconfig *config, const char *key)
{
    log_assert(config);
    log_assert(key);

    for (uint32_t i = 0; i < config->nentries; i++) {
        if (!strcmp(config->entries[i].key, key)) {
            return &config->entries[i];
        }
    }

    return NULL;
}

void cconfig_set(
    struct cconfig *config,
    const char *key,
    const char *value,
    const char *desc)
{
    struct cconfig_entry *entry;

    log_assert(config);
    log_assert(key);
    log_assert(value);
    log_assert(desc);

    entry = cconfig_get(config, key);

    if (!entry) {
        entry = cconfig_extend_config(config);
    } else {
        free(entry->key);
        free(entry->value);
        free(entry->desc);

        memset(entry, 0, sizeof(struct cconfig_entry));
    }

    entry->key = str_dup(key);
    entry->desc = str_dup(desc);
    entry->value = str_dup(value);
}

void cconfig_set2(struct cconfig *config, const char *key, const char *value)
{
    struct cconfig_entry *entry;

    log_assert(config);
    log_assert(key);
    log_assert(value);

    entry = cconfig_get(config, key);

    if (!entry) {
        entry = cconfig_extend_config(config);
    } else {
        free(entry->key);
        free(entry->value);
    }

    entry->key = str_dup(key);
    entry->value = str_dup(value);

    /* Description optional, but do not wipe previous description if
       available */
    if (!entry->desc) {
        entry->desc = "";
    }
}

void cconfig_finit(struct cconfig *config)
{
    for (uint32_t i = 0; i < config->nentries; i++) {
        free(config->entries[i].key);
        free(config->entries[i].value);
        free(config->entries[i].desc);
    }

    free(config->entries);
    free(config);
}