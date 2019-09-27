#ifndef JBHOOK1_P3IO_H
#define JBHOOK1_P3IO_H

#include <windows.h>

#include "hook/iohook.h"

#include "security/id.h"
#include "security/mcode.h"

/**
 * Initialize the jbhook1 specific p3io emulation backend.
 * 
 * @param mcode Mcode of the target game to run. Required for dongle emulation.
 * @param pcbid PCBDID
 * @param eamid EAMID
 */
void jbhook1_p3io_init(const struct security_mcode* mcode,
        const struct security_id* pcbid, const struct security_id* eamid);

/**
 * Shutdown the p3io emulation backend.
 */
void jbhook1_p3io_fini(void);

#endif
