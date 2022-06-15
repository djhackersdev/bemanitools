#define LOG_MODULE "usbmem"

// clang-format off
// Don't format because the order is important here
#include <windows.h>
#include <devioctl.h>
#include <ntdef.h>
#include <ntddser.h>
// clang-format on

#include <stdbool.h>
#include <string.h>
#include <wchar.h>

#include "hook/iohook.h"

#include "util/crc.h"
#include "util/iobuf.h"
#include "util/log.h"
#include "util/fs.h"
#include "util/str.h"

#define USBMEM_DEVICE_COUNT 2
#define USBMEM_COMMAND_BUF_SIZE 128
#define USBMEM_DATA_BUF_SIZE (1024 + 5)

static HANDLE usbmem_fd;
static char usbmem_response[USBMEM_DATA_BUF_SIZE];
static bool usbmem_pending;
static size_t usbmem_response_length;
static bool usbmem_enabled;

static HRESULT usbmem_open(struct irp *irp);
static HRESULT usbmem_close(struct irp *irp);
static HRESULT usbmem_write(struct irp *irp);
static HRESULT usbmem_read(struct irp *irp);
static HRESULT usbmem_ioctl(struct irp *irp);

typedef enum {
    USBMEM_FILE_TYPE_NONE = 0,
    USBMEM_FILE_TYPE_READ,
    USBMEM_FILE_TYPE_WRITE,
} USBMEM_FILE_TYPE;

struct USBMEM_STATE {
    bool connected;
    bool opened;
    bool errored;
    USBMEM_FILE_TYPE file_type;

    char basepath[MAX_PATH];
    char path[MAX_PATH];
    char filename[MAX_PATH];

    uint8_t *buffer;
    size_t buffer_len;
    size_t buffer_index;
    int buffer_frame;
};

static int target_device_id;
static struct USBMEM_STATE usbmem_state[USBMEM_DEVICE_COUNT];

static void usbmem_reset_file_state(int port)
{
    if (usbmem_state[port].buffer) {
        free(usbmem_state[port].buffer);
        usbmem_state[port].buffer = NULL;
    }

    usbmem_state[port].buffer_len = 0;
    usbmem_state[port].buffer_index = 0;
    usbmem_state[port].buffer_frame = 0;
    usbmem_state[port].file_type = USBMEM_FILE_TYPE_NONE;
}

void usbmem_init(const char *path_p1, const char *path_p2, const bool enabled)
{
    log_assert(usbmem_fd == NULL);

    HRESULT hr;
    char usb_data_path[USBMEM_DEVICE_COUNT][MAX_PATH];

    hr = iohook_open_nul_fd(&usbmem_fd);

    if (hr != S_OK) {
        log_fatal("Opening nul fd failed: %08lx", hr);
    }

    usbmem_enabled = enabled;

    GetFullPathNameA(path_p1, sizeof(usb_data_path[0]), usb_data_path[0], NULL);
    log_misc("USB memory data path (P1): %s", usb_data_path[0]);

    GetFullPathNameA(path_p2, sizeof(usb_data_path[1]), usb_data_path[1], NULL);
    log_misc("USB memory data path (P2): %s", usb_data_path[1]);

    if (enabled && !path_exists(usb_data_path[0]) && !path_exists(usb_data_path[1])) {
        log_warning("USB memory data path does not exist, disabling");
        usbmem_enabled = false;
    }

    target_device_id = -1;
    for (int i = 0; i < USBMEM_DEVICE_COUNT; i++) {
        usbmem_state[i].connected = false;
        usbmem_state[i].opened = false;
        usbmem_state[i].errored = false;
        strcpy(usbmem_state[i].basepath, usb_data_path[i]);
        memset(usbmem_state[i].path, 0, sizeof(usbmem_state[i].path));
        memset(usbmem_state[i].filename, 0, sizeof(usbmem_state[i].filename));
        usbmem_reset_file_state(i);
    }
}

void usbmem_fini(void)
{
    if (usbmem_fd != NULL) {
        CloseHandle(usbmem_fd);
    }

    usbmem_fd = NULL;
}

