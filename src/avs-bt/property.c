#define LOG_MODULE "avs-bt-property"

#include "imports/avs.h"

// TODO get remaining ones and also implement them to complete the API 

uint32_t property_read_query_memsize(
    avs_reader_t reader, uint32_t context, int unk0, int unk1)
{
    // TODO
    return 0;
}

struct property *property_create(int flags, void *buffer, uint32_t buffer_size)
{
    // TODO
    return NULL;
}

struct property_node *property_search(
    struct property *prop, struct property_node *root, const char *path)
{
    // TODO
    return NULL;
}

int property_insert_read(
    struct property *prop,
    struct property_node *root,
    avs_reader_t reader,
    uint32_t context)
{
    // TODO
    return -1;
}

int property_mem_write(struct property *prop, void *bytes, int nbytes)
{
    // TODO
    return -1;
}

void *property_desc_to_buffer(struct property *prop)
{
    // TODO
    return NULL;
}

avs_error property_query_size(struct property *prop)
{
    // TODO
    return -1;
}

void property_file_write(struct property *prop, const char *path)
{
    // TODO
}

int property_set_flag(struct property *prop, int flags, int mask)
{
    // TODO
    return -1;
}

void property_destroy(struct property *prop)
{
    // TODO
}

avs_error property_get_error(struct property *prop)
{
    // TODO
    return -1;
}

void property_clear_error(struct property *prop)
{
    // TODO
}

avs_error property_psmap_import(
    struct property *prop,
    struct property_node *root,
    void *dest,
    const struct property_psmap *psmap)
{
    // TODO
    return -1;   
}

avs_error property_psmap_export(
    struct property *prop,
    struct property_node *root,
    const void *src,
    const struct property_psmap *psmap)
{
    // TODO
    return -1;
}

struct property_node *property_node_clone(
    struct property *parent_prop,
    struct property_node *parent_node,
    struct property_node *src,
    bool deep)
{
    // TODo
    return NULL;
}

struct property_node *property_node_create(
    struct property *prop,
    struct property_node *parent,
    int type,
    const char *key,
    ...)
{
    // TODO
    return NULL;
}

void property_node_name(struct property_node *node, char *chars, int nchars)
{
    // TODO
    return;
}

const char *property_node_refdata(struct property_node *node)
{
    // TODO
    return NULL;
}

int property_node_refer(
    struct property *prop,
    struct property_node *node,
    const char *name,
    enum property_type type,
    void *bytes,
    uint32_t nbytes)
{
    // TODO
    return -1;
}

void property_node_remove(struct property_node *node)
{
    // TODO
}

enum property_type property_node_type(struct property_node *node)
{
    // TODO
    return -1;   
}

struct property_node *property_node_traversal(
    struct property_node *node, enum property_node_traversal direction)
{
    // TODO
    return NULL;   
}

int property_node_datasize(struct property_node *node)
{
    // TODO
    return -1;
}

avs_error property_node_read(struct property_node *node, enum property_type type, void* data, uint32_t data_size)
{
    // TODO
    return -1;
}