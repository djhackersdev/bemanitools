#ifndef EZUSB_IIDX_EEPROM_CMD_H
#define EZUSB_IIDX_EEPROM_CMD_H

/* commands are the same for v1 and v2 */
enum ezusb_iidx_eeprom_command {
    EZUSB_IIDX_EEPROM_CMD_READ = 0x02,
    EZUSB_IIDX_EEPROM_CMD_WRITE = 0x03
};

enum ezusb_iidx_eeprom_command_status_v1 {
    EZUSB_IIDX_EEPROM_CMD_STATUS_V1_READ_OK = 0x01,
    EZUSB_IIDX_EEPROM_CMD_STATUS_V1_WRITE_OK = 0x02,
    EZUSB_IIDX_EEPROM_CMD_STATUS_V1_FAULT = 0xFE
};

enum ezusb_iidx_eeprom_command_status_v2 {
    EZUSB_IIDX_EEPROM_CMD_STATUS_V2_READ_OK = 0x21,
    EZUSB_IIDX_EEPROM_CMD_STATUS_V2_WRITE_OK = 0x22,
    EZUSB_IIDX_EEPROM_CMD_STATUS_V2_FAULT = 0xFE
};

#endif