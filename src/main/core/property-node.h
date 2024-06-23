#ifndef CORE_PROPERTY_NODE_H
#define CORE_PROPERTY_NODE_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "api/core/log.h"

#include "main/core/property.h"

#define CORE_PROPERTY_NODE_RESULT_IS_ERROR(x) \
    (x != CORE_PROPERTY_NODE_RESULT_SUCCESS)
// Based on actual AVS impl max size
#define CORE_PROPERTY_NODE_NAME_SIZE_MAX 256
// Guestimate, should be enough, I hope?
#define CORE_PROPERTY_NODE_ATTR_NAME_SIZE_MAX 128
// Guestimate, should be long enough, I hope?
#define CORE_PROPERTY_NODE_PATH_LEN_MAX 4096

// Macro to allow inlining of the caller function and line numbers
// to make debugging easier
#define core_property_node_fatal_on_error(result)       \
    if (result != CORE_PROPERTY_NODE_RESULT_SUCCESS) {  \
        log_fatal(                                      \
            "Operation on property-node failed: %s",    \
            core_property_node_result_to_str(result));  \
    }                                                   \

typedef struct core_property_node core_property_node_t;

typedef enum core_property_node_result {
    CORE_PROPERTY_NODE_RESULT_SUCCESS = 0,
    CORE_PROPERTY_NODE_RESULT_ERROR_INTERNAL = 1,
    CORE_PROPERTY_NODE_RESULT_NODE_NOT_FOUND = 2,
    CORE_PROPERTY_NODE_RESULT_INVALID_NODE_TYPE = 3,
    CORE_PROPERTY_NODE_RESULT_INVALID_NODE_STRUCTURE = 4,
    CORE_PROPERTY_NODE_RESULT_INVALID_NODE_DATA = 5,
} core_property_node_result_t;

typedef core_property_node_result_t (*core_property_node_name_get_t)(
    const core_property_node_t *node, char *name, size_t len);
typedef core_property_node_result_t (*core_property_node_size_t)(
    const core_property_node_t *node, size_t *size);
typedef core_property_node_result_t (*core_property_node_search_t)(
    const core_property_node_t *node,
    const char *path,
    core_property_node_t *node_out);
typedef core_property_node_result_t (*core_property_node_next_result_search_t)(
    const core_property_node_t *node, core_property_node_t *node_out);
typedef core_property_node_result_t (*core_property_node_child_get_t)(
    const core_property_node_t *node, core_property_node_t *node_out);
typedef core_property_node_result_t (*core_property_node_next_sibling_get_t)(
    const core_property_node_t *node, core_property_node_t *node_out);
typedef core_property_node_result_t (*core_property_node_void_create_t)(
    const core_property_node_t *parent_node,
    const char *key,
    core_property_node_t *node_out);
typedef core_property_node_result_t (*core_property_node_s8_create_t)(
    const core_property_node_t *parent_node,
    const char *key,
    int8_t value,
    core_property_node_t *node_out);
typedef core_property_node_result_t (*core_property_node_u8_create_t)(
    const core_property_node_t *parent_node,
    const char *key,
    uint8_t value,
    core_property_node_t *node_out);
typedef core_property_node_result_t (*core_property_node_s16_create_t)(
    const core_property_node_t *parent_node,
    const char *key,
    int16_t value,
    core_property_node_t *node_out);
typedef core_property_node_result_t (*core_property_node_u16_create_t)(
    const core_property_node_t *parent_node,
    const char *key,
    uint16_t value,
    core_property_node_t *node_out);
typedef core_property_node_result_t (*core_property_node_s32_create_t)(
    const core_property_node_t *parent_node,
    const char *key,
    int32_t value,
    core_property_node_t *node_out);
typedef core_property_node_result_t (*core_property_node_u32_create_t)(
    const core_property_node_t *parent_node,
    const char *key,
    uint32_t value,
    core_property_node_t *node_out);
typedef core_property_node_result_t (*core_property_node_s64_create_t)(
    const core_property_node_t *parent_node,
    const char *key,
    int64_t value,
    core_property_node_t *node_out);
typedef core_property_node_result_t (*core_property_node_u64_create_t)(
    const core_property_node_t *parent_node,
    const char *key,
    uint64_t value,
    core_property_node_t *node_out);
typedef core_property_node_result_t (*core_property_node_bin_create_t)(
    const core_property_node_t *parent_node,
    const char *key,
    void *data,
    size_t len,
    core_property_node_t *node_out);
typedef core_property_node_result_t (*core_property_node_str_create_t)(
    const core_property_node_t *parent_node,
    const char *key,
    const char *value,
    core_property_node_t *node_out);
typedef core_property_node_result_t (*core_property_node_ipv4_create_t)(
    const core_property_node_t *parent_node,
    const char *key,
    uint32_t value,
    core_property_node_t *node_out);
typedef core_property_node_result_t (*core_property_node_float_create_t)(
    const core_property_node_t *parent_node,
    const char *key,
    float value,
    core_property_node_t *node_out);
typedef core_property_node_result_t (*core_property_node_double_create_t)(
    const core_property_node_t *parent_node,
    const char *key,
    double value,
    core_property_node_t *node_out);
