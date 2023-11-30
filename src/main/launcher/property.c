#define LOG_MODULE "launcher-property"

#include <windows.h>

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "imports/avs.h"

#include "launcher/property.h"

#include "util/log.h"
#include "util/mem.h"
#include "util/str.h"

typedef void (*rewinder)(uint32_t context);

static struct property *do_property_load(
    avs_reader_t reader, rewinder rewinder, uint32_t context, const char *name)
{
    struct property *prop;
    void *buffer;
    int nbytes;

    nbytes = property_read_query_memsize(reader, context, 0, 0);

    if (nbytes < 0) {
        log_fatal("%s: Error querying configuration file", name);
    }

    buffer = xmalloc(nbytes);
    prop = property_create(
        PROPERTY_FLAG_READ | PROPERTY_FLAG_WRITE | PROPERTY_FLAG_CREATE |
            PROPERTY_FLAG_APPEND,
        buffer,
        nbytes);
    rewinder(context);

    if (!property_insert_read(prop, 0, reader, context)) {
        log_fatal("%s: Error reading configuration file", name);
    }

    return prop;
}

static int boot_property_fread(uint32_t context, void *bytes, size_t nbytes)
{
    FILE *f;

    f = TlsGetValue(context);

    return fread(bytes, 1, nbytes, f);
}

static void boot_property_frewind(uint32_t context)
{
    FILE *f = TlsGetValue(context);
    rewind(f);
}

static void boot_property_log_node_tree_rec(
        struct property_node *parent_node,
        const char* parent_path)
{
    char cur_path[4096];
    // 256 found in AVS code as size used on property_node_name
    char cur_node_name[256];

    struct property_node* child_node;
    enum property_type property_type;

    // Carry on the full root path down the node tree
    property_node_name(parent_node, cur_node_name, sizeof(cur_node_name));

    str_cpy(cur_path, sizeof(cur_path), parent_path);
    str_cat(cur_path, sizeof(cur_path), "/");
    str_cat(cur_path, sizeof(cur_path), cur_node_name);

    child_node = property_node_traversal(parent_node, TRAVERSE_FIRST_CHILD);

    // parent node is a leaf node, print all data of it
    if (child_node == NULL) {
        property_type = property_node_type(parent_node);

        switch (property_type) {
            case PROPERTY_TYPE_VOID:
                log_misc("%s: <VOID>", cur_path);
                break;

            case PROPERTY_TYPE_S8:
                int8_t value_s8;

                property_node_read(parent_node, property_type, &value_s8, sizeof(value_s8));
                log_misc("%s: %d", cur_path, value_s8);
                break;

            case PROPERTY_TYPE_S16:
                int16_t value_s16;

                property_node_read(parent_node, property_type, &value_s16, sizeof(value_s16));
                log_misc("%s: %d", cur_path, value_s16);
                break;

            case PROPERTY_TYPE_S32:
                int32_t value_s32;

                property_node_read(parent_node, property_type, &value_s32, sizeof(value_s32));
                log_misc("%s: %d", cur_path, value_s32);
                break;

            case PROPERTY_TYPE_S64:
                int64_t value_s64;

                property_node_read(parent_node, property_type, &value_s64, sizeof(value_s64));
                log_misc("%s: %lld", cur_path, value_s64);
                break;
            
            case PROPERTY_TYPE_U8:
                uint8_t value_u8;

                property_node_read(parent_node, property_type, &value_u8, sizeof(value_u8));
                log_misc("%s: %u", cur_path, value_u8);
                break;

            case PROPERTY_TYPE_U16:
                uint16_t value_u16;

                property_node_read(parent_node, property_type, &value_u16, sizeof(value_u16));
                log_misc("%s: %u", cur_path, value_u16);
                break;

            case PROPERTY_TYPE_U32:
                uint32_t value_u32;

                property_node_read(parent_node, property_type, &value_u32, sizeof(value_u32));
                log_misc("%s: %u", cur_path, value_u32);
                break;

            case PROPERTY_TYPE_U64:
                uint64_t value_u64;

                property_node_read(parent_node, property_type, &value_u64, sizeof(value_u64));
                log_misc("%s: %llu", cur_path, value_u64);
                break;

            case PROPERTY_TYPE_STR:
                char value_str[4096];

                property_node_read(parent_node, property_type, value_str, sizeof(value_str));
                log_misc("%s: %s", cur_path, value_str);

                break;

            case PROPERTY_TYPE_BOOL:
                bool value_bool;

                property_node_read(parent_node, property_type, &value_bool, sizeof(value_bool));
                log_misc("%s: %d", cur_path, value_bool);

                break;

            case PROPERTY_TYPE_BIN:
                log_misc("%s: <BINARY>", cur_path);
                break;

            case PROPERTY_TYPE_ATTR:
                log_misc("%s: <ATTRIBUTE>", cur_path);
                break;

            // Treat as string
            case PROPERTY_TYPE_UNTYPED:
                char value_untyped[4096];

                property_node_read(parent_node, property_type, value_untyped, sizeof(value_untyped));
                log_misc("%s: %s", cur_path, value_untyped);

                break;

            default:
                log_misc("%s: <UNKNOWN TYPE> (%d)", cur_path, property_type);
                break;
        }
    } else {
        while (child_node) {
            boot_property_log_node_tree_rec(child_node, cur_path);

            child_node = property_node_traversal(child_node, TRAVERSE_NEXT_SIBLING);
        }
    }
}

