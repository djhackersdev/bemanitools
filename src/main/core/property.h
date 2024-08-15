#ifndef CORE_PROPERTY_H
#define CORE_PROPERTY_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "api/core/log.h"

// Macro to allow inlining of the caller function and line numbers
// to make debugging easier
#define core_property_fatal_on_error(result)        \
    if (result != CORE_PROPERTY_RESULT_SUCCESS) {   \
        log_fatal(                                  \
            "Operation on property failed: %s",     \
            core_property_result_to_str(result));   \
    }                                               \

typedef struct core_property {
    // Have size known, but not contents, to allow for stack allocations
    void *v1;
} core_property_t;

typedef struct core_property_node {
    // Have size known, but not contents, to allow for stack allocations
    void *v1;
    void *v2;
} core_property_node_t;

typedef enum core_property_result {
    CORE_PROPERTY_RESULT_SUCCESS = 0,
    CORE_PROPERTY_RESULT_ERROR_INTERNAL = 1,
    CORE_PROPERTY_RESULT_ERROR_ALLOC = 2,
    CORE_PROPERTY_RESULT_NOT_FOUND = 3,
    CORE_PROPERTY_RESULT_ERROR_PERMISSIONS = 4,
    CORE_PROPERTY_RESULT_ERROR_READ = 5,
} core_property_result_t;

typedef core_property_result_t (*core_property_create_t)(
    size_t size, core_property_t **result);
typedef core_property_result_t (*core_property_file_load_t)(
    const char *path, core_property_t **result);
typedef core_property_result_t (*core_property_str_load_t)(
    const char *str, core_property_t **result);
typedef core_property_result_t (*core_property_size_t)(
    const core_property_t *property, size_t *size);
typedef core_property_result_t (*core_property_clone_t)(
    const core_property_t *property, core_property_t **result);
typedef core_property_result_t (*core_property_root_node_get_t)(
    const core_property_t *property, core_property_node_t *result);
typedef core_property_result_t (*core_property_other_node_insert_t)(
    core_property_t *property, const core_property_node_t *node);
typedef void (*core_property_free_t)(core_property_t **property);

typedef struct core_property_api {
    uint16_t version;

    struct {
        core_property_create_t create;
        core_property_file_load_t file_load;
        core_property_str_load_t str_load;
        core_property_size_t size;
        core_property_clone_t clone;
        core_property_root_node_get_t root_node_get;
        core_property_other_node_insert_t other_node_insert;
        core_property_free_t free;
    } v1;
} core_property_api_t;

void core_property_trace_log_enable(bool enable);
void core_property_api_set(const core_property_api_t *api);
void core_property_api_get(core_property_api_t *api);

const char *core_property_result_to_str(core_property_result_t result);

core_property_result_t
core_property_create(size_t size, core_property_t **property);
core_property_result_t
core_property_file_load(const char *path, core_property_t **property);
core_property_result_t
core_property_str_load(const char *str, core_property_t **property);
core_property_result_t
core_property_size(const core_property_t *property, size_t *size);
core_property_result_t core_property_clone(
    const core_property_t *property, core_property_t **property_cloned);
core_property_result_t core_property_root_node_get(
    const core_property_t *property, core_property_node_t *node);
core_property_result_t core_property_other_node_insert(
    core_property_t *property, const core_property_node_t *node);
void core_property_free(core_property_t **property);

#endif