typedef core_property_node_result_t (*core_property_node_attr_create_t)(
    const core_property_node_t *parent_node,
    const char *key,
    const char *value);
typedef core_property_node_result_t (*core_property_node_bool_create_t)(
    const core_property_node_t *parent_node,
    const char *key,
    bool value,
    core_property_node_t *node_out);
typedef core_property_node_result_t (*core_property_node_s8_read_t)(
    const core_property_node_t *parent_node, int8_t *value);
typedef core_property_node_result_t (*core_property_node_u8_read_t)(
    const core_property_node_t *parent_node, uint8_t *value);
typedef core_property_node_result_t (*core_property_node_s16_read_t)(
    const core_property_node_t *parent_node, int16_t *value);
typedef core_property_node_result_t (*core_property_node_u16_read_t)(
    const core_property_node_t *parent_node, uint16_t *value);
typedef core_property_node_result_t (*core_property_node_s32_read_t)(
    const core_property_node_t *parent_node, int32_t *value);
typedef core_property_node_result_t (*core_property_node_u32_read_t)(
    const core_property_node_t *parent_node, uint32_t *value);
typedef core_property_node_result_t (*core_property_node_s64_read_t)(
    const core_property_node_t *parent_node, int64_t *value);
typedef core_property_node_result_t (*core_property_node_u64_read_t)(
    const core_property_node_t *parent_node, uint64_t *value);
typedef core_property_node_result_t (*core_property_node_bin_read_t)(
    const core_property_node_t *parent_node, void *value, size_t len);
typedef core_property_node_result_t (*core_property_node_str_read_t)(
    const core_property_node_t *parent_node, char *value, size_t len);
typedef core_property_node_result_t (*core_property_node_ipv4_read_t)(
    const core_property_node_t *parent_node, uint32_t *value);
typedef core_property_node_result_t (*core_property_node_float_read_t)(
    const core_property_node_t *parent_node, float *value);
typedef core_property_node_result_t (*core_property_node_double_read_t)(
    const core_property_node_t *parent_node, double *value);
typedef core_property_node_result_t (*core_property_node_attr_read_t)(
    const core_property_node_t *parent_node, const char *key, char *value, size_t len);
typedef core_property_node_result_t (*core_property_node_bool_read_t)(
    const core_property_node_t *parent_node, bool *value);
typedef core_property_node_result_t (*core_property_node_remove_t)(
    const core_property_node_t *node);
typedef core_property_node_result_t (*core_property_node_attr_remove_t)(
    const core_property_node_t *parent_node, const char *key);
typedef core_property_node_result_t (*core_property_node_copy_t)(
    core_property_node_t *dst_node, const core_property_node_t *src_node);

typedef struct core_property_node_api {
    uint16_t version;

    struct {
        core_property_node_name_get_t name_get;
        core_property_node_size_t size;
        core_property_node_search_t search;
        core_property_node_next_result_search_t next_result_search;
        core_property_node_child_get_t child_get;
        core_property_node_next_sibling_get_t next_sibling_get;
        core_property_node_void_create_t void_create;
        core_property_node_s8_create_t s8_create;
        core_property_node_u8_create_t u8_create;
        core_property_node_s16_create_t s16_create;
        core_property_node_u16_create_t u16_create;
        core_property_node_s32_create_t s32_create;
        core_property_node_u32_create_t u32_create;
        core_property_node_s64_create_t s64_create;
        core_property_node_u64_create_t u64_create;
        core_property_node_bin_create_t bin_create;
        core_property_node_str_create_t str_create;
        core_property_node_ipv4_create_t ipv4_create;
        core_property_node_float_create_t float_create;
        core_property_node_double_create_t double_create;
        core_property_node_attr_create_t attr_create;
        core_property_node_bool_create_t bool_create;
        core_property_node_s8_read_t s8_read;
        core_property_node_u8_read_t u8_read;
        core_property_node_s16_read_t s16_read;
        core_property_node_u16_read_t u16_read;
        core_property_node_s32_read_t s32_read;
        core_property_node_u32_read_t u32_read;
        core_property_node_s64_read_t s64_read;
        core_property_node_u64_read_t u64_read;
        core_property_node_bin_read_t bin_read;
        core_property_node_str_read_t str_read;
        core_property_node_ipv4_read_t ipv4_read;
        core_property_node_float_read_t float_read;
        core_property_node_double_read_t double_read;
        core_property_node_attr_read_t attr_read;
        core_property_node_bool_read_t bool_read;
        core_property_node_remove_t remove;
        core_property_node_attr_remove_t attr_remove;
        core_property_node_copy_t copy;
    } v1;
} core_property_node_api_t;

void core_property_node_trace_log_enable(bool enable);
void core_property_node_api_set(const core_property_node_api_t *impl);
void core_property_node_api_get(core_property_node_api_t *impl);

const char *
core_property_node_result_to_str(core_property_node_result_t result);

core_property_node_result_t core_property_node_name_get(
    const core_property_node_t *node, char *name, size_t len);
