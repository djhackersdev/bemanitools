#ifndef SECURITY_RP2_H
#define SECURITY_RP2_H

#include <stdbool.h>
#include <stdint.h>

#include "security/id.h"
#include "security/mcode.h"
#include "security/rp-util.h"

/**
 * Structure for data which is usually stored in the eeprom section of the
 * dongle. This contains a signiture to verify the ROM's contents as well as
 * the game this dongle is signed for.
 */
struct security_rp2_eeprom {
    uint8_t signature[6];
    uint8_t packed_payload[6];
};

/**
 * Generate signed eeprom data from non encrypted and unobfuscated data required
 * to pass security checks on games using black (game specific) and white
 * (eamuse) roundplugs.
 * 
 * This implementation (rp 2) is used by the following games
 * - iidx 14 to 17
 * 
 * @param type Type of plug to sign eeprom data for (black or white).
 * @param boot_version The boot version mcode that is used for bootstrapping
 *                     the security backend (of the ezusb.dll).
 * @param plug_mcode The mcode of the game to boot. Typically, this code is
 *                   printed onto the housing of the black dongle. For white
 *                   dongles, the "mcode" @@@@@@@@ is used.
 * @param plug_id The id stored on the plug. This data is normally stored in the
 *                ROM area of the black dongle is is often refered to as the
 *                "PCBID" or "EAMID" when stored on the white dongle.
 * @param out Pointer to the eeprom data struct for the resulting data. 
 */
void security_rp2_generate_signed_eeprom_data(
        enum security_rp_util_rp_type type,
        const struct security_mcode* boot_version,
        const struct security_mcode* plug_mcode,
        const struct security_id* plug_id, struct security_rp2_eeprom* out);

#endif