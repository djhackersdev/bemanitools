#ifndef EZUSB_IIDX_EMU_NODE_SECURITY_PLUG_H
#define EZUSB_IIDX_EMU_NODE_SECURITY_PLUG_H

#include "ezusb-iidx-emu/node.h"

#include "security/id.h"
#include "security/mcode.h"
#include "security/rp-sign-key.h"

/**
 * Set the boot version of the game. The boot version mcode is used for
 * bootstrapping the security backend (of the ezusb.dll).
 *
 * @param boot_version Pointer to the boot version to set.
 */
void ezusb_iidx_emu_node_security_plug_set_boot_version(
    const struct security_mcode *boot_version);

/**
 * Set the boot seeds. The boot seeds (three numbers >= 0) are used for
 * bootstrapping the security backend (of the ezusb.dll).
 *
 * @param seeds Pointer to an array of three boot seeds to set.
 */
void ezusb_iidx_emu_node_security_plug_set_boot_seeds(const uint32_t *seeds);

/**
 * Set the signing key for the black dongle.
 *
 * @param sign_key Pointer to the black dongle sign key.
 */
void ezusb_iidx_emu_node_security_plug_set_plug_black_sign_key(
    const struct security_rp_sign_key *sign_key);

/**
 * Set the signing key for the white dongle.
 *
 * @param sign_key Pointer to the white dongle sign key.
 */
void ezusb_iidx_emu_node_security_plug_set_plug_white_sign_key(
    const struct security_rp_sign_key *sign_key);

/**
 * The mcode of the target game to boot. This mcode is typically printed onto
 * the black plug corresponding to the game to boot.
 *
 * @param mcode Pointer to the mcode to set.
 */
void ezusb_iidx_emu_node_security_plug_set_plug_black_mcode(
    const struct security_mcode *mcode);

/**
 * The mcode of the target game to boot. This is not an actual mcode but to our
 * knowledge is always the string "@@@@@@@@" which is used to identify the
 * white plug.
 *
 * @param mcode Pointer to the mcode to set.
 */
void ezusb_iidx_emu_node_security_plug_set_plug_white_mcode(
    const struct security_mcode *mcode);

/**
 * Set the PCBID. This ID is stored in the ROM of the black plug and is
 * basically the serial number for the copy of the game.
 *
 * @param pcbid Pointer to the PCBID to set.
 */
void ezusb_iidx_emu_node_security_plug_set_pcbid(
    const struct security_id *pcbid);

/**
 * Set the EAMID. This ID is stored in the ROM of the white plug and is
 * basically the identifier for eamuse participation. This ID (and the white
 * dongle) were not available on IIDX 09 to 13 and got introduced with IIDX 14
 * (and a bunch of other games). I assume the idea was to have the same EAMID
 * accross multiple games (e.g. same arcade) but different PCBIDs still
 * identifying the copies of the games running.
 *
 * @param eamid Pointer to the EAMID to set.
 */
void ezusb_iidx_emu_node_security_plug_set_eamid(
    const struct security_id *eamid);

/**
 * Trigger a command on the security node. This call is used on IIDX 9 to 13.
 *
 * @param cmd_id Id of the command (see enums).
 * @param cmd_data Additional data for command (used/unused depending on cmd).
 * @param cmd_data2 Additional data for command (used/unused depending on cmd).
 * @return Status value of the executed operation.
 */
uint8_t ezusb_iidx_emu_node_security_plug_process_cmd_v1(
    uint8_t cmd_id, uint8_t cmd_data, uint8_t cmd_data2);

/**
 * Trigger a command on the security node. This call is used on IIDX 14 to 17.
 *
 * @param cmd_id Id of the command (see enums).
 * @param cmd_data Additional data for command (used/unused depending on cmd).
 * @param cmd_data2 Additional data for command (used/unused depending on cmd).
 * @return Status value of the executed operation.
 */
uint8_t ezusb_iidx_emu_node_security_plug_process_cmd_v2(
    uint8_t cmd_id, uint8_t cmd_data, uint8_t cmd_data2);

/**
 * Bulk endpoint to read data from the security node. This is preceeded by
 * calling a sepcific command (see process cmd functions and enums of cmds).
 *
 * This version is used on IIDX 9 to 13.
 *
 * @param pkg Pointer to a bulk package to read the data into.
 * @return True on success, false on error.
 */
bool ezusb_iidx_emu_node_security_plug_read_packet_v1(
    struct ezusb_iidx_msg_bulk_packet *pkg);

/**
 * Bulk endpoint to read data from the security node. This is preceeded by
 * calling a sepcific command (see process cmd functions and enums of cmds).
 *
 * This version is used on IIDX 14 to 17.
 *
 * @param pkg Pointer to a bulk package to read the data into.
 * @return True on success, false on error.
 */
bool ezusb_iidx_emu_node_security_plug_read_packet_v2(
    struct ezusb_iidx_msg_bulk_packet *pkg);

/**
 * Bulk endpoint to provide data to write to the security node. This is
 * preceeded by calling a sepcific command (see process cmd functions and enums
 * of cmds).
 *
 * @param pkg Pointer to a bulk package with data to write to the node.
 * @result True on success, false on error.
 */
bool ezusb_iidx_emu_node_security_plug_write_packet(
    const struct ezusb_iidx_msg_bulk_packet *pkg);

#endif