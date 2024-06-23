#define LOG_MODULE "core-property-node-mxml"

#include <inttypes.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include <mxml/mxml.h>

#include "core/property-mxml-internal.h"

#include "iface-core/log.h"

#include "util/hex.h"
#include "util/mem.h"
#include "util/str.h"

static core_property_node_result_t _core_property_node_mxml_node_type_create(
    const core_property_node_t *parent_node_,
    core_property_node_t *node_out_,
    const char *key,
    const char *type,
    const char *format,
    ...)
{
    core_property_mxml_internal_property_node_t *parent_node;
    core_property_mxml_internal_property_node_t *node_out;
    mxml_node_t *node_new;
    mxml_node_t *tmp;
    va_list	args;
    char buffer[16384];

    parent_node = (core_property_mxml_internal_property_node_t *) parent_node_;
    node_out = (core_property_mxml_internal_property_node_t *) node_out_;

    node_new = mxmlNewElement(parent_node->node, key);

    if (node_new == NULL) {
        return CORE_PROPERTY_RESULT_ERROR_INTERNAL;
    }

    // No return value
    mxmlElementSetAttr(node_new, "__type", type);

    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    tmp = mxmlNewText(node_new, false, buffer);

    if (tmp == NULL) {
        return CORE_PROPERTY_RESULT_ERROR_INTERNAL;
    }

    if (node_out) {
        memset(node_out, 0, sizeof(core_property_mxml_internal_property_node_t));

        node_out->property = parent_node->property;
        node_out->node = node_new;
    }

    return CORE_PROPERTY_NODE_RESULT_SUCCESS;
}

static core_property_node_result_t _core_property_node_mxml_name_get(
    const core_property_node_t *node_, char *name, size_t len)
{
    core_property_mxml_internal_property_node_t *node;
    const char *elem_name;

    node = (core_property_mxml_internal_property_node_t *) node_;

    elem_name = mxmlGetElement(node->node);

    if (elem_name == NULL) {
        return CORE_PROPERTY_NODE_RESULT_INVALID_NODE_TYPE;
    }

    str_cpy(name, len, elem_name);

    return CORE_PROPERTY_NODE_RESULT_SUCCESS;
}

static core_property_node_result_t
_core_property_node_mxml_size(const core_property_node_t *node_, size_t *size)
{
    *size = CORE_PROPERTY_MXML_INTERNAL_FIXED_SIZE_DUMMY;

    return CORE_PROPERTY_NODE_RESULT_SUCCESS;
}

static core_property_node_result_t _core_property_node_mxml_search(
    const core_property_node_t *node_,
    const char *path,
    core_property_node_t *node_out_)
{
    core_property_mxml_internal_property_node_t *node;
    core_property_mxml_internal_property_node_t *node_out;
    mxml_node_t *tmp;

    node = (core_property_mxml_internal_property_node_t *) node_;
    node_out = (core_property_mxml_internal_property_node_t *) node_out_;

    memset(node_out, 0, sizeof(core_property_mxml_internal_property_node_t));

    tmp = mxmlFindPath(node->node, path);

    if (!tmp) {
        return CORE_PROPERTY_NODE_RESULT_NODE_NOT_FOUND;
    }

    node_out->property = node->property;
    node_out->node = tmp;

    return CORE_PROPERTY_NODE_RESULT_SUCCESS;
}

static core_property_node_result_t _core_property_node_mxml_next_result_search(
    const core_property_node_t *node_, core_property_node_t *node_out_)
{
    core_property_mxml_internal_property_node_t *node;
    core_property_mxml_internal_property_node_t *node_out;
    mxml_node_t *tmp;
    const char *elem_name;

    node = (core_property_mxml_internal_property_node_t *) node_;
    node_out = (core_property_mxml_internal_property_node_t *) node_out_;

    memset(node_out, 0, sizeof(core_property_mxml_internal_property_node_t));

    elem_name = mxmlGetElement(node->node);

    if (elem_name == NULL) {
        return CORE_PROPERTY_NODE_RESULT_ERROR_INTERNAL;
    }

    tmp = mxmlFindElement(node->node, node->node, elem_name, NULL, NULL, MXML_DESCEND_NONE);

    if (tmp == NULL) {
        return CORE_PROPERTY_NODE_RESULT_NODE_NOT_FOUND;
    }

    node_out->property = node->property;
    node_out->node = tmp;

    return CORE_PROPERTY_NODE_RESULT_SUCCESS;
}

