#define LOG_MODULE "core-property-mxml"

#include <windows.h>

#include <errno.h>
#include <stdio.h>

#include <mxml/mxml.h>

#include "core/property-mxml-internal.h"
#include "core/property-node-mxml.h"
#include "core/property-mxml.h"

#include "iface-core/log.h"

#include "util/fs.h"
#include "util/mem.h"

static core_property_result_t
_core_property_mxml_create(size_t size, core_property_t **property_)
{
    mxml_node_t *node;
    core_property_mxml_internal_property_t *property;

    // There is no need to use the size parameter here as everything's allocated
    // dynamically on the heap and no initial fixed size is needed
    node = mxmlNewXML(NULL);
    
    if (!node) {
        return CORE_PROPERTY_RESULT_ERROR_ALLOC;
    }

    property = xmalloc(sizeof(core_property_mxml_internal_property_t));

    property->document = node;

    *property_ = (core_property_t *) property;

    return CORE_PROPERTY_RESULT_SUCCESS;
}

static core_property_result_t
_core_property_mxml_file_load(const char *path, core_property_t **property_)
{
    mxml_node_t *node;
    core_property_mxml_internal_property_t *property;

    // The return value NULL of the loading function
    // doesn't distinguish file not found and loading error =|
    if (!path_exists(path)) {
        return CORE_PROPERTY_RESULT_NOT_FOUND;
    }

    node = mxmlLoadFilename(NULL, NULL, path);
    
    if (!node) {
        return CORE_PROPERTY_RESULT_ERROR_READ;
    }

    property = xmalloc(sizeof(core_property_mxml_internal_property_t));

    property->document = node;

    *property_ = (core_property_t *) property;

    return CORE_PROPERTY_RESULT_SUCCESS;
}

static core_property_result_t
_core_property_mxml_str_load(const char *str, core_property_t **property_)
{
    mxml_node_t *node;
    core_property_mxml_internal_property_t *property;

    node = mxmlLoadString(NULL, NULL, str);
    
    if (!node) {
        return CORE_PROPERTY_RESULT_ERROR_READ;
    }

    property = xmalloc(sizeof(core_property_mxml_internal_property_t));

    property->document = node;

    *property_ = (core_property_t *) property;

    return CORE_PROPERTY_RESULT_SUCCESS;
}

static core_property_result_t
_core_property_mxml_size(const core_property_t *property_, size_t *size)
{
    *size = CORE_PROPERTY_MXML_INTERNAL_FIXED_SIZE_DUMMY;

    return CORE_PROPERTY_RESULT_SUCCESS;
}

static core_property_result_t _core_property_mxml_clone(
    const core_property_t *property_, core_property_t **property_cloned_)
{
    const core_property_mxml_internal_property_t *property;
    core_property_mxml_internal_property_t **property_cloned;

    char *str_copy;
    mxml_node_t *node_cloned;

    property = (const core_property_mxml_internal_property_t*) property_;
    property_cloned = (core_property_mxml_internal_property_t**) property_cloned_;

    // Ensure actual cloning by storing and loading this
    // Just "adding it" to the other tree with mxml creates a reference only

    str_copy = mxmlSaveAllocString(property->document, NULL);

    if (str_copy == NULL) {
        return CORE_PROPERTY_RESULT_ERROR_READ;
    }

    node_cloned = mxmlLoadString(NULL, NULL, str_copy);

    if (node_cloned == NULL) {
        return CORE_PROPERTY_RESULT_ERROR_WRITE;
    }

    *property_cloned = xmalloc(sizeof(core_property_mxml_internal_property_t));

    (*property_cloned)->document = node_cloned;

    return CORE_PROPERTY_RESULT_SUCCESS;
}

static core_property_result_t _core_property_mxml_root_node_get(
    const core_property_t *property_, core_property_node_t *node_)
{
    core_property_mxml_internal_property_t *property;
    core_property_mxml_internal_property_node_t *node;
    mxml_node_t *mxml_node;
    mxml_type_t type;

    property = (core_property_mxml_internal_property_t *) property_;
    node = (core_property_mxml_internal_property_node_t *) node_;

    memset(node, 0, sizeof(core_property_mxml_internal_property_node_t));

    // Walk the nodes as the loaded xml node is the document root
    // which can be the xml directive, or if missing, the first root
    // element
    for (mxml_node = property->document;
        mxml_node != NULL;
        mxml_node = mxmlWalkNext(mxml_node, property->document, MXML_DESCEND_FIRST))
    {
        type = mxmlGetType(mxml_node);
        // Consider the first element to be found the root element of the
        // document. Having multiple roots isn't a valid XML document
        // anyway
        if (type == MXML_TYPE_ELEMENT) {
            break;
        }
    }

    if (mxml_node == NULL) {
        return CORE_PROPERTY_RESULT_NOT_FOUND;
    }
 
    node->property = property;
    // No difference for property vs. property_node
    node->node = mxml_node;

    return CORE_PROPERTY_RESULT_SUCCESS;
}

static core_property_result_t _core_property_mxml_other_node_insert(
    core_property_t *property_, const core_property_node_t *node_)
{
    core_property_mxml_internal_property_t *property;
    core_property_mxml_internal_property_node_t *node;

    char *str_copy;
    mxml_node_t *node_cloned;

    property = (core_property_mxml_internal_property_t *) property_;
    node = (core_property_mxml_internal_property_node_t *) node_;

    // Ensure actual cloning by storing and loading this
    // Just "adding it" to the other tree with mxml creates a reference only

    str_copy = mxmlSaveAllocString(node->node, NULL);

    if (str_copy == NULL) {
        return CORE_PROPERTY_RESULT_ERROR_READ;
    }

    node_cloned = mxmlLoadString(NULL, NULL, str_copy);

    if (node_cloned == NULL) {
        return CORE_PROPERTY_RESULT_ERROR_WRITE;
    }

    // Hook the new node into the existing tree

    mxmlAdd(property->document, MXML_ADD_AFTER, NULL, node_cloned);

    return CORE_PROPERTY_NODE_RESULT_SUCCESS;
}

static void _core_property_mxml_free(core_property_t **property_)
{
    core_property_mxml_internal_property_t *property;

    property = (core_property_mxml_internal_property_t *) (*property_);

    mxmlDelete(property->document);

    free(*property_);
    *property_ = NULL;
}

static void _core_property_mxml_core_api_get(core_property_api_t *api)
{
    log_assert(api);

    api->version = 1;

    api->v1.create = _core_property_mxml_create;
    api->v1.file_load = _core_property_mxml_file_load;
    api->v1.str_load = _core_property_mxml_str_load;
    api->v1.size = _core_property_mxml_size;
    api->v1.clone = _core_property_mxml_clone;
    api->v1.root_node_get = _core_property_mxml_root_node_get;
    api->v1.other_node_insert = _core_property_mxml_other_node_insert;
    api->v1.free = _core_property_mxml_free;
}

void core_property_mxml_core_api_set()
{
    core_property_api_t api;

    _core_property_mxml_core_api_get(&api);
    core_property_api_set(&api);
}
