#include <string.h>

#include "security/mcode.h"

#include "util/log.h"
#include "util/mem.h"

const struct security_mcode security_mcode_eamuse = {
    .header = '@',
    .unkn = '@',
    .game = "@@@",
    .region = '@',
    .cabinet = '@',
    .revision = '@',
};

bool security_mcode_parse(const char* str, struct security_mcode* mcode)
{
    size_t len;

    log_assert(str);
    log_assert(mcode);

    memset(mcode, ' ', sizeof(struct security_mcode));

    len = strlen(str);

    if (len > sizeof(struct security_mcode)) {
        return false;
    }

    memcpy(mcode, str, len);

    return true;
}

char* security_mcode_to_str(const struct security_mcode* mcode)
{
    char* str;

    log_assert(mcode);

    str = xmalloc(sizeof(struct security_mcode) + 1);
    memcpy(str, mcode, sizeof(struct security_mcode));
    str[sizeof(struct security_mcode)] = '\0';

    return str;
}