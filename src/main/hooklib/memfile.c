#include "hooklib/memfile.h"

#include <stdbool.h>
#include <windows.h>

#include "hook/hr.h"
#include "hook/iohook.h"
#include "hook/table.h"

#include "util/array.h"
#include "util/log.h"
#include "util/str.h"

struct file_entry {
    char *path;
    wchar_t *wpath;
    HANDLE fd;
    int64_t pos;

    const void *data;
    uint32_t sz;
    enum memfile_hook_path_mode path_mode;
};

static struct array hooked_files;
static CRITICAL_SECTION hooked_files_cs;


BOOL my_GetFileInformationByHandle(
  HANDLE                       hFile,
  LPBY_HANDLE_FILE_INFORMATION lpFileInformation
);

BOOL (*real_GetFileInformationByHandle)(
  HANDLE                       hFile,
  LPBY_HANDLE_FILE_INFORMATION lpFileInformation
);

static const struct hook_symbol memfile_hook_kernel32_syms[] = {
    {.name = "GetFileInformationByHandle",
     .patch = my_GetFileInformationByHandle,
     .link = (void **) &real_GetFileInformationByHandle},
};


void memfile_hook_init(void)
{
    array_init(&hooked_files);
    InitializeCriticalSection(&hooked_files_cs);

    hook_table_apply(
        NULL,
        "Kernel32.dll",
        memfile_hook_kernel32_syms,
        lengthof(memfile_hook_kernel32_syms));

    log_info("Initialized");
}

void memfile_hook_fini(void)
{
    EnterCriticalSection(&hooked_files_cs);
    struct file_entry *entry;

    for (size_t i = 0; i < hooked_files.nitems; i++) {
        entry = array_item(struct file_entry, &hooked_files, i);

        free(entry->wpath);
        free(entry->path);

        if (entry->fd != NULL) {
            CloseHandle(entry->fd);
        }
    }

    array_fini(&hooked_files);
    LeaveCriticalSection(&hooked_files_cs);

    DeleteCriticalSection(&hooked_files_cs);

    log_info("Finished");
}

void memfile_hook_add_fd(
    const char *path, enum memfile_hook_path_mode path_mode, const void *data, uint32_t sz)
{
    log_assert(path != NULL);

    HRESULT hr;
    struct file_entry *entry = NULL;

    EnterCriticalSection(&hooked_files_cs);
    entry = array_append(struct file_entry, &hooked_files);

    entry->path = strdup(path);
    entry->wpath = str_widen(path);

    hr = iohook_open_nul_fd(&entry->fd);

    if (hr != S_OK) {
        LeaveCriticalSection(&hooked_files_cs);
        log_fatal("Opening nul fd failed: %08lx", hr);
    }

    entry->pos = 0;

    entry->data = data;
    entry->sz = sz;
    entry->path_mode = path_mode;

    LeaveCriticalSection(&hooked_files_cs);

    log_misc("memfile_hook_add_fd: path %s, mode %d, data size %d", path, path_mode, sz);
}

static struct file_entry *memfile_hook_match_irp(const struct irp *irp)
{
    struct file_entry *entry;
    log_assert(irp != NULL);

    if (irp->op == IRP_OP_OPEN) {
        for (size_t i = 0; i < hooked_files.nitems; i++) {
            entry = array_item(struct file_entry, &hooked_files, i);

            if (entry->path_mode == ABSOLUTE_MATCH) {
                if (wstr_eq(entry->wpath, irp->open_filename)) {
                    log_misc("Memfile Open: Absolute matched: %s", entry->path);
                    return entry;
                }
            } else if (entry->path_mode == ENDING_MATCH) {
                if (wstr_ends_with(irp->open_filename, entry->wpath)) {
                    log_misc("Memfile Open: Ending matched: %s", entry->path);
                    return entry;
                }
            }
        }
    } else {
        for (size_t i = 0; i < hooked_files.nitems; i++) {
            entry = array_item(struct file_entry, &hooked_files, i);

            if (entry->fd == irp->fd) {
                return entry;
            }
        }
    }

    return NULL;
}

static HRESULT memfile_hook_irp_read(struct file_entry *entry, struct irp *irp)
{
    log_assert(entry != NULL);
    log_assert(irp != NULL);

    size_t nread = min(entry->sz - entry->pos, irp->read.nbytes);

    if (nread > 0) {
        memcpy(irp->read.bytes + irp->read.pos, (uint8_t*)entry->data + entry->pos, nread);

        entry->pos += nread;
        irp->read.pos += nread;
    }

    return S_OK;
}

static HRESULT memfile_hook_irp_seek(struct file_entry *entry, struct irp *irp)
{
    log_assert(entry != NULL);
    log_assert(irp != NULL);

    switch (irp->seek_origin) {
        case FILE_BEGIN:
            entry->pos = irp->seek_offset;
            break;
        case FILE_CURRENT:
            entry->pos += irp->seek_offset;
            break;
        case FILE_END:
            entry->pos = entry->sz + irp->seek_offset;
            break;
        default:
            log_fatal("Invalid seek origin");
    }

    irp->seek_pos = entry->pos;
    return S_OK;
}

static HRESULT
memfile_hook_irp_handler(struct file_entry *entry, struct irp *irp)
{
    log_assert(entry != NULL);
    log_assert(irp != NULL);

    switch (irp->op) {
        case IRP_OP_OPEN:
            irp->fd = entry->fd;
            entry->pos = 0;
            return S_OK;

        case IRP_OP_CLOSE:
            return S_OK;

        case IRP_OP_READ:
            return memfile_hook_irp_read(entry, irp);

        case IRP_OP_WRITE:
            log_warning("write attempted on memfile, unsupported");
            return S_OK;

        case IRP_OP_IOCTL:
            log_warning("ioctl attempted on memfile, unsupported");
            return S_OK;

        case IRP_OP_FSYNC:
            log_warning("fsync attempted on memfile, unsupported");
            return S_OK;

        case IRP_OP_SEEK:
            return memfile_hook_irp_seek(entry, irp);

        default:
            return E_NOTIMPL;
    }
}

HRESULT memfile_hook_dispatch_irp(struct irp *irp)
{
    log_assert(irp != NULL);

    HRESULT hr;

    EnterCriticalSection(&hooked_files_cs);

    struct file_entry *entry = memfile_hook_match_irp(irp);

    if (!entry) {
        LeaveCriticalSection(&hooked_files_cs);
        return iohook_invoke_next(irp);
    }

    hr = memfile_hook_irp_handler(entry, irp);

    LeaveCriticalSection(&hooked_files_cs);
    return hr;
}

// this isn't a standard iohook IRP atm.
BOOL my_GetFileInformationByHandle(
  HANDLE                       hFile,
  LPBY_HANDLE_FILE_INFORMATION lpFileInformation
)
{
    struct file_entry *entry;
    for (size_t i = 0; i < hooked_files.nitems; i++) {
        entry = array_item(struct file_entry, &hooked_files, i);

        if (entry->fd == hFile) {
            lpFileInformation->dwFileAttributes = FILE_ATTRIBUTE_READONLY;
            lpFileInformation->nFileSizeHigh = 0;
            lpFileInformation->nFileSizeLow = entry->sz;
            return true;
        }
    }

    return real_GetFileInformationByHandle(hFile, lpFileInformation);
}