static core_property_node_result_t _core_property_node_mxml_child_get(
    const core_property_node_t *node_, core_property_node_t *node_out_)
{
    core_property_mxml_internal_property_node_t *node;
    core_property_mxml_internal_property_node_t *node_out;
    mxml_node_t *tmp;
    mxml_node_t *tree;

    node = (core_property_mxml_internal_property_node_t *) node_;
    node_out = (core_property_mxml_internal_property_node_t *) node_out_;

    tree = node->node;

    tmp = mxmlFindElement(tree, tree, NULL, NULL, NULL, MXML_DESCEND_FIRST);

    if (!tmp) {
        return CORE_PROPERTY_NODE_RESULT_NODE_NOT_FOUND;
    }

    memset(node_out, 0, sizeof(core_property_mxml_internal_property_node_t));

    node_out->property = node->property;
    node_out->node = tmp;
    node_out->node_root_iter = tree;

    return CORE_PROPERTY_NODE_RESULT_SUCCESS;
}

static core_property_node_result_t _core_property_node_mxml_next_sibling_get(
    const core_property_node_t *node_, core_property_node_t *node_out_)
{
    core_property_mxml_internal_property_node_t *node;
    core_property_mxml_internal_property_node_t *node_out;
    mxml_node_t *tmp;

    node = (core_property_mxml_internal_property_node_t *) node_;
    node_out = (core_property_mxml_internal_property_node_t *) node_out_;

    log_assert(node->node_root_iter);

    tmp = mxmlFindElement(node->node, node->node_root_iter, NULL, NULL, NULL, MXML_DESCEND_NONE);

    if (!tmp) {
        return CORE_PROPERTY_NODE_RESULT_NODE_NOT_FOUND;
    }

    memset(node_out, 0, sizeof(core_property_mxml_internal_property_node_t));

    node_out->property = node->property;
    node_out->node = tmp;
    node_out->node_root_iter = node->node_root_iter;

    return CORE_PROPERTY_NODE_RESULT_SUCCESS;
}

static core_property_node_result_t _core_property_node_mxml_void_create(
    const core_property_node_t *parent_node_,
    const char *key,
    core_property_node_t *node_out_)
{
    core_property_mxml_internal_property_node_t *parent_node;
    core_property_mxml_internal_property_node_t *node_out;
    mxml_node_t *tmp;

    parent_node = (core_property_mxml_internal_property_node_t *) parent_node_;
    node_out = (core_property_mxml_internal_property_node_t *) node_out_;

    tmp = mxmlNewElement(parent_node->node, key);

    if (!tmp) {
        return CORE_PROPERTY_NODE_RESULT_ERROR_INTERNAL;
    }

    if (node_out) {
        memset(node_out, 0, sizeof(core_property_mxml_internal_property_node_t));

        node_out->property = parent_node->property;
        node_out->node = tmp;
    }

    return CORE_PROPERTY_NODE_RESULT_SUCCESS;
}

static core_property_node_result_t _core_property_node_mxml_s8_create(
    const core_property_node_t *parent_node_,
    const char *key,
    int8_t value,
    core_property_node_t *node_out_)
{
    return _core_property_node_mxml_node_type_create(parent_node_, node_out_, key, "s8", "%d", value);
}

static core_property_node_result_t _core_property_node_mxml_u8_create(
    const core_property_node_t *parent_node_,
    const char *key,
    uint8_t value,
    core_property_node_t *node_out_)
{
    return _core_property_node_mxml_node_type_create(parent_node_, node_out_, key, "u8", "%u", value);
}

static core_property_node_result_t _core_property_node_mxml_s16_create(
    const core_property_node_t *parent_node_,
    const char *key,
    int16_t value,
    core_property_node_t *node_out_)
{
    return _core_property_node_mxml_node_type_create(parent_node_, node_out_, key, "s16", "%d", value);
}

static core_property_node_result_t _core_property_node_mxml_u16_create(
    const core_property_node_t *parent_node_,
    const char *key,
    uint16_t value,
    core_property_node_t *node_out_)
{
    return _core_property_node_mxml_node_type_create(parent_node_, node_out_, key, "u16", "%u", value);
}

static core_property_node_result_t _core_property_node_mxml_s32_create(
    const core_property_node_t *parent_node_,
    const char *key,
    int32_t value,
    core_property_node_t *node_out_)
{
    return _core_property_node_mxml_node_type_create(parent_node_, node_out_, key, "s32", "%d", value);
}