core_property_node_result_t
core_property_node_size(const core_property_node_t *node, size_t *size);
core_property_node_result_t core_property_node_search(
    const core_property_node_t *node,
    const char *path,
    core_property_node_t *node_out);
core_property_node_result_t core_property_node_next_result_search(
    const core_property_node_t *node, core_property_node_t *node_out);
core_property_node_result_t core_property_node_child_get(
    const core_property_node_t *node, core_property_node_t *node_out);
core_property_node_result_t core_property_node_next_sibling_get(
    const core_property_node_t *node, core_property_node_t *node_out);
// Doc remark: node_out is optional here
core_property_node_result_t core_property_node_void_create(
    const core_property_node_t *parent_node,
    const char *key,
    core_property_node_t *node_out);
core_property_node_result_t core_property_node_s8_create(
    const core_property_node_t *parent_node,
    const char *key,
    int8_t value,
    core_property_node_t *node_out);
core_property_node_result_t core_property_node_u8_create(
    const core_property_node_t *parent_node,
    const char *key,
    uint8_t value,
    core_property_node_t *node_out);
core_property_node_result_t core_property_node_s16_create(
    const core_property_node_t *parent_node,
    const char *key,
    int16_t value,
    core_property_node_t *node_out);
core_property_node_result_t core_property_node_u16_create(
    const core_property_node_t *parent_node,
    const char *key,
    uint16_t value,
    core_property_node_t *node_out);
core_property_node_result_t core_property_node_s32_create(
    const core_property_node_t *parent_node,
    const char *key,
    int32_t value,
    core_property_node_t *node_out);
core_property_node_result_t core_property_node_u32_create(
    const core_property_node_t *parent_node,
    const char *key,
    uint32_t value,
    core_property_node_t *node_out);
core_property_node_result_t core_property_node_s64_create(
    const core_property_node_t *parent_node,
    const char *key,
    int64_t value,
    core_property_node_t *node_out);
core_property_node_result_t core_property_node_u64_create(
    const core_property_node_t *parent_node,
    const char *key,
    uint64_t value,
    core_property_node_t *node_out);
core_property_node_result_t core_property_node_bin_create(
    const core_property_node_t *parent_node,
    const char *key,
    void *data,
    size_t len,
    core_property_node_t *node_out);
core_property_node_result_t core_property_node_str_create(
    const core_property_node_t *parent_node,
    const char *key,
    const char *value,
    core_property_node_t *node_out);
core_property_node_result_t core_property_node_ipv4_create(
    const core_property_node_t *parent_node,
    const char *key,
    uint32_t value,
    core_property_node_t *node_out);
core_property_node_result_t core_property_node_float_create(
    const core_property_node_t *parent_node,
    const char *key,
    float value,
    core_property_node_t *node_out);
core_property_node_result_t core_property_node_double_create(
    const core_property_node_t *parent_node,
    const char *key,
    double value,
    core_property_node_t *node_out);
core_property_node_result_t core_property_node_attr_create(
    const core_property_node_t *parent_node,
    const char *key,
    const char *value);
core_property_node_result_t core_property_node_bool_create(
    const core_property_node_t *parent_node,
    const char *key,
    bool value,
    core_property_node_t *node_out);
core_property_node_result_t core_property_node_s8_read(
    const core_property_node_t *parent_node, int8_t *value);
core_property_node_result_t core_property_node_u8_read(
    const core_property_node_t *parent_node, uint8_t *value);
core_property_node_result_t core_property_node_s16_read(
    const core_property_node_t *parent_node, int16_t *value);
core_property_node_result_t core_property_node_u16_read(
    const core_property_node_t *parent_node, uint16_t *value);
core_property_node_result_t core_property_node_s32_read(
    const core_property_node_t *parent_node, int32_t *value);
core_property_node_result_t core_property_node_u32_read(
    const core_property_node_t *parent_node, uint32_t *value);
core_property_node_result_t core_property_node_s64_read(
    const core_property_node_t *parent_node, int64_t *value);
core_property_node_result_t core_property_node_u64_read(
    const core_property_node_t *parent_node, uint64_t *value);
core_property_node_result_t core_property_node_bin_read(
    const core_property_node_t *parent_node, void *value, size_t len);
core_property_node_result_t core_property_node_str_read(
    const core_property_node_t *parent_node, char *value, size_t len);
core_property_node_result_t core_property_node_ipv4_read(
    const core_property_node_t *parent_node, uint32_t *value);
core_property_node_result_t core_property_node_float_read(
    const core_property_node_t *parent_node, float *value);
core_property_node_result_t core_property_node_double_read(
    const core_property_node_t *parent_node, double *value);
core_property_node_result_t core_property_node_attr_read(
    const core_property_node_t *parent_node, const char *key, char *value, size_t len);
core_property_node_result_t core_property_node_bool_read(
    const core_property_node_t *parent_node, bool *value);
core_property_node_result_t
core_property_node_remove(const core_property_node_t *node);
core_property_node_result_t core_property_node_attr_remove(
    const core_property_node_t *parent_node, const char *key);
core_property_node_result_t core_property_node_copy(
    core_property_node_t *dst_node, const core_property_node_t *src_node);

#endif