#include <shlobj.h>
#include <windows.h>

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "core/log.h"

#include "util/defs.h"
#include "util/fs.h"
#include "util/mem.h"
#include "util/str.h"

bool file_load(
    const char *filename, void **out_bytes, size_t *out_nbytes, bool text_mode)
{
    FILE *f;
    void *bytes;
    size_t nbytes;
    int result;

    log_assert(out_bytes != NULL);
    *out_bytes = NULL;

    f = fopen(filename, "rb");

    if (f == NULL) {
        log_warning("%s: Error opening file: %s", filename, strerror(errno));

        goto open_fail;
    }

    fseek(f, 0, SEEK_END);
    nbytes = ftell(f);
    fseek(f, 0, SEEK_SET);

    /* Add for null terminator */
    if (text_mode) {
        nbytes++;
    }

    bytes = malloc(nbytes);

    if (bytes == NULL) {
        log_warning("%s: malloc(%u) failed", filename, (unsigned int) nbytes);

        goto malloc_fail;
    }

    if (text_mode) {
        result = fread(bytes, nbytes - 1, 1, f);
    } else {
        result = fread(bytes, nbytes, 1, f);
    }

    if (result != 1) {
        log_warning("%s: Error reading file: %s", filename, strerror(errno));

        goto read_fail;
    }

    *out_bytes = bytes;

    if (out_nbytes != NULL) {
        *out_nbytes = nbytes;
    }

    fclose(f);

    /* Add null terminator */
    if (text_mode) {
        ((char *) bytes)[nbytes - 1] = '\0';
    }

    log_misc(
        "File loaded %s, size %lu", filename, (long unsigned int) *out_nbytes);

    return true;

read_fail:
    free(bytes);

malloc_fail:
    fclose(f);

open_fail:
    return false;
}

bool file_save(const char *filename, const void *bytes, size_t nbytes)
{
    FILE *f;
    int result;

    f = fopen(filename, "wb");

    if (f == NULL) {
        log_warning("%s: Error creating file: %s", filename, strerror(errno));

        goto open_fail;
    }

    result = fwrite(bytes, nbytes, 1, f);

    if (result != 1) {
        log_warning("%s: Error writing file: %s", filename, strerror(errno));

        goto write_fail;
    }

    fclose(f);

    return true;

write_fail:
    fclose(f);

open_fail:
    return false;
}

bool path_exists(const char *path)
{
    DWORD attrib = GetFileAttributes(path);

    return attrib != INVALID_FILE_ATTRIBUTES;
}

bool path_exists_wstr(const wchar_t *path)
{
    DWORD attrib = GetFileAttributesW(path);

    return attrib != INVALID_FILE_ATTRIBUTES;
}

FILE *fopen_appdata(const char *vendor, const char *filename, const char *mode)
{
    char root[MAX_PATH];
    char path[MAX_PATH];
    HRESULT hr;
    FILE *f;

    hr = SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, SHGFP_TYPE_CURRENT, root);

    if (FAILED(hr)) {
        log_warning("Failed to get AppData folder: hr=%08x", (unsigned int) hr);

        return NULL;
    }

    str_format(path, sizeof(path), "%s\\%s", root, vendor);

    if (!CreateDirectory(path, NULL) &&
        GetLastError() != ERROR_ALREADY_EXISTS) {
        log_warning(
            "Failed to create %s: %08x", path, (unsigned int) GetLastError());

        return NULL;
    }

    str_format(path, sizeof(path), "%s\\%s\\%s", root, vendor, filename);
    f = fopen(path, mode);

    if (f == NULL) {
        log_warning(
            "fopen(\"%s\", \"%s\") failed: %s", path, mode, strerror(errno));
    }

    return f;
}

bool path_mkdir(const char *path)
{
    bool success;
    char buf[MAX_PATH];
    char *pos;

    str_cpy(buf, lengthof(buf), path);

    for (pos = path_next_element(buf); pos; pos = path_next_element(pos + 1)) {
        char c = *pos;
        *pos = '\0';

        if (strlen(buf) != 2 || (strlen(buf) >= 2 && buf[1] != ':')) {
            success = CreateDirectory(buf, NULL);

            if (!success && GetLastError() != ERROR_ALREADY_EXISTS) {
                log_warning(
                    "%s: Cannot create directory: %#x",
                    buf,
                    (unsigned int) GetLastError());

                return false;
            }
        }

        *pos = c;
    }

    success = CreateDirectory(buf, NULL);

    if (!success && GetLastError() != ERROR_ALREADY_EXISTS) {
        log_warning(
            "%s: Cannot create directory: %#x",
            buf,
            (unsigned int) GetLastError());

        return false;
    }

    return true;
}

char *path_next_element(char *path)
{
    char *c;

    for (c = path; *c; c++) {
        if (*c == '/' || *c == '\\') {
            return c;
        }
    }

    return NULL;
}

bool read_str(FILE *f, char **str)
{
    uint32_t len;
    char *buf;

    if (fread(&len, sizeof(len), 1, f) != 1) {
        goto len_fail;
    }

    buf = malloc(len + 1);

    if (buf == NULL) {
        goto alloc_fail;
    }

    if (len > 0 && fread(buf, len, 1, f) != 1) {
        goto body_fail;
    }

    buf[len] = '\0';
    *str = buf;

    return true;

body_fail:
    free(buf);

alloc_fail:
len_fail:
    return false;
}

void write_str(FILE *f, const char *str)
{
    uint32_t len;

    len = strlen(str);

    fwrite(&len, sizeof(len), 1, f);
    fwrite(str, len, 1, f);
}