static core_property_node_result_t _core_property_node_mxml_u32_create(
    const core_property_node_t *parent_node_,
    const char *key,
    uint32_t value,
    core_property_node_t *node_out_)
{
    return _core_property_node_mxml_node_type_create(parent_node_, node_out_, key, "u32", "%u", value);
}

static core_property_node_result_t _core_property_node_mxml_s64_create(
    const core_property_node_t *parent_node_,
    const char *key,
    int64_t value,
    core_property_node_t *node_out_)
{
    return _core_property_node_mxml_node_type_create(parent_node_, node_out_, key, "s64", "%lld", value);
}

static core_property_node_result_t _core_property_node_mxml_u64_create(
    const core_property_node_t *parent_node_,
    const char *key,
    uint64_t value,
    core_property_node_t *node_out_)
{
    return _core_property_node_mxml_node_type_create(parent_node_, node_out_, key, "u64", "%llu", value);
}

static core_property_node_result_t _core_property_node_mxml_bin_create(
    const core_property_node_t *parent_node_,
    const char *key,
    void *data,
    size_t len,
    core_property_node_t *node_out_)
{
    size_t size;
    char *buffer;
    core_property_node_result_t result;

    // Each byte needs 2 chars + null terminator at the end
    size = len * 2 + 1;
    buffer = xmalloc(size);

    hex_encode_uc(data, len, buffer, size);

    result = _core_property_node_mxml_node_type_create(parent_node_, node_out_, key, "bin", "%s", buffer);

    free(buffer);

    return result;
}

static core_property_node_result_t _core_property_node_mxml_str_create(
    const core_property_node_t *parent_node_,
    const char *key,
    const char *value,
    core_property_node_t *node_out_)
{
    return _core_property_node_mxml_node_type_create(parent_node_, node_out_, key, "str", "%s", value);
}

static core_property_node_result_t _core_property_node_mxml_ipv4_create(
    const core_property_node_t *parent_node_,
    const char *key,
    uint32_t value,
    core_property_node_t *node_out_)
{
    char buffer[4 * 3 + 3 + 1];
    core_property_node_result_t result;

    str_format(buffer, sizeof(buffer), "%d.%d.%d.%d", 
        (value >> 24) & 0xFF, 
        (value >> 16) & 0xFF,
        (value >> 8) & 0xFF,
        value & 0xFF);

    result = _core_property_node_mxml_node_type_create(parent_node_, node_out_, key, "ip4", "%s", buffer);

    return result;
}

static core_property_node_result_t _core_property_node_mxml_float_create(
    const core_property_node_t *parent_node_,
    const char *key,
    float value,
    core_property_node_t *node_out_)
{
    return _core_property_node_mxml_node_type_create(parent_node_, node_out_, key, "float", "%f", value);
}

static core_property_node_result_t _core_property_node_mxml_double_create(
    const core_property_node_t *parent_node_,
    const char *key,
    double value,
    core_property_node_t *node_out_)
{
    return _core_property_node_mxml_node_type_create(parent_node_, node_out_, key, "double", "%f", value);
}

static core_property_node_result_t _core_property_node_mxml_attr_create(
    const core_property_node_t *parent_node_,
    const char *key,
    const char *value)
{
    core_property_mxml_internal_property_node_t *parent_node;

    parent_node = (core_property_mxml_internal_property_node_t *) parent_node_;

    // No return value
    mxmlElementSetAttr(parent_node->node, key, value);

    return CORE_PROPERTY_NODE_RESULT_SUCCESS;
}

static core_property_node_result_t _core_property_node_mxml_bool_create(
    const core_property_node_t *parent_node_,
    const char *key,
    bool value,
    core_property_node_t *node_out_)
{
    return _core_property_node_mxml_node_type_create(parent_node_, node_out_, key, "bool", "%d", value ? "1" : "0");
}

static core_property_node_result_t _core_property_node_mxml_s8_read(
    const core_property_node_t *parent_node_, int8_t *value)
{
    core_property_mxml_internal_property_node_t *parent_node;
    const char *attr_type;
    const char *text;
    int64_t tmp;

    parent_node = (core_property_mxml_internal_property_node_t *) parent_node_;

    attr_type = mxmlElementGetAttr(parent_node->node, "__type");

    if (attr_type == NULL) {
        return CORE_PROPERTY_NODE_RESULT_INVALID_NODE_STRUCTURE;
    }

    if (!str_eq(attr_type, "s8")) {
        return CORE_PROPERTY_NODE_RESULT_INVALID_NODE_TYPE;
    }

    text = mxmlGetText(parent_node->node, NULL);

    if (!text) {
        return CORE_PROPERTY_NODE_RESULT_INVALID_NODE_DATA;
    }

    errno = 0;

    tmp = strtoll(text, NULL, 10);

    if (errno != 0) {
        return CORE_PROPERTY_NODE_RESULT_INVALID_NODE_DATA;
    }

    if (tmp < INT8_MIN || tmp > INT8_MAX) {
        return CORE_PROPERTY_NODE_RESULT_INVALID_NODE_DATA;
    }

    *value = tmp;

    return CORE_PROPERTY_NODE_RESULT_SUCCESS;
}

