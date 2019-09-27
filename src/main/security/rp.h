#ifndef SECURITY_RP_H
#define SECURITY_RP_H

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
struct security_rp_eeprom {
    uint8_t signature[6];
    uint8_t packed_payload[6];
};

/**
 * Generate signed eeprom data from non encrypted and unobfuscated data required
 * to pass security checks on games using black (game specific) roundplugs.
 * 
 * This implementation (rp (1)) is used by the following games
 * - iidx 09 to 13
 * 
 * @param boot_version The boot version mcode that is used for bootstrapping
 *                     the security backend (of the ezusb.dll).
 * @param boot_seeds Boot seeds (three numbers >= 0) set when the game is
 *                   bootstrapping the security backend (of the ezusb.dll).
 * @param plug_mcode The mcode of the game to boot. Typically, this code is
 *                   printed onto the housing of the black dongle. 
 * @param plug_id The id stored of the plug. This data is normally stored in the
 *                ROM area of the black dongle is is often refered to as the
 *                "PCBID" or "EAMID" when stored on the white dongle.
 * @param out Pointer to the eeprom data struct for the resulting data. 
 */
void security_rp_generate_signed_eeprom_data(
        const struct security_mcode* boot_version, const uint32_t* boot_seeds,
        const struct security_mcode* plug_mcode,
        const struct security_id* plug_id, struct security_rp_eeprom* out);

#endif