#include "avs-ext/error.h"
#include "avs-ext/property-internal.h"

#include "imports/avs.h"

#include "iface-core/log.h"

#include "main/core/property.h"

#include "util/mem.h"

#include "avs-ext/fs.h"

void avs_ext_property_ext_avs_file_load(
    const char *path, core_property_t **property_)
{
    avs_desc desc;
    void *buffer;
    int nbytes;
    avs_error error;

    struct property *prop;
    avs_ext_property_internal_property_t *property;

    log_assert(path);
    log_assert(property_);

    desc = avs_fs_open(path, AVS_FILE_READ, AVS_FILE_FLAG_SHARE_READ);

    if (AVS_IS_ERROR(desc)) {
        log_fatal(
            "%s: Error opening avs property file: %s",
            path,
            avs_ext_error_str(desc));
    }

    nbytes = property_read_query_memsize(avs_fs_read, desc, 0, 0);

    if (AVS_IS_ERROR(nbytes)) {
        log_fatal(
            "%s: Error querying avs property file: ",
            path,
            avs_ext_error_str(nbytes));
    }

    buffer = xmalloc(nbytes);
    prop = property_create(
        PROPERTY_FLAG_READ | PROPERTY_FLAG_WRITE | PROPERTY_FLAG_CREATE |
            PROPERTY_FLAG_APPEND,
        buffer,
        nbytes);

    avs_fs_lseek(desc, 0, AVS_SEEK_SET);

    error = property_insert_read(prop, 0, avs_fs_read, desc);

    if (AVS_IS_ERROR(error)) {
        log_fatal(
            "%s: Error reading avs property file: %s",
            path,
            avs_ext_error_str(error));
    }

    avs_fs_close(desc);

    *property_ = xmalloc(sizeof(avs_ext_property_internal_property_t));

    property = (avs_ext_property_internal_property_t *) *property_;

    property->property = prop;
}