static core_property_node_result_t _core_property_node_mxml_u8_read(
    const core_property_node_t *parent_node_, uint8_t *value)
{
    core_property_mxml_internal_property_node_t *parent_node;
    const char *attr_type;
    const char *text;
    int64_t tmp;

    parent_node = (core_property_mxml_internal_property_node_t *) parent_node_;

    attr_type = mxmlElementGetAttr(parent_node->node, "__type");

    if (attr_type == NULL) {
        return CORE_PROPERTY_NODE_RESULT_INVALID_NODE_STRUCTURE;
    }

    if (!str_eq(attr_type, "u8")) {
        return CORE_PROPERTY_NODE_RESULT_INVALID_NODE_TYPE;
    }

    text = mxmlGetText(parent_node->node, NULL);

    if (!text) {
        return CORE_PROPERTY_NODE_RESULT_INVALID_NODE_DATA;
    }

    errno = 0;

    tmp = strtoll(text, NULL, 10);

    if (errno != 0) {
        return CORE_PROPERTY_NODE_RESULT_INVALID_NODE_DATA;
    }

    if (tmp < 0 || tmp > UINT8_MAX) {
        return CORE_PROPERTY_NODE_RESULT_INVALID_NODE_DATA;
    }

    *value = tmp;

    return CORE_PROPERTY_NODE_RESULT_SUCCESS;
}

static core_property_node_result_t _core_property_node_mxml_s16_read(
    const core_property_node_t *parent_node_, int16_t *value)
{
    core_property_mxml_internal_property_node_t *parent_node;
    const char *attr_type;
    const char *text;
    int64_t tmp;

    parent_node = (core_property_mxml_internal_property_node_t *) parent_node_;

    attr_type = mxmlElementGetAttr(parent_node->node, "__type");

    if (attr_type == NULL) {
        return CORE_PROPERTY_NODE_RESULT_INVALID_NODE_STRUCTURE;
    }

    if (!str_eq(attr_type, "s16")) {
        return CORE_PROPERTY_NODE_RESULT_INVALID_NODE_TYPE;
    }

    text = mxmlGetText(parent_node->node, NULL);

    if (!text) {
        return CORE_PROPERTY_NODE_RESULT_INVALID_NODE_DATA;
    }

    errno = 0;

    tmp = strtoll(text, NULL, 10);

    if (errno != 0) {
        return CORE_PROPERTY_NODE_RESULT_INVALID_NODE_DATA;
    }

    if (tmp < INT16_MIN || tmp > INT16_MAX) {
        return CORE_PROPERTY_NODE_RESULT_INVALID_NODE_DATA;
    }

    *value = tmp;

    return CORE_PROPERTY_NODE_RESULT_SUCCESS;   
}

static core_property_node_result_t _core_property_node_mxml_u16_read(
    const core_property_node_t *parent_node_, uint16_t *value)
{
    core_property_mxml_internal_property_node_t *parent_node;
    const char *attr_type;
    const char *text;
    int64_t tmp;

    parent_node = (core_property_mxml_internal_property_node_t *) parent_node_;

    attr_type = mxmlElementGetAttr(parent_node->node, "__type");

    if (attr_type == NULL) {
        return CORE_PROPERTY_NODE_RESULT_INVALID_NODE_STRUCTURE;
    }

    if (!str_eq(attr_type, "u16")) {
        return CORE_PROPERTY_NODE_RESULT_INVALID_NODE_TYPE;
    }

    text = mxmlGetText(parent_node->node, NULL);

    if (!text) {
        return CORE_PROPERTY_NODE_RESULT_INVALID_NODE_DATA;
    }

    errno = 0;

    tmp = strtoll(text, NULL, 10);

    if (errno != 0) {
        return CORE_PROPERTY_NODE_RESULT_INVALID_NODE_DATA;
    }

    if (tmp < 0 || tmp > UINT16_MAX) {
        return CORE_PROPERTY_NODE_RESULT_INVALID_NODE_DATA;
    }

    *value = tmp;

    return CORE_PROPERTY_NODE_RESULT_SUCCESS;  
}

