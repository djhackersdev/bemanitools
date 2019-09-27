#define LOG_MODULE "chart-patch"

#include <math.h>
#include <string.h>
#include <windows.h>

#include "hook/iohook.h"

#include "util/crc.h"
#include "util/defs.h"
#include "util/fs.h"
#include "util/log.h"
#include "util/mem.h"
#include "util/str.h"

struct chart_patch_file {
    HANDLE fd;
    struct const_iobuf state;
    bool active;
};

struct chart_toc_entry {
    uint32_t offset;
    uint32_t length;
} __attribute__((__packed__));

struct chart_event {
    uint32_t time;
    uint8_t type;
    uint8_t data1;
    uint16_t data2;
} __attribute__((__packed__));

struct ospreybin_entry {
    char filename[32];
    uint32_t checksum1;
    uint32_t checksum2;
    uint32_t offset_in_file;
    uint32_t checksum_length;
    uint32_t filesize;
} __attribute__((__packed__));

struct ospreybin {
    /* IIDX */
    char magic[4];
    /* e.g. 17 for Sirius */
    uint32_t version;
    /* seems to be unused */
    uint32_t num_entries;
    uint32_t checksum_type;
};

static bool chart_patch_initted;

static double chart_patch_hz;
static double chart_patch_timebase;

static bool chart_patch_file_detour;
static char chart_patch_file_1_name[6];
static struct chart_patch_file chart_patch_file_1;
static struct chart_patch_file chart_patch_file_checksum;
static CRITICAL_SECTION chart_patch_lock;

static bool chart_patch_file_load(struct irp* irp_open, 
        struct chart_patch_file *file, const wchar_t* path)
{
    struct irp irp;
    void *bytes;
    size_t nbytes;
    HANDLE fd;
    HRESULT hr;
    char* tmp;

    /* Reset file to ensure consistent not-available state */
    memset(file, 0, sizeof(struct chart_patch_file));

    memset(&irp, 0, sizeof(irp));
    irp.op = IRP_OP_OPEN;
    irp.next_handler = irp_open->next_handler;
    irp.open_filename = path;
    irp.open_access = GENERIC_READ;
    irp.open_creation = OPEN_EXISTING;

    hr = irp_invoke_next(&irp);

    if (FAILED(hr)) {
        return false;
    }

    fd = irp.fd;

    memset(&irp, 0, sizeof(irp));
    irp.op = IRP_OP_SEEK;
    irp.fd = fd;
    irp.seek_origin = FILE_END;
    irp.seek_offset = 0;

    hr = irp_invoke_next(&irp);

    if (FAILED(hr)) {
        log_fatal("Seeking end failed");
    }

    nbytes = (size_t) irp.seek_pos;
    bytes = xmalloc(nbytes);

    memset(&irp, 0, sizeof(irp));
    irp.op = IRP_OP_SEEK;
    irp.fd = fd;
    irp.seek_origin = FILE_BEGIN;
    irp.seek_offset = 0;

    hr = irp_invoke_next(&irp);

    if (FAILED(hr)) {
        log_fatal("Seeking begin failed");
    }

    memset(&irp, 0, sizeof(irp));
    irp.op = IRP_OP_READ;
    irp.fd = fd;
    irp.read.bytes = bytes;
    irp.read.nbytes = nbytes;
    irp.read.pos = 0;

    hr = irp_invoke_next(&irp);

    if (FAILED(hr)) {
        log_fatal("Reading failed");
    }

    if (irp.read.pos != nbytes) {
        hr = HRESULT_FROM_WIN32(ERROR_HANDLE_EOF);

        log_fatal("Reading entire file failed");
    }

    memset(&irp, 0, sizeof(irp));
    irp.op = IRP_OP_CLOSE;
    irp.fd = fd;

    hr = irp_invoke_next(&irp);

    if (FAILED(hr)) {
        log_fatal("Closing failed");
    }