void boot_property_log(struct property *property)
{
    boot_property_log_node_tree_rec(property_search(property, NULL, "/"), "");
}

void boot_property_node_log(struct property_node *node)
{
    boot_property_log_node_tree_rec(node, "");
}

struct property *boot_property_load(const char *filename)
{
    FILE *f;
    uint32_t f_keyhole;
    struct property *prop;

    /* AVS callbacks are only given a 32-bit context parameter, even in 64-bit
       builds of AVS. We allocate a 32-bit TLS key and pass the context in this
       manner instead. Inefficient, but it works. */

    f = fopen(filename, "r");

    f_keyhole = TlsAlloc();
    TlsSetValue(f_keyhole, f);

    if (f == NULL) {
        log_fatal("%s: Error opening configuration file", filename);
    }

    prop = do_property_load(
        boot_property_fread, boot_property_frewind, f_keyhole, filename);

    TlsFree(f_keyhole);

    fclose(f);

    return prop;
}

struct cstring_read_handle {
    const char *buffer;
    size_t buffer_len;
    size_t offset;
};

static int
boot_property_cstring_read(uint32_t context, void *bytes, size_t nbytes)
{
    int result = 0;
    struct cstring_read_handle *h = TlsGetValue(context);

    if (h->offset < h->buffer_len) {
        result = min(nbytes, h->buffer_len - h->offset);
        memcpy(bytes, (const void *) (h->buffer + h->offset), result);
        h->offset += result;
    }
    return result;
}

static void boot_property_cstring_rewind(uint32_t context)
{
    struct cstring_read_handle *h = TlsGetValue(context);
    h->offset = 0;
}

struct property *boot_property_load_cstring(const char *cstring)
{
    uint32_t s_keyhole;
    struct property *prop;

    // see above
    struct cstring_read_handle read_handle;
    read_handle.buffer = cstring;
    read_handle.buffer_len = strlen(cstring);
    read_handle.offset = 0;

    s_keyhole = TlsAlloc();
    TlsSetValue(s_keyhole, &read_handle);

    prop = do_property_load(
        boot_property_cstring_read,
        boot_property_cstring_rewind,
        s_keyhole,
        "<string>");

    TlsFree(s_keyhole);

    return prop;
}

static int boot_property_avs_read(uint32_t context, void *bytes, size_t nbytes)
{
    avs_desc desc = (avs_desc) context;
    return avs_fs_read(desc, bytes, nbytes);
}

static void boot_property_avs_rewind(uint32_t context)
{
    avs_desc desc = (avs_desc) context;
    avs_fs_lseek(desc, 0, AVS_SEEK_SET);
}

struct property *boot_property_load_avs(const char *filename)
{
    uint16_t mode;
    avs_desc desc;
    struct property *prop;

    // Apparently, the mode enum flags have changed, but this is currently unverified to apply
    // a generic solution to avs.h, so keep this contained for now as a "hack"
    // Using the wrong flag here, querying the eamuse-config.xml file fails on DDR on
    // older AVS versions
#if AVS_VERSION <= 1306
    // Currently only verified with 1306 and older on DDR
    mode = 0;
#else
    // Currently only verified with 1508 on DDR
    mode = AVS_FILE_READ;
#endif

    desc = avs_fs_open(filename, mode, AVS_FILE_FLAG_SHARE_READ);

    if (!desc) {
        log_fatal("%s: Error opening configuration file", filename);
    }

    prop = do_property_load(
        boot_property_avs_read, boot_property_avs_rewind, desc, filename);

    avs_fs_close(desc);

    return prop;
}

void boot_property_node_replace_u8(
        struct property *property,
        struct property_node *node,
        const char *name,
        uint8_t val)
{
    struct property_node *tmp;

    tmp = property_search(property, node, name);

    if (tmp) {
        property_node_remove(tmp);
    }

    property_node_create(property, node, PROPERTY_TYPE_U8, name, val);
}

void boot_property_node_replace_bool(
        struct property *property,
        struct property_node *node,
        const char *name,
        bool val)
{
    struct property_node *tmp;

    tmp = property_search(property, node, name);

    if (tmp) {
        property_node_remove(tmp);
    }

    property_node_create(property, node, PROPERTY_TYPE_BOOL, name, val);
}

void boot_property_node_replace_str(
        struct property *property,
        struct property_node *node,
        const char *name,
        const char *val)
{
    struct property_node *tmp;

    tmp = property_search(property, node, name);

    if (tmp) {
        property_node_remove(tmp);
    }

    property_node_create(property, node, PROPERTY_TYPE_STR, name, val);
}

void boot_property_free(struct property *prop)
{
    void *buffer;

    buffer = property_desc_to_buffer(prop);
    property_destroy(prop);
    free(buffer);
}
