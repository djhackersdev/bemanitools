
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

enum property_create_flag {
    PROPERTY_FLAG_READ = 0x1,
    PROPERTY_FLAG_WRITE = 0x2,
    PROPERTY_FLAG_CREATE = 0x4,
    PROPERTY_FLAG_BINARY = 0x8,
    PROPERTY_FLAG_APPEND = 0x10,
};

enum property_node_traversal {
    TRAVERSE_PARENT = 0,
    TRAVERSE_FIRST_CHILD = 1,
    TRAVERSE_FIRST_ATTR = 2,
    TRAVERSE_FIRST_SIBLING = 3,
    TRAVERSE_NEXT_SIBLING = 4,
    TRAVERSE_PREVIOUS_SIBLING = 5,
    TRAVERSE_LAST_SIBLING = 6,
    TRAVERSE_NEXT_SEARCH_RESULT = 7,
    TRAVERSE_PREV_SEARCH_RESULT = 8,
};

enum property_type {
    PROPERTY_TYPE_VOID = 0x01,
    PROPERTY_TYPE_S8 = 0x02,
    PROPERTY_TYPE_U8 = 0x03,
    PROPERTY_TYPE_S16 = 0x04,
    PROPERTY_TYPE_U16 = 0x05,
    PROPERTY_TYPE_S32 = 0x06,
    PROPERTY_TYPE_U32 = 0x07,
    PROPERTY_TYPE_S64 = 0x08,
    PROPERTY_TYPE_U64 = 0x09,
    PROPERTY_TYPE_BIN = 0x0A,
    PROPERTY_TYPE_STR = 0x0B,
    PROPERTY_TYPE_IP4 = 0x0C,
    PROPERTY_TYPE_TIME = 0x0D,
    PROPERTY_TYPE_FLOAT = 0x0E,
    PROPERTY_TYPE_DOUBLE = 0x0F,
    PROPERTY_TYPE_4U8 = 0x25,
    PROPERTY_TYPE_ATTR = 0x2E,
    PROPERTY_TYPE_BOOL = 0x34,

    // Missing __type attribute
    PROPERTY_TYPE_VOID_WITH_ATTRIBUTES = 0x40 | PROPERTY_TYPE_VOID,
    PROPERTY_TYPE_ARRAY_S8 = 0x40 | PROPERTY_TYPE_S8,
    PROPERTY_TYPE_ARRAY_U8 = 0x40 | PROPERTY_TYPE_U8,
    PROPERTY_TYPE_ARRAY_S16 = 0x40 | PROPERTY_TYPE_S16,
    PROPERTY_TYPE_ARRAY_U16 = 0x40 | PROPERTY_TYPE_U16,
    PROPERTY_TYPE_ARRAY_S32 = 0x40 | PROPERTY_TYPE_S32,
    PROPERTY_TYPE_ARRAY_U32 = 0x40 | PROPERTY_TYPE_U32,
    PROPERTY_TYPE_ARRAY_S64 = 0x40 | PROPERTY_TYPE_S64,
    PROPERTY_TYPE_ARRAY_U64 = 0x40 | PROPERTY_TYPE_U64,
    PROPERTY_TYPE_ARRAY_TIME = 0x40 | PROPERTY_TYPE_TIME,
    PROPERTY_TYPE_ARRAY_BOOL = 0x40 | PROPERTY_TYPE_BOOL,

    PROPERTY_TYPE_STR_WITH_ATTRIBUTES = 0x40 | PROPERTY_TYPE_STR,
};

struct core_property;
struct core_property_node;

typedef struct core_property core_property_t;
typedef struct core_property_node_t core_property_node_t;

typedef int core_property_result_t;

const char *core_property_result_to_str(core_property_result_t result);

// TODO memory management, just ask for size and buffer allocated automatically?
core_property_result_t core_property_create(uint32_t size, core_property_t **result)

core_property_result_t core_property_file_load(const char *path, core_property_t **result);
core_property_result_t core_property_file_save(const core_property_t *property, const char *path);
core_property_result_t core_property_cstring_load(const char *cstring, core_property_t **result);

core_property_result_t core_property_size(const core_property_t *prop);

void core_property_log(const core_property_t *property, core_log_message_t log_impl);

core_property_t *core_property_clone(const core_property_t *property);

void core_property_free(core_property_t *property);

// ------------------------------------

core_property_result_t core_property_root_node_get(const core_property_t *prop, core_property_node_t **node);

core_property_result_t core_property_node_search(const core_property_node_t *root, const char *path, core_property_node_t **result);

void core_property_node_log(const core_property_node_t *node, core_log_message_t log_impl);

// TODO better clone API that actually clones with re-allocation etc?
core_property_result_t core_property_node_clone(
    core_property_t *dst_prop,
    const core_property_node_t *dst_node,
    const core_property_node_t *src_node,
    bool deep,
    core_property_node_t **result);