static core_property_node_result_t _core_property_node_mxml_s32_read(
    const core_property_node_t *parent_node_, int32_t *value)
{
    core_property_mxml_internal_property_node_t *parent_node;
    const char *attr_type;
    const char *text;
    int64_t tmp;

    parent_node = (core_property_mxml_internal_property_node_t *) parent_node_;

    attr_type = mxmlElementGetAttr(parent_node->node, "__type");

    if (attr_type == NULL) {
        return CORE_PROPERTY_NODE_RESULT_INVALID_NODE_STRUCTURE;
    }

    if (!str_eq(attr_type, "s32")) {
        return CORE_PROPERTY_NODE_RESULT_INVALID_NODE_TYPE;
    }

    text = mxmlGetText(parent_node->node, NULL);

    if (!text) {
        return CORE_PROPERTY_NODE_RESULT_INVALID_NODE_DATA;
    }

    errno = 0;

    tmp = strtoll(text, NULL, 10);

    if (errno != 0) {
        return CORE_PROPERTY_NODE_RESULT_INVALID_NODE_DATA;
    }

    if (tmp < INT32_MIN || tmp > INT32_MAX) {
        return CORE_PROPERTY_NODE_RESULT_INVALID_NODE_DATA;
    }

    *value = tmp;

    return CORE_PROPERTY_NODE_RESULT_SUCCESS;  
}

static core_property_node_result_t _core_property_node_mxml_u32_read(
    const core_property_node_t *parent_node_, uint32_t *value)
{
core_property_mxml_internal_property_node_t *parent_node;
    const char *attr_type;
    const char *text;
    int64_t tmp;

    parent_node = (core_property_mxml_internal_property_node_t *) parent_node_;

    attr_type = mxmlElementGetAttr(parent_node->node, "__type");

    if (attr_type == NULL) {
        return CORE_PROPERTY_NODE_RESULT_INVALID_NODE_STRUCTURE;
    }

    if (!str_eq(attr_type, "u32")) {
        return CORE_PROPERTY_NODE_RESULT_INVALID_NODE_TYPE;
    }

    text = mxmlGetText(parent_node->node, NULL);

    if (!text) {
        return CORE_PROPERTY_NODE_RESULT_INVALID_NODE_DATA;
    }

    errno = 0;

    tmp = strtoll(text, NULL, 10);

    if (errno != 0) {
        return CORE_PROPERTY_NODE_RESULT_INVALID_NODE_DATA;
    }

    if (tmp < 0 || tmp > UINT32_MAX) {
        return CORE_PROPERTY_NODE_RESULT_INVALID_NODE_DATA;
    }

    *value = tmp;

    return CORE_PROPERTY_NODE_RESULT_SUCCESS;  
}

static core_property_node_result_t _core_property_node_mxml_s64_read(
    const core_property_node_t *parent_node_, int64_t *value)
{
    core_property_mxml_internal_property_node_t *parent_node;
    const char *attr_type;
    const char *text;
    int64_t tmp;

    parent_node = (core_property_mxml_internal_property_node_t *) parent_node_;

    attr_type = mxmlElementGetAttr(parent_node->node, "__type");

    if (attr_type == NULL) {
        return CORE_PROPERTY_NODE_RESULT_INVALID_NODE_STRUCTURE;
    }

    if (!str_eq(attr_type, "s64")) {
        return CORE_PROPERTY_NODE_RESULT_INVALID_NODE_TYPE;
    }

    text = mxmlGetText(parent_node->node, NULL);

    if (!text) {
        return CORE_PROPERTY_NODE_RESULT_INVALID_NODE_DATA;
    }

    errno = 0;

    tmp = strtoll(text, NULL, 10);

    if (errno != 0) {
        return CORE_PROPERTY_NODE_RESULT_INVALID_NODE_DATA;
    }

    *value = tmp;

    return CORE_PROPERTY_NODE_RESULT_SUCCESS;  
}

