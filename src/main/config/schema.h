#ifndef CONFIG_SCHEMA_H
#define CONFIG_SCHEMA_H

#include <stddef.h>
#include <stdint.h>

struct action_def {
    uint8_t bit;
    unsigned int name_rsrc;
};

struct light_def {
    uint8_t bit;
    unsigned int name_rsrc;
};

struct analog_def {
    uint8_t tag;
    unsigned int label_rsrc;
};

struct eam_unit_def {
    unsigned int label_rsrc;
    uint8_t unit_no;
};

struct schema {
    const char *name;
    unsigned int label;
    const struct action_def *actions;
    size_t nactions;
    const struct light_def *lights;
    size_t nlights;
    const struct analog_def *analogs;
    size_t nanalogs;
    const struct eam_unit_def *units;
    size_t nunits;
};

extern const struct schema schemas[];
extern const size_t nschemas;

#endif
