#include <windows.h>

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "imports/avs.h"

#include "launcher/property.h"

#include "util/log.h"
#include "util/mem.h"

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
    avs_desc desc;
    struct property *prop;

    desc = avs_fs_open(filename, AVS_FILE_READ, AVS_FILE_FLAG_SHARE_READ);
    if (!desc) {
        log_fatal("%s: Error opening configuration file", filename);
    }

    prop = do_property_load(
        boot_property_avs_read, boot_property_avs_rewind, desc, filename);

    avs_fs_close(desc);

    return prop;
}

void boot_property_free(struct property *prop)
{
    void *buffer;

    buffer = property_desc_to_buffer(prop);
    property_destroy(prop);
    free(buffer);
}