static core_property_node_result_t _core_property_node_mxml_u64_read(
    const core_property_node_t *parent_node_, uint64_t *value)
{
    core_property_mxml_internal_property_node_t *parent_node;
    const char *attr_type;
    const char *text;
    uint64_t tmp;

    parent_node = (core_property_mxml_internal_property_node_t *) parent_node_;

    attr_type = mxmlElementGetAttr(parent_node->node, "__type");

    if (attr_type == NULL) {
        return CORE_PROPERTY_NODE_RESULT_INVALID_NODE_STRUCTURE;
    }

    if (!str_eq(attr_type, "u64")) {
        return CORE_PROPERTY_NODE_RESULT_INVALID_NODE_TYPE;
    }

    text = mxmlGetText(parent_node->node, NULL);

    if (!text) {
        return CORE_PROPERTY_NODE_RESULT_INVALID_NODE_DATA;
    }

    errno = 0;

    tmp = strtoull(text, NULL, 10);

    if (errno != 0) {
        return CORE_PROPERTY_NODE_RESULT_INVALID_NODE_DATA;
    }

    *value = tmp;

    return CORE_PROPERTY_NODE_RESULT_SUCCESS;  
}

static core_property_node_result_t _core_property_node_mxml_bin_read(
    const core_property_node_t *parent_node_, void *value, size_t len)
{
    core_property_mxml_internal_property_node_t *parent_node;
    const char *attr_type;
    const char *text;

    parent_node = (core_property_mxml_internal_property_node_t *) parent_node_;

    attr_type = mxmlElementGetAttr(parent_node->node, "__type");

    if (attr_type == NULL) {
        return CORE_PROPERTY_NODE_RESULT_INVALID_NODE_STRUCTURE;
    }

    if (!str_eq(attr_type, "bin")) {
        return CORE_PROPERTY_NODE_RESULT_INVALID_NODE_TYPE;
    }

    text = mxmlGetText(parent_node->node, NULL);

    if (!text) {
        return CORE_PROPERTY_NODE_RESULT_INVALID_NODE_DATA;
    }

    if (!hex_decode(value, len, text, strlen(text))) {
        return CORE_PROPERTY_NODE_RESULT_INVALID_NODE_DATA;
    }

    return CORE_PROPERTY_NODE_RESULT_SUCCESS; 
}

static core_property_node_result_t _core_property_node_mxml_str_read(
    const core_property_node_t *parent_node_, char *value, size_t len)
{
    core_property_mxml_internal_property_node_t *parent_node;
    const char *attr_type;
    const char *text;

    parent_node = (core_property_mxml_internal_property_node_t *) parent_node_;

    attr_type = mxmlElementGetAttr(parent_node->node, "__type");

    if (attr_type == NULL) {
        return CORE_PROPERTY_NODE_RESULT_INVALID_NODE_STRUCTURE;
    }

    if (!str_eq(attr_type, "str")) {
        return CORE_PROPERTY_NODE_RESULT_INVALID_NODE_TYPE;
    }

    text = mxmlGetText(parent_node->node, NULL);

    if (!text) {
        return CORE_PROPERTY_NODE_RESULT_INVALID_NODE_DATA;
    }

    str_cpy(value, len, text);

    return CORE_PROPERTY_NODE_RESULT_SUCCESS;
}

static core_property_node_result_t _core_property_node_mxml_ipv4_read(
    const core_property_node_t *parent_node_, uint32_t *value)
{
    core_property_mxml_internal_property_node_t *parent_node;
    const char *attr_type;
    const char *text;
    uint8_t *tmp;
    int res;

    parent_node = (core_property_mxml_internal_property_node_t *) parent_node_;

    attr_type = mxmlElementGetAttr(parent_node->node, "__type");

    if (attr_type == NULL) {
        return CORE_PROPERTY_NODE_RESULT_INVALID_NODE_STRUCTURE;
    }

    if (!str_eq(attr_type, "ip4")) {
        return CORE_PROPERTY_NODE_RESULT_INVALID_NODE_TYPE;
    }

    text = mxmlGetText(parent_node->node, NULL);

    if (!text) {
        return CORE_PROPERTY_NODE_RESULT_INVALID_NODE_DATA;
    }

    *value = 0;
    tmp = (uint8_t*) value;

    res = sscanf(text, "%u.%u.%u.%u", &tmp[3], &tmp[2], &tmp[1], &tmp[0]);

    if (res != 4) {
        return CORE_PROPERTY_NODE_RESULT_INVALID_NODE_DATA;
    }

    return CORE_PROPERTY_NODE_RESULT_SUCCESS;
}

