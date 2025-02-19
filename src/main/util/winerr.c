#include <windows.h>

#include "util/mem.h"
#include "util/str.h"
#include "util/winerr.h"

char* util_winerr_format_last_error_code()
{
    return util_winerr_format_error_code(GetLastError());
}

char* util_winerr_format_error_code(int err_code)
{
    // Max size according to docs
    char buffer[64 * 1024];
    char *buffer_dyn;
    DWORD format_flags;
    DWORD language_id;
    size_t len;

    format_flags = FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS;
    language_id = MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US);

    len = FormatMessageA(format_flags, NULL, err_code, language_id, buffer, sizeof(buffer), NULL);

    if (len > 0) {
        // Have this on the heap by don't use the dynamic allocation provided by
        // the function because it's on a different heap and can't be free'd
        // with free()
        buffer_dyn = xmalloc(len);

        str_cpy(buffer_dyn, len, buffer);

        return buffer_dyn;
    } else {
        return NULL;
    }
}