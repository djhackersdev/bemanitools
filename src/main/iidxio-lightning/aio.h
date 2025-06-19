#pragma once

#include <stdint.h>

struct AIO_NODE;
struct AIO_NMGR;
struct AIO_SCI;

#pragma pack(push, 1)
struct AIO_IOB2_BI2X_TDJ__DEVSTATUS {
    uint8_t unk1[4];
    uint8_t b_test;
    uint8_t b_service;
    uint8_t b_coinmech;
    uint8_t unk2;
    uint8_t b_start[2];
    uint8_t b_vefx;
    uint8_t b_effect;
    uint8_t b_headphone[2];

    uint8_t unk3[6]; // 0x10-0x13
    uint8_t a_turntable[2];
    uint8_t unk4;
    uint8_t unk6[4];
    uint8_t b_p1[7];
    uint8_t b_p2[7];
    uint8_t unk7[7];

    uint8_t unk8[0x9A]; // 0x30-0xC9
};
#pragma pack(pop)

_Static_assert(
    sizeof(struct AIO_IOB2_BI2X_TDJ__DEVSTATUS) == 202,
    "bi2x_device_status is the wrong size");

typedef void (*t_aioNodeCtl_UpdateDevicesStatus)(void);
typedef unsigned int (*t_aioNodeCtl_Destroy)(struct AIO_NODE *node);
typedef unsigned int (*t_aioNodeMgr_Destroy)(struct AIO_NMGR *mgr);
typedef unsigned int (*t_aioSci_Destroy)(struct AIO_SCI *sci);

typedef struct AIO_SCI* (*t_aioIob2Bi2x_OpenSciUsbCdc)(uint8_t unk);
typedef struct AIO_NMGR* (*t_aioNMgrIob2_Create)(struct AIO_SCI *port, unsigned int delay);

typedef struct AIO_NODE* (*t_aioIob2Bi2xTDJ_Create)(struct AIO_NMGR *node_mgr, uint8_t param);
typedef void (*t_aioIob2Bi2xTDJ_IoReset)(struct AIO_NODE *AIO_IOB2_BI2X_TDJ_this, int value);
typedef void (*t_AIO_IOB2_BI2X_TDJ__GetDeviceStatus)(struct AIO_NODE *AIO_IOB2_BI2X_TDJ_this, struct AIO_IOB2_BI2X_TDJ__DEVSTATUS *status);
typedef void (*t_AIO_IOB2_BI2X_TDJ__SetTurnTableResist)(struct AIO_NODE *AIO_IOB2_BI2X_TDJ_this, int side, uint8_t resist);