HRESULT
usbmem_dispatch_irp(struct irp *irp)
{
    log_assert(irp != NULL);

    if (irp->op != IRP_OP_OPEN && irp->fd != usbmem_fd) {
        return iohook_invoke_next(irp);
    }

    switch (irp->op) {
        case IRP_OP_OPEN:
            return usbmem_open(irp);
        case IRP_OP_CLOSE:
            return usbmem_close(irp);
        case IRP_OP_READ:
            return usbmem_read(irp);
        case IRP_OP_WRITE:
            return usbmem_write(irp);
        case IRP_OP_IOCTL:
            return usbmem_ioctl(irp);
        default:
            return E_NOTIMPL;
    }
}

static HRESULT usbmem_open(struct irp *irp)
{
    log_assert(irp != NULL);

    if (!wstr_eq(irp->open_filename, L"COM3")) {
        return iohook_invoke_next(irp);
    }

    irp->fd = usbmem_fd;
    log_info("USB edit data PCB opened");

    return S_OK;
}

static HRESULT usbmem_close(struct irp *irp)
{
    log_info("USB edit data PCB closed");

    return S_OK;
}

static HRESULT usbmem_write(struct irp *irp)
{
    struct const_iobuf *src;
    char request[USBMEM_COMMAND_BUF_SIZE];
    uint32_t nbytes;

    log_assert(irp != NULL);
    log_assert(irp->write.bytes != NULL);

    src = &irp->write;
    nbytes = src->nbytes > USBMEM_COMMAND_BUF_SIZE ? USBMEM_COMMAND_BUF_SIZE : src->nbytes;
    memcpy(request, src->bytes, nbytes);
    request[nbytes - 1] = '\0'; /* This is always a CR. */

    if (!usbmem_pending && target_device_id >= 0 && target_device_id < USBMEM_DEVICE_COUNT && usbmem_state[target_device_id].file_type == USBMEM_FILE_TYPE_READ) {
        memset(usbmem_response, 0, sizeof(usbmem_response));

        // log_misc("Read progress %08x/%08x bytes", usbmem_state[target_device_id].buffer_index, usbmem_state[target_device_id].buffer_len);

        if (usbmem_state[target_device_id].buffer_index < usbmem_state[target_device_id].buffer_len) {
            usbmem_response_length = sizeof(usbmem_response);
            usbmem_response[0] = 0x02; // 1 = 0x80 buffer, 2 = 0x400 buffer
            usbmem_response[1] = usbmem_state[target_device_id].buffer_frame;
            usbmem_response[2] = ~usbmem_response[1];

            if (usbmem_state[target_device_id].buffer_frame == 0) {
                snprintf(usbmem_response + 4, sizeof(usbmem_response) - 5, "%Iu ", usbmem_state[target_device_id].buffer_len);
            } else {
                size_t len = sizeof(usbmem_response) - 5;

                if (usbmem_state[target_device_id].buffer_index + len > usbmem_state[target_device_id].buffer_len)
                    len = usbmem_state[target_device_id].buffer_len - usbmem_state[target_device_id].buffer_index;

                memcpy(usbmem_response + 3, usbmem_state[target_device_id].buffer + usbmem_state[target_device_id].buffer_index, len);
                usbmem_state[target_device_id].buffer_index += len;
            }

            usbmem_state[target_device_id].buffer_frame++;

            uint16_t crc = crc16_msb(usbmem_response + 3, sizeof(usbmem_response) - 5, 0);
            usbmem_response[sizeof(usbmem_response) - 2] = crc >> 8;
            usbmem_response[sizeof(usbmem_response) - 1] = crc & 0xff;
        } else {
            usbmem_response_length = 1;
            usbmem_response[0] = 0x04; // End
        }
    } else if (strlen(request) > 0) {
        log_misc(">%s", request);

        // Try to detect device ID
        // The only commands without a device ID are "sver", "start", and "init".
        char target_device_val = request[strlen(request) - 1];
        char *target_device_id_ptr = strstr(request, " ");
        if (target_device_id_ptr != NULL) {
            target_device_val = *(target_device_id_ptr - 1);
        }

        // TODO: Rewrite this code to more cleanly handle hotplugging USB drives
        if (str_eq(request, "sver")) {
            str_cpy(
                usbmem_response,
                sizeof(usbmem_response),
                "done GQHDXJAA DJHACKRS");
        } else if (str_eq(request, "init") || str_eq(request, "start")) {
            str_cpy(usbmem_response, sizeof(usbmem_response), "done");
        } else if (target_device_val == 'a' || target_device_val == 'b') {
            // Counterintuitively, b is P1 and a is P2
            target_device_id = target_device_val == 'b' ? 0 : 1;

            if (!usbmem_enabled) {
                // Ignore all other USB device specific commands and pretend a device isn't plugged in
                // when USB memory emulation is disabled.
                str_cpy(usbmem_response, sizeof(usbmem_response), "fail");
            } else if (usbmem_state[target_device_id].errored) {
                // If the device went through the entire process once and the file didn't exist
                // then just force it to be disabled until the game is restarted because otherwise
                // it'll get stuck in a loop.
                // TODO: This could be better emulated by using a keybind to simulate inserting and
                // ejecting the USB drive to additionally clear the error flag.
                str_cpy(usbmem_response, sizeof(usbmem_response), "fail");
            } else if (str_eq(request, "on_a") || str_eq(request, "on_b")) {
                usbmem_state[target_device_id].connected = path_exists(usbmem_state[target_device_id].basepath);
                usbmem_state[target_device_id].errored = false;

                usbmem_reset_file_state(target_device_id);

                if (usbmem_state[target_device_id].connected)
                    str_cpy(usbmem_response, sizeof(usbmem_response), "done");
                else
                    str_cpy(usbmem_response, sizeof(usbmem_response), "not connected");
            } else if (str_eq(request, "offa") || str_eq(request, "offb")) {
                if (usbmem_state[target_device_id].connected)
                    str_cpy(usbmem_response, sizeof(usbmem_response), "done");
                else
                    str_cpy(usbmem_response, sizeof(usbmem_response), "not connected");

                usbmem_state[target_device_id].connected = false;
                usbmem_reset_file_state(target_device_id);
            } else if (str_eq(request, "opna") || str_eq(request, "opnb")) {
                bool basepath_exists = path_exists(usbmem_state[target_device_id].basepath);

                if (!usbmem_state[target_device_id].connected || !basepath_exists) {
                    usbmem_state[target_device_id].opened = false;
                    str_cpy(usbmem_response, sizeof(usbmem_response), "not connected");
                } else {
                    usbmem_state[target_device_id].opened = true;
                    str_cpy(usbmem_response, sizeof(usbmem_response), "done");
                }
            } else if (str_eq(request, "clsa") || str_eq(request, "clsb")) {
                if (usbmem_state[target_device_id].opened)
                    str_cpy(usbmem_response, sizeof(usbmem_response), "done");
                else
                    str_cpy(usbmem_response, sizeof(usbmem_response), "not connected");

                usbmem_state[target_device_id].opened = false;
                usbmem_reset_file_state(target_device_id);
            } else if (strncmp(request, "cda ", 4) == 0 || strncmp(request, "cdb ", 4) == 0) {
                char *path = request + 4;

                if (!usbmem_state[target_device_id].connected || !usbmem_state[target_device_id].opened) {
                    str_cpy(usbmem_response, sizeof(usbmem_response), "done");
                } else if (path[1] == ':') {
                    // Absolute path
                    char temp[MAX_PATH];
                    snprintf(temp, sizeof(temp), "%s\\%s", usbmem_state[target_device_id].basepath, path + 3);

                    if (!path_exists(temp)) {
                        log_warning("Couldn't find path %s", temp);
                        str_cpy(usbmem_response, sizeof(usbmem_response), "done");
                    } else {
                        log_misc("Changing path to %s", temp);
                        str_cpy(usbmem_state[target_device_id].path, sizeof(usbmem_state[target_device_id].path), temp);
                        str_cpy(usbmem_response, sizeof(usbmem_response), "done");
                    }
                } else {
                    str_cpy(usbmem_response, sizeof(usbmem_response), "fail");
                }
            } else if (strncmp(request, "rda ", 4) == 0 || strncmp(request, "rdb ", 4) == 0) {
                usbmem_reset_file_state(target_device_id);

                if (!usbmem_state[target_device_id].connected || !usbmem_state[target_device_id].opened) {
                    str_cpy(usbmem_response, sizeof(usbmem_response), "fail");
                } else {
                    char temp[MAX_PATH] = {0};
                    char *filename = request + 4;

                    snprintf(temp, sizeof(temp), "%s\\%s", usbmem_state[target_device_id].path, filename);

                    if (usbmem_state[target_device_id].buffer) {
                        free(usbmem_state[target_device_id].buffer);
                        usbmem_state[target_device_id].buffer = NULL;
                    }

                    usbmem_state[target_device_id].file_type = USBMEM_FILE_TYPE_NONE;
                    usbmem_state[target_device_id].buffer_len = 0;
                    usbmem_state[target_device_id].buffer_index = 0;
                    usbmem_state[target_device_id].buffer_frame = 0;
                    memset(usbmem_state[target_device_id].filename, 0, sizeof(usbmem_state[target_device_id].filename));

                    if (!path_exists(temp)) {
                        log_warning("Couldn't find file %s", temp);
                        str_cpy(usbmem_response, sizeof(usbmem_response), "fail");
                        usbmem_state[target_device_id].errored = true;
                    } else {
                        bool loaded = file_load(temp, (void**)&usbmem_state[target_device_id].buffer,
                            &usbmem_state[target_device_id].buffer_len, false);

                        if (loaded) {
                            log_misc("Reading file %s", temp);
                            usbmem_state[target_device_id].file_type = USBMEM_FILE_TYPE_READ;

                            str_cpy(usbmem_state[target_device_id].filename, sizeof(usbmem_state[target_device_id].filename), filename);
                            str_cpy(usbmem_response, sizeof(usbmem_response), "start");
                        } else {
                            log_warning("Couldn't read file %s", temp);
                            str_cpy(usbmem_response, sizeof(usbmem_response), "fail");
                            usbmem_state[target_device_id].errored = true;
                        }
                    }
                }
            } else if (strncmp(request, "wra ", 4) == 0 || strncmp(request, "wrb ", 4) == 0) {
                // Open file for writing
                usbmem_reset_file_state(target_device_id);
                str_cpy(usbmem_response, sizeof(usbmem_response), "not supported");
            } else if (strncmp(request, "wha ", 4) == 0 || strncmp(request, "whb ", 4) == 0) {
                // Something relating to writing?
                str_cpy(usbmem_response, sizeof(usbmem_response), "not supported");
            } else if (strncmp(request, "lma ", 4) == 0 || strncmp(request, "lmb ", 4) == 0) {
                // What is "lm"?
                str_cpy(usbmem_response, sizeof(usbmem_response), "done");
            } else {
                str_cpy(usbmem_response, sizeof(usbmem_response), "fail");
            }
        } else {
            str_cpy(usbmem_response, sizeof(usbmem_response), "fail");
        }

        str_cat(usbmem_response, sizeof(usbmem_response), "\r>");
        usbmem_response_length = strlen(usbmem_response);
    }

    usbmem_pending = true;
    src->pos = nbytes;

    return S_OK;
}