    file->fd = iohook_open_dummy_fd();
    memset(&file->state, 0, sizeof(struct const_iobuf));
    file->state.bytes = bytes;
    file->state.nbytes = nbytes;
    file->active = false;

    wstr_narrow(path, &tmp);
    log_misc("Loading %s successful, fd %p", tmp, file->fd);
    free(tmp);

    return true;
}

static bool chart_patch_is_chart_id(const wchar_t* str)
{
    for (int i = 0; i < 4; i++) {
        if (!(str[i] >= L'0' && str[i] <= L'9')) {
            return false;
        }
    }

    return true;
}

static void chart_patch_execute_patch_1()
{
    double factor;
    struct chart_toc_entry *toc;
    struct chart_event *event;
    size_t chart_offset;
    size_t i;

    log_assert(chart_patch_file_1.fd != NULL);
    log_assert(chart_patch_hz > 0);
    log_assert(chart_patch_timebase > 0);

    factor = chart_patch_hz / chart_patch_timebase;
    toc = (struct chart_toc_entry *) chart_patch_file_1.state.bytes;

    log_misc("Patching chart: orig %f, target %f, factor %f", 
        chart_patch_timebase, chart_patch_hz, factor);

    for (i = 0 ; i < 12 ; i++) {
        if (toc[i].offset == 0) {
            continue; /* No such chart */
        }

        for (   chart_offset  = toc[i].offset ;
                chart_offset  < toc[i].offset + toc[i].length ;
                chart_offset += sizeof(struct chart_event)) {
            event = (struct chart_event *) 
                &chart_patch_file_1.state.bytes[chart_offset];

            /* Don't clobber the end-of-chart marker */

            if (event->time != 0x7FFFFFFF) {
                event->time  = (uint32_t) round(event->time * factor);
            }
        }
    }
}

static void chart_patch_execute_patch_checksum()
{
    const struct ospreybin *header;
    struct ospreybin_entry *entries;
    struct ospreybin_entry *entry;
    const void *span_start;
    size_t span_len;
    uint32_t checksum;
    size_t i;

    header = (struct ospreybin *) &chart_patch_file_checksum.state.bytes[0];
    entries = (struct ospreybin_entry *) 
        &chart_patch_file_checksum.state.bytes[sizeof(*header)];

    for (i = 0 ; i < header->num_entries ; i++) {
        entry = &entries[i];

        if (!str_eq(chart_patch_file_1_name, entry->filename)) {
            continue;
        }

        span_start = 
            &chart_patch_file_1.state.bytes[entry->offset_in_file];
        span_len = entry->checksum_length;
        checksum = crc32(span_start, span_len, 0);

        entry->checksum1 = checksum;
        entry->checksum2 = checksum;

        return;
    }

    log_warning("Could not find checksum row for %s", chart_patch_file_1_name);
}

