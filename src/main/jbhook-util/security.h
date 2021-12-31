#ifndef JBHOOK_UTIL_SECURITY_H
#define JBHOOK_UTIL_SECURITY_H

#include "security/mcode.h"

static const struct security_mcode jbhook_util_security_default_mcode = {
    .header = SECURITY_MCODE_HEADER,
    .unkn = SECURITY_MCODE_UNKN_C,
    .game = SECURITY_MCODE_GAME_JB_1,
    .region = SECURITY_MCODE_REGION_JAPAN,
    .cabinet = SECURITY_MCODE_CABINET_A,
    .revision = SECURITY_MCODE_REVISION_B,
};

#endif
