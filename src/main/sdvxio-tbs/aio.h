#pragma once

#include <stdint.h>

struct AIO_NODE;
struct AIO_NMGR;
struct AIO_SCI;

struct INPUT
{
   char DevIoCounter;
   char bExIoAErr;
   char bExIoBErr;
   char bPcPowerOn;
   char bPcPowerCheck;
   char CoinCount;
   char bTest;
   char bService;
   char bCoinSw;
   char bCoinJam;
   char bHPDetect;
   uint16_t StickX;
   uint16_t StickY;
   char bStickBtn;
   char bTrigger1;
   char bTrigger2;
   char bButton0;
   char bButton1;
   char bButton2;
   char bButton3;
};

struct INPUTDATA
{
    char Data[10];
};

struct OUTPUTDATA
{
    char Data[8];
};

struct IORESETDATA
{
    char Data[4];
};

struct AIO_IOB2_BI2X_TBS__DEVSTATUS {
    char InputCounter;
    char OutputCounter;
    char TapeLedCounter;
    struct INPUT Input;
    struct INPUTDATA InputData;
    struct OUTPUTDATA OutputData;
    struct IORESETDATA IoResetData;
};

_Static_assert(
    sizeof(struct AIO_IOB2_BI2X_TBS__DEVSTATUS) == 50,
    "bi2x_device_status is the wrong size");

typedef void (*t_aioNodeCtl_UpdateDevicesStatus)(void);
typedef unsigned int (*t_aioNodeCtl_Destroy)(struct AIO_NODE *node);
typedef unsigned int (*t_aioNodeMgr_Destroy)(struct AIO_NMGR *mgr);
typedef unsigned int (*t_aioSci_Destroy)(struct AIO_SCI *sci);

typedef struct AIO_SCI* (*t_aioIob2Bi2x_OpenSciUsbCdc)(uint8_t unk);
typedef struct AIO_NMGR* (*t_aioNMgrIob2_Create)(struct AIO_SCI *port, unsigned int delay);

typedef struct AIO_NODE* (*t_aioIob2Bi2xTBS_Create)(struct AIO_NMGR *node_mgr, uint8_t param);
typedef void (*t_aioIob2Bi2xTBS_IoReset)(struct AIO_NODE *i_pNodeCtl, int value);
typedef void (*t_aioIob2Bi2xTBS_GetDeviceStatus)(struct AIO_NODE *i_pNodeCtl, struct AIO_IOB2_BI2X_TBS__DEVSTATUS *status, int size);
