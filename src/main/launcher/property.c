#include <windows.h>

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "imports/avs.h"

#include "launcher/property.h"

#include "util/log.h"
#include "util/mem.h"

static int boot_property_fread(uint32_t context, void *bytes, size_t nbytes)
{
    FILE *f;

    f = TlsGetValue(context);

    return fread(bytes, 1, nbytes, f);
}

struct property *boot_property_load(const char *filename)
{
    struct property *prop;
    void *buffer;
    int nbytes;
    FILE *f;
    uint32_t f_keyhole;

    /* AVS callbacks are only given a 32-bit context parameter, even in 64-bit
       builds of AVS. We allocate a 32-bit TLS key and pass the context in this
       manner instead. Inefficient, but it works. */

    f = fopen(filename, "r");

    f_keyhole = TlsAlloc();
    TlsSetValue(f_keyhole, f);

    if (f == NULL) {
        log_fatal("%s: Error opening configuration file", filename);
    }

    nbytes = property_read_query_memsize(boot_property_fread, f_keyhole, 0, 0);

    if (nbytes < 0) {
        log_fatal("%s: Error parsing configuration file", filename);
    }

    buffer = xmalloc(nbytes);
    prop = property_create(
                    PROPERTY_FLAG_READ |
                    PROPERTY_FLAG_WRITE |
                    PROPERTY_FLAG_CREATE |
                    PROPERTY_FLAG_APPEND,
            buffer,
            nbytes);
    rewind(f);

    if (!property_insert_read(prop, 0, boot_property_fread, f_keyhole)) {
        log_fatal("%s: Error parsing configuration file", filename);
    }

    TlsFree(f_keyhole);

    fclose(f);

    return prop;
}

void boot_property_free(struct property *prop)
{
    void *buffer;

    buffer = property_desc_to_buffer(prop);
    property_destroy(prop);
    free(buffer);
}

