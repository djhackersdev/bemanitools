#ifndef CORE_PROPERTY_H
#define CORE_PROPERTY_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "core/log.h"
#include "core/property-node.h"

#define CORE_PROPERTY_RESULT_IS_ERROR(x) (x > CORE_PROPERTY_RESULT_SUCCESS)

struct core_property;
typedef struct core_property core_property_t;

enum core_property_result {
    CORE_PROPERTY_RESULT_SUCCESS = 0,
    CORE_PROPERTY_RESULT_ERROR_INTERNAL = 1,
    CORE_PROPERTY_RESULT_ERROR_ALLOC = 2,
    CORE_PROPERTY_RESULT_NOT_FOUND = 3,
    CORE_PROPERTY_RESULT_ERROR_PERMISSIONS = 4,
    CORE_PROPERTY_RESULT_ERROR_READ = 5,
};
typedef enum core_property_result core_property_result_t;
// TODO make error carry details about internal errors -> uint64_t higher + lower bytes, lower = btapi error, higher = implementation error code

typedef core_property_result_t (*core_property_create_t)(size_t size, core_property_t **result);
typedef core_property_result_t (*core_property_file_load_t)(const char *path, core_property_t **result);
typedef core_property_result_t (*core_property_str_load_t)(const char *str, core_property_t **result);
typedef core_property_result_t (*core_property_size_t)(const core_property_t *property, size_t *size);
typedef core_property_result_t (*core_property_clone_t)(const core_property_t *property, core_property_t **result);
typedef void (*core_property_log_t)(const core_property_t *property, core_log_message_t log_impl);
typedef core_property_result_t (*core_property_root_node_get_t)(const core_property_t *property, core_property_node_t **result);
typedef void (*core_property_free_t)(core_property_t *property);

struct core_property_impl {
    core_property_create_t create;
    core_property_file_load_t file_load;
    core_property_str_load_t str_load;
    core_property_size_t size;
    core_property_clone_t clone;
    core_property_log_t log;
    core_property_root_node_get_t root_node_get;
    core_property_free_t free;
};

typedef struct core_property_impl core_property_impl_t;

void core_property_impl_set(const core_property_impl_t *impl);
const core_property_impl_t *core_property_impl_get(); 

const char *core_property_result_to_str(core_property_result_t result);
void core_property_fatal_on_error(core_property_result_t result);

core_property_result_t core_property_create(size_t size, core_property_t **property);
core_property_result_t core_property_file_load(const char *path, core_property_t **property);
core_property_result_t core_property_str_load(const char *str, core_property_t **property);
core_property_result_t core_property_size(const core_property_t *property, size_t *size);
core_property_result_t core_property_clone(const core_property_t *property, core_property_t **property_cloned);
void core_property_log(const core_property_t *property, core_log_message_t log_impl);
core_property_result_t core_property_root_node_get(const core_property_t *property, core_property_node_t **node);
void core_property_free(core_property_t *property);

#endif