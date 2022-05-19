#ifndef DDRHOOK_UTIL_P3IO_H
#define DDRHOOK_UTIL_P3IO_H

#include <windows.h>

#include "hook/iohook.h"

#include "security/rp-sign-key.h"
#include "security/rp3.h"

extern const wchar_t p3io_dev_node_prefix[];
extern const wchar_t p3io_dev_node[];

void p3io_ddr_init(void);
void p3io_ddr_init_with_plugs(
    const struct security_mcode *mcode,
    const struct security_id *pcbid,
    const struct security_id *eamid,
    const struct security_rp_sign_key *black_sign_key,
    const struct security_rp_sign_key *white_sign_key);
void p3io_ddr_fini(void);

#endif
