#define LOG_MODULE "p3iodrv-ddr"

#include "p3io/cmd.h"

#include "util/log.h"

#include "ddr.h"
#include "device.h"

static uint8_t p3iodrv_ddr_seq_counter;

static uint8_t p3iodrv_ddr_get_and_update_seq_counter()
{
    return p3iodrv_ddr_seq_counter++ & 0xF;
}

static HRESULT p3iodrv_ddr_check_resp(
    const union p3io_resp_any *resp,
    uint8_t expected_size,
    const union p3io_req_any *corresponding_req)
{
    uint8_t actual_size;

    log_assert(resp);

    actual_size = p3io_get_full_resp_size(resp);

    if (actual_size != expected_size) {
        log_warning(
            "Incorrect response size, actual %d != expected %d",
            actual_size,
            expected_size);
        return HRESULT_FROM_WIN32(ERROR_BAD_LENGTH);
    }

    if (resp->hdr.seq_no != corresponding_req->hdr.seq_no) {
        log_warning(
            "Incorrect sequence num in response, actual %d != expected %d",
            resp->hdr.seq_no,
            corresponding_req->hdr.seq_no);
        return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
    }

    if (resp->hdr.cmd != corresponding_req->hdr.cmd) {
        log_warning(
            "Incorrect command in response, actual 0x%d != expected 0x%d",
            resp->hdr.cmd,
            corresponding_req->hdr.cmd);
        return HRESULT_FROM_WIN32(ERROR_BAD_COMMAND);
    }

    return S_OK;
}

HRESULT p3iodrv_ddr_init(HANDLE handle)
{
    HRESULT hr;
    uint8_t seq_cnt;
    union p3io_req_any req;
    union p3io_resp_any resp;

    log_assert(handle != INVALID_HANDLE_VALUE);

    seq_cnt = p3iodrv_ddr_get_and_update_seq_counter();

    p3io_req_hdr_init(&req.hdr, seq_cnt, P3IO_CMD_INIT, sizeof(req.init));

    hr = p3iodrv_device_transfer(handle, &req, &resp);

    if (FAILED(hr)) {
        return hr;
    }

    hr = p3iodrv_ddr_check_resp(&resp, sizeof(resp.init), &req);

    if (FAILED(hr)) {
        return hr;
    }

    if (resp.init.status != 0) {
        log_warning("Initialization failed");
        return HRESULT_FROM_WIN32(ERROR_GEN_FAILURE);
    }

    return S_OK;
}

HRESULT p3iodrv_ddr_get_version(
    HANDLE handle,
    char str[4],
    uint32_t *major,
    uint32_t *minor,
    uint32_t *patch)
{
    HRESULT hr;
    uint8_t seq_cnt;
    union p3io_req_any req;
    union p3io_resp_any resp;

    log_assert(handle != INVALID_HANDLE_VALUE);
    log_assert(major);
    log_assert(minor);
    log_assert(patch);

    seq_cnt = p3iodrv_ddr_get_and_update_seq_counter();

    p3io_req_hdr_init(
        &req.hdr, seq_cnt, P3IO_CMD_GET_VERSION, sizeof(req.version));

    hr = p3iodrv_device_transfer(handle, &req, &resp);

    if (FAILED(hr)) {
        return hr;
    }

    hr = p3iodrv_ddr_check_resp(&resp, sizeof(resp.version), &req);

    if (FAILED(hr)) {
        return hr;
    }

    memcpy(str, resp.version.str, sizeof(char[4]));
    *major = resp.version.major;
    *minor = resp.version.minor;
    *patch = resp.version.patch;

    return S_OK;
}

HRESULT p3iodrv_ddr_set_watchdog(HANDLE handle, bool enable)
{
    HRESULT hr;
    uint8_t seq_cnt;
    union p3io_req_any req;
    union p3io_resp_any resp;

    log_assert(handle != INVALID_HANDLE_VALUE);

    seq_cnt = p3iodrv_ddr_get_and_update_seq_counter();

    p3io_req_hdr_init(
        &req.hdr, seq_cnt, P3IO_CMD_SET_WATCHDOG, sizeof(req.watchdog));

    req.watchdog.enable = enable ? 1 : 0;

    hr = p3iodrv_device_transfer(handle, &req, &resp);

    if (FAILED(hr)) {
        return hr;
    }

    hr = p3iodrv_ddr_check_resp(&resp, sizeof(resp.watchdog), &req);

    if (FAILED(hr)) {
        return hr;
    }

    return S_OK;
}