static core_property_node_result_t _core_property_node_mxml_float_read(
    const core_property_node_t *parent_node_, float *value)
{
    core_property_mxml_internal_property_node_t *parent_node;
    const char *attr_type;
    const char *text;
    float tmp;

    parent_node = (core_property_mxml_internal_property_node_t *) parent_node_;

    attr_type = mxmlElementGetAttr(parent_node->node, "__type");

    if (attr_type == NULL) {
        return CORE_PROPERTY_NODE_RESULT_INVALID_NODE_STRUCTURE;
    }

    if (!str_eq(attr_type, "float")) {
        return CORE_PROPERTY_NODE_RESULT_INVALID_NODE_TYPE;
    }

    text = mxmlGetText(parent_node->node, NULL);

    if (!text) {
        return CORE_PROPERTY_NODE_RESULT_INVALID_NODE_DATA;
    }

    errno = 0;

    tmp = strtof(text, NULL);

    if (errno != 0) {
        return CORE_PROPERTY_NODE_RESULT_INVALID_NODE_DATA;
    }

    *value = tmp;

    return CORE_PROPERTY_NODE_RESULT_SUCCESS;
}

static core_property_node_result_t _core_property_node_mxml_double_read(
    const core_property_node_t *parent_node_, double *value)
{
    core_property_mxml_internal_property_node_t *parent_node;
    const char *attr_type;
    const char *text;
    double tmp;

    parent_node = (core_property_mxml_internal_property_node_t *) parent_node_;

    attr_type = mxmlElementGetAttr(parent_node->node, "__type");

    if (attr_type == NULL) {
        return CORE_PROPERTY_NODE_RESULT_INVALID_NODE_STRUCTURE;
    }

    if (!str_eq(attr_type, "double")) {
        return CORE_PROPERTY_NODE_RESULT_INVALID_NODE_TYPE;
    }

    text = mxmlGetText(parent_node->node, NULL);

    if (!text) {
        return CORE_PROPERTY_NODE_RESULT_INVALID_NODE_DATA;
    }

    errno = 0;

    tmp = strtod(text, NULL);

    if (errno != 0) {
        return CORE_PROPERTY_NODE_RESULT_INVALID_NODE_DATA;
    }

    *value = tmp;

    return CORE_PROPERTY_NODE_RESULT_SUCCESS;
}

static core_property_node_result_t _core_property_node_mxml_attr_read(
    const core_property_node_t *parent_node_, const char *key, char *value, size_t len)
{
    core_property_mxml_internal_property_node_t *parent_node;
    const char *attr;

    parent_node = (core_property_mxml_internal_property_node_t *) parent_node_;

    attr = mxmlElementGetAttr(parent_node->node, key);

    if (attr == NULL) {
        return CORE_PROPERTY_NODE_RESULT_NODE_NOT_FOUND;
    }

    str_cpy(value, len, attr);

    return CORE_PROPERTY_NODE_RESULT_SUCCESS;
}

static core_property_node_result_t _core_property_node_mxml_bool_read(
    const core_property_node_t *parent_node_, bool *value)
{
    core_property_mxml_internal_property_node_t *parent_node;
    const char *attr_type;
    const char *text;
    int64_t tmp;

    parent_node = (core_property_mxml_internal_property_node_t *) parent_node_;

    attr_type = mxmlElementGetAttr(parent_node->node, "__type");

    if (attr_type == NULL) {
        return CORE_PROPERTY_NODE_RESULT_INVALID_NODE_STRUCTURE;
    }

    if (!str_eq(attr_type, "bool")) {
        return CORE_PROPERTY_NODE_RESULT_INVALID_NODE_TYPE;
    }

    text = mxmlGetText(parent_node->node, NULL);

    if (!text) {
        return CORE_PROPERTY_NODE_RESULT_INVALID_NODE_DATA;
    }

    errno = 0;

    tmp = strtoll(text, NULL, 10);

    if (errno != 0) {
        return CORE_PROPERTY_NODE_RESULT_INVALID_NODE_DATA;
    }

    if (tmp != 0 && tmp != 1) {
        return CORE_PROPERTY_NODE_RESULT_INVALID_NODE_DATA; 
    }

    *value = tmp > 0;

    return CORE_PROPERTY_NODE_RESULT_SUCCESS;
}

static core_property_node_result_t
_core_property_node_mxml_remove(const core_property_node_t *node_)
{
    core_property_mxml_internal_property_node_t *node;

    node = (core_property_mxml_internal_property_node_t *) node_;

    mxmlDelete(node->node);

    return CORE_PROPERTY_NODE_RESULT_SUCCESS;
}