static HRESULT usbmem_read(struct irp *irp)
{
    struct iobuf *dest;

    log_assert(irp != NULL);
    log_assert(irp->read.bytes != NULL);

    dest = &irp->read;

    if (usbmem_pending && usbmem_response_length != USBMEM_DATA_BUF_SIZE) {
        log_misc("%s", usbmem_response);
    }

    if (usbmem_pending && usbmem_response_length == 0) {
        str_cpy(usbmem_response, sizeof(usbmem_response), "\r>");
        usbmem_response_length = strlen(usbmem_response);
    }

    dest->pos = usbmem_response_length;
    memcpy(dest->bytes, usbmem_response, usbmem_response_length);

    usbmem_pending = false;
    usbmem_response_length = 0;
    memset(usbmem_response, 0, sizeof(usbmem_response));

    return S_OK;
}

static HRESULT usbmem_ioctl(struct irp *irp)
{
    SERIAL_STATUS *status;

    log_assert(irp != NULL);

    switch (irp->ioctl) {
        case IOCTL_SERIAL_GET_COMMSTATUS:
            if (irp->read.bytes == NULL) {
                log_warning(
                    "IOCTL_SERIAL_GET_COMMSTATUS: Output buffer is NULL");

                return E_INVALIDARG;
            }

            if (irp->read.nbytes < sizeof(*status)) {
                log_warning("IOCTL_SERIAL_GET_COMMSTATUS: Buffer is too small");

                return HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
            }

            status = (SERIAL_STATUS *) irp->read.bytes;
            status->Errors = 0;
            status->AmountInInQueue = usbmem_pending ? 1 : 0;

            irp->read.pos = sizeof(*status);

            break;

        default:
            break;
    }

    return S_OK;
}