static bool chart_patch_file_trap(struct irp* irp)
{
    wchar_t buffer_1_path[1024];
    wchar_t buffer_checksum_path[1024];
    wchar_t chart_id[5];
    wchar_t chart_file[7];
    bool is_1_file;
    bool is_checksum_file;
    char* tmp;

    /* This has to trap three different scenarios:
       1. open .1 chart file only
       2. open .1 chart file first, then osprey.bin
       3. open osprey.bin first, then .1 chart file
       In the end, we have to serve either just the .1 file or both the .1
       and checksum file from memory until the application closes one or both
       files */
    if (swscanf(irp->open_filename, L"..\\data\\sd_data%*[\\/]%c%c%c%c%*[\\/]", 
            &chart_id[0], &chart_id[1], &chart_id[2], &chart_id[3]) == 4 &&
            chart_patch_is_chart_id(chart_id)) {
        chart_id[4] = '\0';

        wstr_narrow(irp->open_filename, &tmp);
        log_misc("Trapping %s...", tmp);
        free(tmp);

        memcpy(chart_file, chart_id, sizeof(chart_id));
        chart_file[4] = L'.';
        chart_file[5] = L'1';
        chart_file[6] = L'\0';

        is_1_file = wstr_ends_with(irp->open_filename, chart_file);
        is_checksum_file = wstr_ends_with(irp->open_filename, L"osprey.bin");

        /* Block a second open call to either .1 or checksum file if detouring
           is already active */
        if (chart_patch_file_detour) {
            if (is_1_file) {
                irp->fd = chart_patch_file_1.fd;
                chart_patch_file_1.active = true;
            } else if (is_checksum_file) {
                irp->fd = chart_patch_file_checksum.fd;
                chart_patch_file_checksum.active = true;
            } else {
                /* other file, like *.2dx */
                return false;
            }

            log_misc("Data already prepared");

            return true;
        }

        if (is_1_file || is_checksum_file) {
            wstr_narrow(chart_id, &tmp);
            log_misc("Preparing in-memory chart %s...", tmp);
            free(tmp);

            wsprintfW(buffer_1_path, L"..\\data\\sd_data\\\\%s\\%s.1", chart_id,
                chart_id);
            wsprintfW(buffer_checksum_path,
                L"..\\data\\sd_data\\%s\\osprey.bin", chart_id);

            if (!chart_patch_file_load(irp, &chart_patch_file_1, 
                    buffer_1_path)) {
                wstr_narrow(buffer_1_path, &tmp);
                log_fatal("Cannot load .1 file %s", tmp);
                free(tmp);
            }

            chart_patch_execute_patch_1();

            wstr_narrow(chart_file, &tmp);
            memcpy(chart_patch_file_1_name, tmp, 
                sizeof(chart_patch_file_1_name));
            free(tmp);

            /* Optional on some games */
            if (chart_patch_file_load(irp, &chart_patch_file_checksum, 
                    buffer_checksum_path)) {
                chart_patch_execute_patch_checksum();
            }

            /* Return dummy handle */
            if (is_1_file) {
                irp->fd = chart_patch_file_1.fd;
                chart_patch_file_1.active = true;
            } else if (is_checksum_file) {
                irp->fd = chart_patch_file_checksum.fd;
                chart_patch_file_checksum.active = true;
            } else {
                log_fatal("Illegal state 2");
            }

            wstr_narrow(chart_id, &tmp);
            log_misc("Patch for %s prepared in memory", tmp);
            free(tmp);

            chart_patch_file_detour = true;

            return true;
        }
    }

    return false;
}

static HRESULT chart_patch_file_close_irp(
        struct chart_patch_file *file,
        struct irp *irp_close)
{
    HRESULT hr;
    struct irp irp;

    file->active = false;

    // log_misc("chart_patch_file_close_irp %d %p %d %p", 
    //     chart_patch_file_1.active,
    //     chart_patch_file_1.fd,
    //     chart_patch_file_checksum.active,
    //     chart_patch_file_checksum.fd);

    /* Check if both files are closed and free resources */
    if (!chart_patch_file_1.active && !chart_patch_file_checksum.active) {
        log_misc("Closing fd %p and cleaning up", file->fd);

        if (chart_patch_file_1.fd != NULL) {
            memcpy(&irp, irp_close, sizeof(struct irp));
            irp.fd = chart_patch_file_1.fd;

            hr = irp_invoke_next(&irp);
            
            if (hr != S_OK) {
                log_warning("Closing dummy .1 file handle failed");
            }
                        
            free((void*) chart_patch_file_1.state.bytes);
            memset(&chart_patch_file_1, 0, sizeof(struct chart_patch_file));
        }

        if (chart_patch_file_checksum.fd != NULL) {
            memcpy(&irp, irp_close, sizeof(struct irp));
            irp.fd = chart_patch_file_checksum.fd;

            hr = irp_invoke_next(&irp);

            if (hr != S_OK) {
                log_warning("Closing dummy checksum file handle failed");
            }

            free((void*) chart_patch_file_checksum.state.bytes);
            memset(&chart_patch_file_checksum, 0, 
                sizeof(struct chart_patch_file));
        }

        chart_patch_file_detour = false;
    } else {
        log_misc("Closing fd %p", file->fd);
    }

