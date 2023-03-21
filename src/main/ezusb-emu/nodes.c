/*
 * This file contains the ezusb nodes that appeared on
 * all IIDX games so far.
 */

#include "ezusb-iidx/msg.h"

#include "ezusb-emu/node-coin.h"
#include "ezusb-emu/node-eeprom.h"
#include "ezusb-emu/node-none.h"
#include "ezusb-emu/node-security-mem.h"
#include "ezusb-emu/node-security-plug.h"
#include "ezusb-emu/node-sram.h"
#include "ezusb-emu/node-wdt.h"
#include "ezusb-emu/node.h"
#include "ezusb-emu/nodes.h"

/* All IIDX games */
const struct ezusb_iidx_emu_node ezusb_iidx_emu_node_coin = {
    .node_id = EZUSB_IIDX_MSG_NODE_COIN,
    .init_node = NULL,
    .process_cmd = ezusb_iidx_emu_node_coin_process_cmd,
    .read_packet = ezusb_iidx_emu_node_coin_read_packet,
    .write_packet = ezusb_iidx_emu_node_coin_write_packet};

/* Used on 9th to DistorteD */
const struct ezusb_iidx_emu_node ezusb_iidx_emu_node_eeprom_v1 = {
    .node_id = EZUSB_IIDX_MSG_NODE_EEPROM,
    .init_node = ezusb_iidx_emu_node_eeprom_init,
    .process_cmd = ezusb_iidx_emu_node_eeprom_process_cmd_v1,
    .read_packet = ezusb_iidx_emu_node_eeprom_read_packet,
    .write_packet = ezusb_iidx_emu_node_eeprom_write_packet};

/* Used on Gold to Sirius */
const struct ezusb_iidx_emu_node ezusb_iidx_emu_node_eeprom_v2 = {
    .node_id = EZUSB_IIDX_MSG_NODE_EEPROM,
    .init_node = ezusb_iidx_emu_node_eeprom_init,
    .process_cmd = ezusb_iidx_emu_node_eeprom_process_cmd_v2,
    .read_packet = ezusb_iidx_emu_node_eeprom_read_packet,
    .write_packet = ezusb_iidx_emu_node_eeprom_write_packet};

/* All IIDX games */
const struct ezusb_iidx_emu_node ezusb_iidx_emu_node_none = {
    .node_id = EZUSB_IIDX_MSG_NODE_NONE,
    .init_node = NULL,
    .process_cmd = ezusb_iidx_emu_node_none_process_cmd,
    .read_packet = ezusb_iidx_emu_node_none_read_packet,
    .write_packet = ezusb_iidx_emu_node_none_write_packet};

/* Used on 9th to DistorteD */
const struct ezusb_iidx_emu_node ezusb_iidx_emu_node_security_mem_v1 = {
    .node_id = EZUSB_IIDX_MSG_NODE_SECURITY_MEM,
    .init_node = ezusb_iidx_emu_node_security_mem_init,
    .process_cmd = ezusb_iidx_emu_node_security_mem_v1_process_cmd,
    .read_packet = ezusb_iidx_emu_node_security_mem_read_packet,
    .write_packet = ezusb_iidx_emu_node_security_mem_write_packet};

/* Used on Gold to Sirius */
const struct ezusb_iidx_emu_node ezusb_iidx_emu_node_security_mem_v2 = {
    .node_id = EZUSB_IIDX_MSG_NODE_SECURITY_MEM,
    .init_node = ezusb_iidx_emu_node_security_mem_init,
    .process_cmd = ezusb_iidx_emu_node_security_mem_v2_process_cmd,
    .read_packet = ezusb_iidx_emu_node_security_mem_read_packet,
    .write_packet = ezusb_iidx_emu_node_security_mem_write_packet};

/* Used on 9th to DistorteD */
const struct ezusb_iidx_emu_node ezusb_iidx_emu_node_security_plug_v1 = {
    .node_id = EZUSB_IIDX_MSG_NODE_SECURITY_PLUG,
    .init_node = NULL,
    .process_cmd = ezusb_iidx_emu_node_security_plug_process_cmd_v1,
    .read_packet = ezusb_iidx_emu_node_security_plug_read_packet_v1,
    .write_packet = ezusb_iidx_emu_node_security_plug_write_packet};

/* Used on Gold to Sirius */
const struct ezusb_iidx_emu_node ezusb_iidx_emu_node_security_plug_v2 = {
    .node_id = EZUSB_IIDX_MSG_NODE_SECURITY_PLUG,
    .init_node = NULL,
    .process_cmd = ezusb_iidx_emu_node_security_plug_process_cmd_v2,
    .read_packet = ezusb_iidx_emu_node_security_plug_read_packet_v2,
    .write_packet = ezusb_iidx_emu_node_security_plug_write_packet};

/* All IIDX games */
const struct ezusb_iidx_emu_node ezusb_iidx_emu_node_sram = {
    .node_id = EZUSB_IIDX_MSG_NODE_SRAM,
    .init_node = NULL,
    .process_cmd = ezusb_iidx_emu_node_sram_process_cmd,
    .read_packet = ezusb_iidx_emu_node_sram_read_packet,
    .write_packet = ezusb_iidx_emu_node_sram_write_packet};

/* Used on 9th to Sirius */
const struct ezusb_iidx_emu_node ezusb_iidx_emu_node_wdt = {
    .node_id = EZUSB_IIDX_MSG_NODE_WDT,
    .init_node = NULL,
    .process_cmd = ezusb_iidx_emu_node_wdt_process_cmd,
    .read_packet = ezusb_iidx_emu_node_wdt_read_packet,
    .write_packet = ezusb_iidx_emu_node_wdt_write_packet};