core_property_result_t core_property_node_void_create(core_property_node_t *parent_node, const char *key);
core_property_result_t core_property_node_s8_create(core_property_node_t *parent_node, const char *key, int8_t value);
core_property_result_t core_property_node_u8_create(core_property_node_t *parent_node, const char *key, uint8_t value);
core_property_result_t core_property_node_s16_create(core_property_node_t *parent_node, const char *key, int16_t value);
core_property_result_t core_property_node_u16_create(core_property_node_t *parent_node, const char *key, uint16_t value);
core_property_result_t core_property_node_s32_create(core_property_node_t *parent_node, const char *key, int32_t value);
core_property_result_t core_property_node_u32_create(core_property_node_t *parent_node, const char *key, uint32_t value);
core_property_result_t core_property_node_s64_create(core_property_node_t *parent_node, const char *key, int64_t value);
core_property_result_t core_property_node_u64_create(core_property_node_t *parent_node, const char *key, uint64_t value);
core_property_result_t core_property_node_bin_create(core_property_node_t *parent_node, const char *key, void *data, size_t len);
core_property_result_t core_property_node_str_create(core_property_node_t *parent_node, const char *key, const char *value);
core_property_result_t core_property_node_ipv4_create(core_property_node_t *parent_node, const char *key, uint32_t value);
core_property_result_t core_property_node_float_create(core_property_node_t *parent_node, const char *key, float value);
core_property_result_t core_property_node_double_create(core_property_node_t *parent_node, const char *key, double value);
core_property_result_t core_property_node_attr_create(core_property_node_t *parent_node, const char *key, const char *value);
core_property_result_t core_property_node_bool_create(core_property_node_t *parent_node, const char *key, bool value);

core_property_result_t core_property_node_s8_read(core_property_node_t *parent_node, const char *key, int8_t *value);
core_property_result_t core_property_node_u8_read(core_property_node_t *parent_node, const char *key,  uint8_t *value);
core_property_result_t core_property_node_s16_read(core_property_node_t *parent_node, const char *key, int16_t *value);
core_property_result_t core_property_node_u16_read(core_property_node_t *parent_node, const char *key, uint16_t *value);
core_property_result_t core_property_node_s32_read(core_property_node_t *parent_node, const char *key, int32_t *value);
core_property_result_t core_property_node_u32_read(core_property_node_t *parent_node, const char *key, uint32_t *value);
core_property_result_t core_property_node_s64_read(core_property_node_t *parent_node, const char *key, int64_t *value);
core_property_result_t core_property_node_u64_read(core_property_node_t *parent_node, const char *key, uint64_t *value);
core_property_result_t core_property_node_bin_read(core_property_node_t *parent_node, const char *key, void *value, size_t len);
core_property_result_t core_property_node_str_read(core_property_node_t *parent_node, const char *key, char *value, size_t len);
core_property_result_t core_property_node_ipv4_read(core_property_node_t *parent_node, const char *key, uint32_t *value);
core_property_result_t core_property_node_float_read(core_property_node_t *parent_node, const char *key, float *value);
core_property_result_t core_property_node_double_read(core_property_node_t *parent_node, const char *key, double *value);
core_property_result_t core_property_node_attr_read(core_property_node_t *parent_node, const char *key, char *value, size_t len);
core_property_result_t core_property_node_bool_read(core_property_node_t *parent_node, const char *key, bool *value);

int property_node_refer(
    struct property *prop,
    struct property_node *node,
    const char *name,
    enum property_type type,
    void *bytes,
    uint32_t nbytes);



enum property_type {
    PROPERTY_TYPE_VOID = 0x01,
    PROPERTY_TYPE_S8 = 0x02,
    PROPERTY_TYPE_U8 = 0x03,
    PROPERTY_TYPE_S16 = 0x04,
    PROPERTY_TYPE_U16 = 0x05,
    PROPERTY_TYPE_S32 = 0x06,
    PROPERTY_TYPE_U32 = 0x07,
    PROPERTY_TYPE_S64 = 0x08,
    PROPERTY_TYPE_U64 = 0x09,
    PROPERTY_TYPE_BIN = 0x0A,
    PROPERTY_TYPE_STR = 0x0B,
    PROPERTY_TYPE_IP4 = 0x0C,
    PROPERTY_TYPE_TIME = 0x0D,
    PROPERTY_TYPE_FLOAT = 0x0E,
    PROPERTY_TYPE_DOUBLE = 0x0F,
    PROPERTY_TYPE_4U8 = 0x25,
    PROPERTY_TYPE_ATTR = 0x2E,
    PROPERTY_TYPE_BOOL = 0x34,

    // Missing __type attribute
    PROPERTY_TYPE_VOID_WITH_ATTRIBUTES = 0x40 | PROPERTY_TYPE_VOID,
    PROPERTY_TYPE_ARRAY_S8 = 0x40 | PROPERTY_TYPE_S8,
    PROPERTY_TYPE_ARRAY_U8 = 0x40 | PROPERTY_TYPE_U8,
    PROPERTY_TYPE_ARRAY_S16 = 0x40 | PROPERTY_TYPE_S16,
    PROPERTY_TYPE_ARRAY_U16 = 0x40 | PROPERTY_TYPE_U16,
    PROPERTY_TYPE_ARRAY_S32 = 0x40 | PROPERTY_TYPE_S32,
    PROPERTY_TYPE_ARRAY_U32 = 0x40 | PROPERTY_TYPE_U32,
    PROPERTY_TYPE_ARRAY_S64 = 0x40 | PROPERTY_TYPE_S64,
    PROPERTY_TYPE_ARRAY_U64 = 0x40 | PROPERTY_TYPE_U64,
    PROPERTY_TYPE_ARRAY_TIME = 0x40 | PROPERTY_TYPE_TIME,
    PROPERTY_TYPE_ARRAY_BOOL = 0x40 | PROPERTY_TYPE_BOOL,

    PROPERTY_TYPE_STR_WITH_ATTRIBUTES = 0x40 | PROPERTY_TYPE_STR,
};







avs_error property_psmap_import(
    struct property *prop,
    struct property_node *root,
    void *dest,
    const struct property_psmap *psmap);



void property_node_name(struct property_node *node, char *chars, int nchars);
int property_node_refer(
    struct property *prop,
    struct property_node *node,
    const char *name,
    enum property_type type,
    void *bytes,
    uint32_t nbytes);
void property_node_remove(struct property_node *node);

struct property_node *property_node_traversal(
    struct property_node *node, enum property_node_traversal direction);