HRESULT p3iodrv_ddr_get_dipsw(HANDLE handle, uint8_t *state)
{
    HRESULT hr;
    uint8_t seq_cnt;
    union p3io_req_any req;
    union p3io_resp_any resp;

    log_assert(handle != INVALID_HANDLE_VALUE);
    log_assert(state);

    seq_cnt = p3iodrv_ddr_get_and_update_seq_counter();

    p3io_req_hdr_init(
        &req.hdr,
        seq_cnt,
        P3IO_CMD_GET_CAB_TYPE_OR_DIPSW,
        sizeof(req.cab_type_or_dipsw));

    req.cab_type_or_dipsw.cab_type_or_dipsw = P3IO_DIP_SW_SELECTOR;

    hr = p3iodrv_device_transfer(handle, &req, &resp);

    if (FAILED(hr)) {
        return hr;
    }

    hr = p3iodrv_ddr_check_resp(&resp, sizeof(resp.cab_type_or_dipsw), &req);

    if (FAILED(hr)) {
        return hr;
    }

    *state = resp.cab_type_or_dipsw.state;

    return S_OK;
}

HRESULT p3iodrv_ddr_get_cab_type(HANDLE handle, enum p3io_cab_type *type)
{
    HRESULT hr;
    uint8_t seq_cnt;
    union p3io_req_any req;
    union p3io_resp_any resp;

    log_assert(handle != INVALID_HANDLE_VALUE);
    log_assert(type);

    seq_cnt = p3iodrv_ddr_get_and_update_seq_counter();

    p3io_req_hdr_init(
        &req.hdr,
        seq_cnt,
        P3IO_CMD_GET_CAB_TYPE_OR_DIPSW,
        sizeof(req.cab_type_or_dipsw));

    req.cab_type_or_dipsw.cab_type_or_dipsw = P3IO_CAB_TYPE_SELECTOR;

    hr = p3iodrv_device_transfer(handle, &req, &resp);

    if (FAILED(hr)) {
        return hr;
    }

    hr = p3iodrv_ddr_check_resp(&resp, sizeof(resp.cab_type_or_dipsw), &req);

    if (FAILED(hr)) {
        return hr;
    }

    *type = resp.cab_type_or_dipsw.state;

    return S_OK;
}

HRESULT p3iodrv_ddr_get_video_freq(HANDLE handle, enum p3io_video_freq *freq)
{
    HRESULT hr;
    uint8_t seq_cnt;
    union p3io_req_any req;
    union p3io_resp_any resp;

    log_assert(handle != INVALID_HANDLE_VALUE);
    log_assert(freq);

    seq_cnt = p3iodrv_ddr_get_and_update_seq_counter();

    p3io_req_hdr_init(
        &req.hdr, seq_cnt, P3IO_CMD_GET_VIDEO_FREQ, sizeof(req.video_freq));

    // Must be set to 5 in order to return the right values. There might be more
    // to this, e.g. this being some kind of selector where to read from the
    // JAMMA harness (?) but this is good for now to get what we want to know
    // for DDR
    req.video_freq.unknown_05 = 5;

    hr = p3iodrv_device_transfer(handle, &req, &resp);

    if (FAILED(hr)) {
        return hr;
    }

    hr = p3iodrv_ddr_check_resp(&resp, sizeof(resp.video_freq), &req);

    if (FAILED(hr)) {
        return hr;
    }

    *freq = (enum p3io_video_freq) resp.video_freq.video_freq;

    return S_OK;
}

HRESULT p3iodrv_ddr_get_jamma(HANDLE handle, struct p3io_ddr_jamma *jamma)
{
    HRESULT hr;
    uint32_t *jamma_raw;

    jamma_raw = (uint32_t *) jamma;

    hr = p3iodrv_device_read_jamma(handle, jamma_raw);

    if (FAILED(hr)) {
        return hr;
    }

    // Inputs are active low for p1, p2 and operator bits
    jamma_raw[0] ^= 0xFFFFFF00;

    return S_OK;
}

HRESULT
p3iodrv_ddr_set_outputs(HANDLE handle, const struct p3io_ddr_output *state)
{
    HRESULT hr;
    uint8_t seq_cnt;
    union p3io_req_any req;
    union p3io_resp_any resp;

    log_assert(handle != INVALID_HANDLE_VALUE);
    log_assert(state);

    seq_cnt = p3iodrv_ddr_get_and_update_seq_counter();

    p3io_req_hdr_init(
        &req.hdr, seq_cnt, P3IO_CMD_SET_OUTPUTS, sizeof(req.set_outputs));

    memcpy(&req.set_outputs.outputs, state, sizeof(req.set_outputs.outputs));

    // Always set by the game like this
    req.set_outputs.unk_FF = 0xFF;

    hr = p3iodrv_device_transfer(handle, &req, &resp);

    if (FAILED(hr)) {
        return hr;
    }

    hr = p3iodrv_ddr_check_resp(&resp, sizeof(resp.set_outputs), &req);

    if (FAILED(hr)) {
        return hr;
    }

    return S_OK;
}