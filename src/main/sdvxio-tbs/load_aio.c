#include <windows.h>

#include <stdbool.h>
#include <stdint.h>

#define LOG_MODULE "load_aio"
#include "util/log.h"

#include "load_aio.h"

#define DECLARE_MODULE_FUNCTION(NAME) t_ ## NAME NAME

struct aio_module {
    // "libaio.dll"
    DECLARE_MODULE_FUNCTION(aioNodeCtl_UpdateDevicesStatus);
    DECLARE_MODULE_FUNCTION(aioNodeCtl_Destroy);
    DECLARE_MODULE_FUNCTION(aioNodeMgr_Destroy);
    DECLARE_MODULE_FUNCTION(aioSci_Destroy);
    // "libaio-iob.dll"
    DECLARE_MODULE_FUNCTION(aioIob2Bi2x_OpenSciUsbCdc);
    DECLARE_MODULE_FUNCTION(aioNMgrIob2_Create);
    // "libaio-iob2_video.dll"
    DECLARE_MODULE_FUNCTION(aioIob2Bi2xTBS_Create);
    // technically the game uses the C++ one, but this one doesn't incur a penalty unlike the C version of aioIob2Bi2xTBS_GetDeviceStatus
    DECLARE_MODULE_FUNCTION(aioIob2Bi2xTBS_IoReset);
    DECLARE_MODULE_FUNCTION(aioIob2Bi2xTBS_GetDeviceStatus);
};

void warn_module(const char *function_name) {
    log_warning("Could not load %s!", function_name);
}

#define LOAD_MODULE_FUNCTION(TARGET, HANDLE, NAME) do {TARGET.NAME = (t_ ## NAME)GetProcAddress(HANDLE, #NAME); if (TARGET.NAME == NULL) {warn_module(#NAME); return;}} while(0)
#define LOAD_MODULE_FUNCTION_EX(TARGET, HANDLE, NAME, REAL_NAME) do {TARGET.NAME = (t_ ## NAME)GetProcAddress(HANDLE, REAL_NAME); if (TARGET.NAME == NULL) {warn_module(#NAME); return;}} while(0)

static struct aio_module module_handles;

void init_aio_handles(void) {
    HMODULE h_aio = LoadLibrary("libaio.dll");
    HMODULE h_aio_iob = LoadLibrary("libaio-iob.dll");
    HMODULE h_aio_iob2_video = LoadLibrary("libaio-iob2_video.dll");

    if (!h_aio) {
        log_warning("Could not load libaio.dll!");
        return;
    }

    if (!h_aio_iob) {
        log_warning("Could not load libaio-iob.dll!");
        return;
    }

    if (!h_aio_iob2_video) {
        log_warning("Could not load libaio-iob2_video.dll!");
        return;
    }

    // "libaio.dll"
    LOAD_MODULE_FUNCTION(module_handles, h_aio, aioNodeCtl_UpdateDevicesStatus);
    LOAD_MODULE_FUNCTION(module_handles, h_aio, aioNodeCtl_Destroy);
    LOAD_MODULE_FUNCTION(module_handles, h_aio, aioNodeMgr_Destroy);
    LOAD_MODULE_FUNCTION(module_handles, h_aio, aioSci_Destroy);

    // "libaio-iob.dll"
    LOAD_MODULE_FUNCTION(module_handles, h_aio_iob, aioIob2Bi2x_OpenSciUsbCdc);
    LOAD_MODULE_FUNCTION(module_handles, h_aio_iob, aioNMgrIob2_Create);


    // "libaio-iob2_video.dll"
    LOAD_MODULE_FUNCTION(module_handles, h_aio_iob2_video, aioIob2Bi2xTBS_Create);
    LOAD_MODULE_FUNCTION(module_handles, h_aio_iob2_video, aioIob2Bi2xTBS_IoReset);
    LOAD_MODULE_FUNCTION(module_handles, h_aio_iob2_video, aioIob2Bi2xTBS_GetDeviceStatus);
    // LOAD_MODULE_FUNCTION_EX(module_handles, h_aio_iob2_video, aioIob2Bi2xTBS_GetDeviceStatus, "?GetDeviceStatus@AIO_IOB2_BI2X_TDJ@@QEBAXAEAUDEVSTATUS@1@@Z");

    log_misc("Done loading all libaio functions required.");
}

struct AIO_NMGR_VTABLE {
    void *_dtor;
    void *AddNodeCtl;
    void *RemoveNodeCtl;
    void *PacketCreate;
    void *PacketSend;
    void *QueryPktRecvTimeout;
    void *PacketRecv;
    void *_GetCommStatus;
    void *_Reconnect;
    void *_unk;
    void (*BeginManage)(struct AIO_NMGR *);
};

struct AIO_NMGR {
    struct AIO_NMGR_VTABLE *vtbl;
    uint8_t data[0x9F0];
};

struct bi2x_ctx {
  struct AIO_SCI *sci_usb;
  struct AIO_NMGR *iob2_mgr;
  struct AIO_NODE *iob_bi2x_node;
};

struct bi2x_ctx *setup_bi2x(void) {
    struct bi2x_ctx *ctx = malloc(sizeof(struct bi2x_ctx));

    ctx->sci_usb = module_handles.aioIob2Bi2x_OpenSciUsbCdc(0);
    if (!ctx->sci_usb) {
        log_warning("Unable to aioIob2Bi2x_OpenSciUsbCdc");
        free(ctx);
        return NULL;
    }

    ctx->iob2_mgr = module_handles.aioNMgrIob2_Create(ctx->sci_usb, 0);
    if (!ctx->iob2_mgr) {
        log_warning("Unable to aioNMgrIob2_Create");
        free(ctx);
        return NULL;
    }

    ctx->iob_bi2x_node = module_handles.aioIob2Bi2xTBS_Create(ctx->iob2_mgr, 1u);
    if (!ctx->iob_bi2x_node) {
        log_warning("Unable to aioIob2Bi2xTBS_Create?");
        free(ctx);
        return NULL;
    }

    ctx->iob2_mgr->vtbl->BeginManage(ctx->iob2_mgr);
    log_misc("BeginManage iob2_mgr");

    // never actually called
    // module_handles.aioIob2Bi2xTBS_IoReset(ctx->iob_bi2x_node, 0x11);

    return ctx;
}

void close_bi2x(struct bi2x_ctx *ctx) {
    if (ctx->iob_bi2x_node) {
        module_handles.aioNodeCtl_Destroy(ctx->iob_bi2x_node);
    }

    if (ctx->iob2_mgr) {
        module_handles.aioNodeMgr_Destroy(ctx->iob2_mgr);
    }

    if (ctx->sci_usb) {
        module_handles.aioSci_Destroy(ctx->sci_usb);
    }
}

void poll_bi2x(struct bi2x_ctx *ctx, struct AIO_IOB2_BI2X_TBS__DEVSTATUS *status) {
    module_handles.aioNodeCtl_UpdateDevicesStatus();

    // technically we should do some checking here

    module_handles.aioIob2Bi2xTBS_GetDeviceStatus(ctx->iob_bi2x_node, status, sizeof(struct AIO_IOB2_BI2X_TBS__DEVSTATUS));
}
