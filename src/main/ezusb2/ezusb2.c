#include <windows.h>
#include <setupapi.h>

#include "ezusb/ezusb.h"

#include "ezusb2/cyioctl.h"
#include "ezusb2/ezusb2.h"

#include "util/crc.h"
#include "util/fs.h"
#include "util/log.h"
#include "util/str.h"

#define REQ_TYPE_HOST_TO_DEV 0x40
#define REQ_TYPE_DEV_TO_HOST 0x80

#define REQ_TIMEOUT_SEC 5
#define REQ_CODE_EP0 0x00

#define EP0_GET_DESCRIPTOR 0x06
#define EP0_EZUSB_WRITE_RAM 0xA0

/* nope, it's not 0x01, probably an endianess mistake (?) */
#define EP0_DESCRIPTOR_TYPE_DEVICE 0x0100
/* Also not 0x03, 0x0301 from DeviceIoCtl dump */
#define EP0_DESCRIPTOR_TYPE_STRING 0x0301

#define EP0_EZUSB_RAM_CPU_RESET_ADDR 0xE600

static int ezusb2_ctrl_transfer(HANDLE handle, uint32_t ioctl_code, 
        uint8_t endpoint, uint8_t req_type, uint8_t req, uint16_t value, 
        uint16_t index, void* buf, uint32_t buf_len)
{
    int xmit_buf_size = sizeof(SINGLE_TRANSFER) + buf_len;
    char xmit_buf[xmit_buf_size];
    DWORD ret_len;

    memset(xmit_buf, 0, xmit_buf_size);

    PSINGLE_TRANSFER transfer = (PSINGLE_TRANSFER) xmit_buf;

    memset(transfer, 0, sizeof(SINGLE_TRANSFER));

    transfer->SetupPacket.bmRequest = req_type;
    transfer->SetupPacket.bRequest = req;
    transfer->SetupPacket.wValue = value;
    transfer->SetupPacket.wIndex = index;
    transfer->SetupPacket.wLength = buf_len;
    /* Timeout is in seconds, wtf ?! */
    transfer->SetupPacket.ulTimeOut = REQ_TIMEOUT_SEC;
    transfer->ucEndpointAddress = endpoint;
    transfer->BufferOffset = sizeof(SINGLE_TRANSFER);
    transfer->BufferLength = buf_len;
    /* All other attributes 0 */

    if (buf) {
        memcpy(xmit_buf + sizeof(SINGLE_TRANSFER), buf, buf_len);
    }

    if (!DeviceIoControl(handle, ioctl_code, xmit_buf, xmit_buf_size, xmit_buf, 
            xmit_buf_size, &ret_len, NULL)) {
        return -1;
    }

    if (buf) {
        memcpy(buf, xmit_buf + sizeof(SINGLE_TRANSFER), buf_len);
    }

    return ret_len - sizeof(SINGLE_TRANSFER);
}

static int ezusb2_ep0_transfer(HANDLE handle, uint8_t req_type, uint8_t req, 
        uint16_t value, uint16_t index, void* buf, uint32_t buf_len)
{
    return ezusb2_ctrl_transfer(handle, IOCTL_ADAPT_SEND_EP0_CONTROL_TRANSFER, 
        REQ_CODE_EP0, req_type, req, value, index, buf, buf_len);
}

/* Any other endpoint than 0 */
static int ezusb2_epx_transfer(HANDLE handle, uint8_t endpoint, 
        uint8_t req_type, uint8_t req, uint16_t value, uint16_t index, 
        void* buf, uint32_t buf_len)
{
    return ezusb2_ctrl_transfer(handle, IOCTL_ADAPT_SEND_NON_EP0_TRANSFER, 
        endpoint, req_type, req, value, index, buf, buf_len);
}

static char* ezusb2_get_device_name(HANDLE handle)
{
    const int len = 256;
    char buf[len];
    DWORD received;

    memset(&buf, 0, len);

    if (!DeviceIoControl(handle, IOCTL_ADAPT_GET_FRIENDLY_NAME, &buf, len, &buf, 
            len, &received, NULL)) {
        return NULL;
    }

    return str_dup((const char*) &buf);
}

static bool ezusb2_get_device_descriptor(HANDLE handle, 
        USB_DEVICE_DESCRIPTOR* desc)
{
    int res;

    memset(desc, 0, sizeof(USB_DEVICE_DESCRIPTOR));

    /* 0: not used */
    res = ezusb2_ep0_transfer(handle, REQ_TYPE_DEV_TO_HOST, EP0_GET_DESCRIPTOR, 
        EP0_DESCRIPTOR_TYPE_DEVICE, 0, desc, sizeof(USB_DEVICE_DESCRIPTOR));

    if (res != sizeof(USB_DEVICE_DESCRIPTOR)) {
        return false;
    }

    return true;
}

