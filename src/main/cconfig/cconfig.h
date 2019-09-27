#ifndef CCONFIG_H
#define CCONFIG_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

struct cconfig_entry {
    char* key;
    char* value;
    char* desc;
};

struct cconfig {
    uint32_t nentries;
    struct cconfig_entry* entries;
};

struct cconfig* cconfig_init();

struct cconfig_entry* cconfig_get(struct cconfig* config, 
        const char* key);

void cconfig_set(struct cconfig* config, const char* key, const char* value,
        const char* desc);

void cconfig_set2(struct cconfig* config, const char* key, const char* value);

void cconfig_finit(struct cconfig* config);

#endif