static core_property_node_result_t _core_property_node_mxml_attr_remove(
    const core_property_node_t *parent_node_, const char *key)
{
    core_property_mxml_internal_property_node_t *parent_node;

    parent_node = (core_property_mxml_internal_property_node_t *) parent_node_;

    mxmlElementClearAttr(parent_node->node, key);

    return CORE_PROPERTY_NODE_RESULT_SUCCESS;
}

static core_property_node_result_t _core_property_node_mxml_copy(
    core_property_node_t *dst_node_, const core_property_node_t *src_node_)
{
    core_property_mxml_internal_property_node_t *dst_node;
    const core_property_mxml_internal_property_node_t *src_node;

    dst_node = (core_property_mxml_internal_property_node_t *) dst_node_;
    src_node = (const core_property_mxml_internal_property_node_t *) src_node_;

    char *str_copy;
    mxml_node_t *node_cloned;

    // Ensure actual cloning by storing and loading this
    // Just "adding it" to the other tree with mxml creates a reference only

    str_copy = mxmlSaveAllocString(src_node->node, NULL);

    if (str_copy == NULL) {
        return CORE_PROPERTY_RESULT_ERROR_READ;
    }

    node_cloned = mxmlLoadString(NULL, NULL, str_copy);

    if (node_cloned == NULL) {
        return CORE_PROPERTY_RESULT_ERROR_WRITE;
    }

    mxmlAdd(dst_node->node, MXML_ADD_AFTER, NULL, node_cloned);

    return CORE_PROPERTY_RESULT_SUCCESS;
}

void core_property_node_mxml_core_api_get(core_property_node_api_t *api)
{
    log_assert(api);

    api->version = 1;

    api->v1.name_get = _core_property_node_mxml_name_get;
    api->v1.size = _core_property_node_mxml_size;
    api->v1.search = _core_property_node_mxml_search;
    api->v1.next_result_search = _core_property_node_mxml_next_result_search;
    api->v1.child_get = _core_property_node_mxml_child_get;
    api->v1.next_sibling_get = _core_property_node_mxml_next_sibling_get;
    api->v1.void_create = _core_property_node_mxml_void_create;
    api->v1.s8_create = _core_property_node_mxml_s8_create;
    api->v1.u8_create = _core_property_node_mxml_u8_create;
    api->v1.s16_create = _core_property_node_mxml_s16_create;
    api->v1.u16_create = _core_property_node_mxml_u16_create;
    api->v1.s32_create = _core_property_node_mxml_s32_create;
    api->v1.u32_create = _core_property_node_mxml_u32_create;
    api->v1.s64_create = _core_property_node_mxml_s64_create;
    api->v1.u64_create = _core_property_node_mxml_u64_create;
    api->v1.bin_create = _core_property_node_mxml_bin_create;
    api->v1.str_create = _core_property_node_mxml_str_create;
    api->v1.ipv4_create = _core_property_node_mxml_ipv4_create;
    api->v1.float_create = _core_property_node_mxml_float_create;
    api->v1.double_create = _core_property_node_mxml_double_create;
    api->v1.attr_create = _core_property_node_mxml_attr_create;
    api->v1.bool_create = _core_property_node_mxml_bool_create;
    api->v1.s8_read = _core_property_node_mxml_s8_read;
    api->v1.u8_read = _core_property_node_mxml_u8_read;
    api->v1.s16_read = _core_property_node_mxml_s16_read;
    api->v1.u16_read = _core_property_node_mxml_u16_read;
    api->v1.s32_read = _core_property_node_mxml_s32_read;
    api->v1.u32_read = _core_property_node_mxml_u32_read;
    api->v1.s64_read = _core_property_node_mxml_s64_read;
    api->v1.u64_read = _core_property_node_mxml_u64_read;
    api->v1.bin_read = _core_property_node_mxml_bin_read;
    api->v1.str_read = _core_property_node_mxml_str_read;
    api->v1.ipv4_read = _core_property_node_mxml_ipv4_read;
    api->v1.float_read = _core_property_node_mxml_float_read;
    api->v1.double_read = _core_property_node_mxml_double_read;
    api->v1.attr_read = _core_property_node_mxml_attr_read;
    api->v1.bool_read = _core_property_node_mxml_bool_read;
    api->v1.remove = _core_property_node_mxml_remove;
    api->v1.attr_remove = _core_property_node_mxml_attr_remove;
    api->v1.copy = _core_property_node_mxml_copy;
}

void core_property_node_mxml_core_api_set()
{
    core_property_node_api_t api;

    core_property_node_mxml_core_api_get(&api);
    core_property_node_api_set(&api);
}