    return S_OK;
}

static HRESULT chart_patch_file_read(
        struct chart_patch_file *file,
        struct irp *irp)
{
    // log_misc("chart_patch_file_read %p", file->fd);

    iobuf_move(&irp->read, &file->state);

    return S_OK;
}

static HRESULT chart_patch_file_seek(
        struct chart_patch_file *file,
        struct irp *irp)
{
    ssize_t pos;

    switch (irp->seek_origin) {
    case FILE_BEGIN:
        pos = irp->seek_offset;

        break;

    case FILE_CURRENT:
        pos = irp->seek_offset + file->state.pos;

        break;

    case FILE_END:
        pos = irp->seek_offset + file->state.nbytes;

        break;

    default:
        return E_INVALIDARG;
    }

    file->state.pos = max(0, min(pos, file->state.nbytes));
    irp->seek_pos = file->state.pos;

    return S_OK;
}

static HRESULT chart_patch_file_dispatch_irp(
        struct chart_patch_file *file,
        struct irp *irp)
{
    // log_misc("chart_patch_file_dispatch_irp %p", file->fd);

    switch (irp->op) {
    case IRP_OP_CLOSE:  return chart_patch_file_close_irp(file, irp);
    case IRP_OP_READ:   return chart_patch_file_read(file, irp);
    case IRP_OP_WRITE:  return HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED);
    case IRP_OP_SEEK:   return chart_patch_file_seek(file, irp);
    default:            return E_NOTIMPL;
    }
}

void iidxhook_util_chart_patch_init(double orig_timebase)
{
    log_assert(!chart_patch_initted);

    chart_patch_file_detour = false;

    chart_patch_timebase = orig_timebase;

    chart_patch_file_1.fd = INVALID_HANDLE_VALUE;
    chart_patch_file_checksum.fd = INVALID_HANDLE_VALUE;

    InitializeCriticalSection(&chart_patch_lock);

    chart_patch_initted = true;

    log_info("Initialized with orig_timebase %f", orig_timebase);
}

void iidxhook_util_chart_patch_fini(void)
{
    log_assert(chart_patch_initted);

    DeleteCriticalSection(&chart_patch_lock);
    chart_patch_initted = false;

    log_info("Fini");
}

void iidxhook_util_chart_patch_set_refresh_rate(double hz)
{
    chart_patch_hz = hz;

    log_info("Set target refresh rate %f", chart_patch_hz);
}

HRESULT iidxhook_util_chart_patch_dispatch_irp(struct irp *irp)
{
    HRESULT hr;

    if (chart_patch_initted) {
        EnterCriticalSection(&chart_patch_lock);

        hr = S_FALSE;

        if (irp->op == IRP_OP_OPEN) {
            if (chart_patch_file_trap(irp)) {
                hr = S_OK;
            } else {
                hr = irp_invoke_next(irp);
            }
        } else {
            if (irp->fd != INVALID_HANDLE_VALUE && irp->fd != NULL) {
                if (irp->fd == chart_patch_file_1.fd) {
                    hr = chart_patch_file_dispatch_irp(&chart_patch_file_1, 
                        irp);
                } else if (irp->fd == chart_patch_file_checksum.fd) {
                    hr = chart_patch_file_dispatch_irp(
                        &chart_patch_file_checksum, irp);
                } else {
                    hr = irp_invoke_next(irp);
                }
            }
        }

        LeaveCriticalSection(&chart_patch_lock);

        return hr;
    } else {
        return irp_invoke_next(irp);
    }
}