static bool ezusb2_write_ram(HANDLE handle, const void* buffer, 
        uint16_t ram_offset, uint32_t size)
{
    return ezusb2_ep0_transfer(handle, REQ_TYPE_HOST_TO_DEV, EP0_EZUSB_WRITE_RAM, 
        ram_offset, 0, (void*) buffer, size) == size;
}

static bool ezusb2_reset(HANDLE handle, bool hold)
{
    uint8_t flag;

    /* Write CPU state */
    flag = hold ? 1 : 0;

    return ezusb2_write_ram(handle, &flag, EP0_EZUSB_RAM_CPU_RESET_ADDR, 
        sizeof(uint8_t));
}

char* ezusb2_find(const GUID* guid)
{
    HDEVINFO info;
    SP_DEVICE_INTERFACE_DATA iface;
    BOOL result;
    ULONG required_len;
	PSP_DEVICE_INTERFACE_DETAIL_DATA detail;
	ULONG length;
    char* res;

    log_assert(guid);

    result = FALSE;
    required_len = 0;
	detail = NULL;

    info = SetupDiGetClassDevs(guid, NULL, NULL, 
        DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);

    if (info == INVALID_HANDLE_VALUE) {
		return NULL;
	}

    /* Enum devices that support the GUID, get first only */
    memset(&iface, 0, sizeof(iface));
    iface.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

    result = SetupDiEnumDeviceInterfaces(info, NULL, guid, 0, &iface);

    if (!result) {
        SetupDiDestroyDeviceInfoList(info);
        return NULL;
    }

    /* Get information about the found device interface. We don't
       know how much memory to allocate to get this information, so
       we will ask by passing in a null buffer and location to
       receive the size of the buffer needed. */
    result = SetupDiGetDeviceInterfaceDetail(info, &iface, NULL, 0, 
        &required_len, NULL);

    if (!required_len) {
        return NULL;
    }

    /* Okay, we got a size back, so let's allocate memory 
       for the interface detail information we want. */
    detail = (PSP_DEVICE_INTERFACE_DETAIL_DATA) LocalAlloc(LMEM_FIXED,
        required_len);

    if (detail == NULL) {
        SetupDiDestroyDeviceInfoList(info);
        return NULL;
    }

    detail->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
    length = required_len;

    result = SetupDiGetDeviceInterfaceDetail(info, &iface, detail, 
        length, &required_len, NULL);

    if (!result) {
        SetupDiDestroyDeviceInfoList(info);
        LocalFree(detail);
        return NULL;
    }

    res = str_dup((const char*) &detail->DevicePath);
    LocalFree(detail);

    SetupDiDestroyDeviceInfoList(info);

    return res;
}

HANDLE ezusb2_open(const char* device_path)
{
    log_assert(device_path);

    return CreateFileA(device_path, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, 
        OPEN_EXISTING, 0, NULL);
}

bool ezusb2_get_ident(HANDLE handle, struct ezusb_ident* ident)
{
    char* name;
    USB_DEVICE_DESCRIPTOR desc;

    log_assert(handle != INVALID_HANDLE_VALUE);
    log_assert(handle);
    log_assert(ident);

    memset(ident, 0, sizeof(struct ezusb_ident));

    name = ezusb2_get_device_name(handle);

    if (!name) {
        return false;
    }

    memcpy(ident->name, name, strlen(name));
    free(name);

    if (!ezusb2_get_device_descriptor(handle, &desc)) {
        return false;
    }

    ident->vid = desc.idVendor;
    ident->pid = desc.idProduct;

    return true;
}

bool ezusb2_download_firmware(HANDLE handle, struct ezusb_firmware* fw)
{
    log_assert(handle != INVALID_HANDLE_VALUE);
    log_assert(handle);
    log_assert(fw);

    if (!ezusb2_reset(handle, true)) {
        return false;
    }

    for (uint16_t i = 0; i < fw->segment_count; i++) {
        /* Important: Writing the full binary does NOT work on the FX2 device
           compared to the legacy ezusb device */
        if (!ezusb2_write_ram(handle, fw->segments[i]->data, 
                fw->segments[i]->offset, fw->segments[i]->size)) {
            return false;
        }
    }

    if (!ezusb2_reset(handle, false)) {
        return false;
    }

    return true;
}

bool ezusb2_endpoint_transfer(HANDLE handle, uint8_t endpoint, void* data, 
        uint32_t size)
{
    if (endpoint == 0) {
        return false;
    }

    return ezusb2_epx_transfer(handle, endpoint, 0, 0, 0, 0, data, size);
}

void ezusb2_close(HANDLE handle)
{
    log_assert(handle != INVALID_HANDLE_VALUE);
    log_assert(handle);

    CloseHandle(handle);
}
