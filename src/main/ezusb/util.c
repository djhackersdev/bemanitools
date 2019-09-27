#include <stdlib.h>
#include <string.h>

#include "ezusb/util.h"

#include "util/log.h"
#include "util/crc.h"
#include "util/fs.h"
#include "util/mem.h"

struct ezusb_firmware* ezusb_firmware_load(const char* file)
{
    void* buffer;
    size_t buffer_size;
    struct ezusb_firmware* fw;
    size_t pos;

    log_assert(file);

    pos = 0;

    if (!file_load(file, &buffer, &buffer_size, false)) {
        return false;
    }

    fw = xmalloc(sizeof(struct ezusb_firmware));

    fw->crc = ((struct ezusb_firmware*) buffer)->crc;
    fw->segment_count = ((struct ezusb_firmware*) buffer)->segment_count;

    pos += sizeof(uint16_t) * 2;

    fw->segments = xmalloc(
        sizeof(struct ezusb_firmware_segment) * fw->segment_count);

    for (uint16_t i = 0; i < fw->segment_count; i++) {
        uint16_t size = *((uint16_t*) (buffer + pos));
        pos += sizeof(uint16_t);

        uint16_t offset = *((uint16_t*) (buffer + pos));
        pos += sizeof(uint16_t);

        fw->segments[i] = xmalloc(sizeof(uint16_t) * 2 + size);

        fw->segments[i]->size = size;
        fw->segments[i]->offset = offset;
        memcpy(fw->segments[i]->data, buffer + pos, size);
        pos += size;
    }

    return fw;
}

bool ezusb_firmware_save(const char* file, struct ezusb_firmware* fw)
{
    size_t size;
    void* buffer;
    size_t pos;
    bool res;

    log_assert(file);
    log_assert(fw);

    size = 0;
    pos = 0;

    for (uint16_t i = 0; i < fw->segment_count; i++) {
        size += fw->segments[i]->size + sizeof(uint16_t) * 2;
    }

    size += sizeof(uint16_t) * 2;

    buffer = xmalloc(size);

    ((struct ezusb_firmware*) buffer)->crc = fw->crc;
    ((struct ezusb_firmware*) buffer)->segment_count = fw->segment_count;

    pos += sizeof(uint16_t) * 2;

    for (uint16_t i = 0; i < fw->segment_count; i++) {
        memcpy(buffer + pos, fw->segments[i], 
            sizeof(uint16_t) * 2 + fw->segments[i]->size);
        pos += sizeof(uint16_t) * 2 + fw->segments[i]->size;
    }

    res = file_save(file, buffer, size);
    free(buffer);

    return res;
}

struct ezusb_firmware* ezusb_firmware_alloc()
{
    struct ezusb_firmware* fw;

    fw = xmalloc(sizeof(struct ezusb_firmware));

    fw->crc = 0;
    fw->segment_count = 0;
    fw->segments = NULL;

    return fw;
}

struct ezusb_firmware_segment* ezusb_firmware_segment_alloc(uint16_t offset, 
        uint16_t size, void* data)
{
    struct ezusb_firmware_segment* seg;

    log_assert(size > 0);
    log_assert(data);

    seg = xmalloc(sizeof(uint16_t) * 2 + size);

    seg->offset = offset;
    seg->size = size;
    memcpy(seg->data, data, size);

    return seg;
}

void ezusb_firmware_add_segment(struct ezusb_firmware* fw, 
        struct ezusb_firmware_segment* segment)
{
    struct ezusb_firmware_segment** tmp;

    log_assert(fw);
    log_assert(segment);

    fw->segment_count++;

    tmp = xrealloc(fw->segments, 
        sizeof(struct ezusb_firmware_segment*) * fw->segment_count);

    if (tmp != NULL) {
        fw->segments = tmp;
    }

    fw->segments[fw->segment_count - 1] = segment;
}

uint16_t ezusb_firmware_crc(struct ezusb_firmware* fw)
{
    uint16_t crc;

    log_assert(fw);

    crc = 0;

    for (uint16_t i = 0; i < fw->segment_count; i++) {
        crc = crc16(fw->segments[i], 
            sizeof(uint16_t) * 2 + fw->segments[i]->size, crc);
    }

    return crc;
}

void ezusb_firmware_free(struct ezusb_firmware* fw)
{
    log_assert(fw);

    for (uint16_t i = 0; i < fw->segment_count; i++) {
        free(fw->segments[i]);
    }

    free(fw);
}