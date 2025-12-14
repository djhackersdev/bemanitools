#include <windows.h>

#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bemanitools/eamio.h"

#include "eamio/eam-impl.h"
#include "eamio/eam-scard.h"

#include "geninput/hid-mgr.h"
#include "geninput/kbd-naive.h"

#include "util/defs.h"
#include "util/fs.h"
#include "util/hex.h"
#include "util/log.h"
#include "util/mem.h"
#include "util/str.h"

#define EAM_SENSOR_COOLDOWN 1000

struct eam_unit {
    char *card_path;
    struct hid_stub *hid;
    size_t keypad_ctls[EAM_IO_KEYPAD_COUNT];
    size_t sensor_ctl;
    bool bound_ctls;
    uint8_t drive_no;
    uint32_t sensor_time;
    bool sensor_hot;
    bool alt_10k;
};

struct eam {
    /* This lock protects drive_no and sensor_time, which get read/written
       respectively by eam_impl_notify_hotplug, which gets called by the Win32
       message pump thread. */

    CRITICAL_SECTION lock;
    struct eam_unit units[EAM_UNIT_COUNT];
    bool autogen;
    bool scard;
    bool mux_keypad;
    bool mux_scard;
};

static const uint32_t eam_keypad_usages[EAM_IO_KEYPAD_COUNT + 1] = {
    /* [EAM_KEYPAD_0]           = */ 0x00070062,
    /* [EAM_KEYPAD_1]           = */ 0x00070059,
    /* [EAM_KEYPAD_4]           = */ 0x0007005C,
    /* [EAM_KEYPAD_7]           = */ 0x0007005F,
    /* [EAM_KEYPAD_00]          = */ 0x00070058, /* Keypad ENTER */
    /* [EAM_KEYPAD_2]           = */ 0x0007005A,
    /* [EAM_KEYPAD_5]           = */ 0x0007005D,
    /* [EAM_KEYPAD_8]           = */ 0x00070060,
    /* [EAM_KEYPAD_DECIMAL]     = */ 0x00070063,
    /* [EAM_KEYPAD_3]           = */ 0x0007005B,
    /* [EAM_KEYPAD_6]           = */ 0x0007005E,
    /* [EAM_KEYPAD_9]           = */ 0x00070061,
    /* Sensor                   = */ 0x00070057};

static const uint32_t eam_keypad_usages_alt[EAM_IO_KEYPAD_COUNT + 1] = {
    /* [EAM_KEYPAD_0]           = */ 0x00070027,
    /* [EAM_KEYPAD_1]           = */ 0x0007001E,
    /* [EAM_KEYPAD_4]           = */ 0x00070021,
    /* [EAM_KEYPAD_7]           = */ 0x00070024,
    /* [EAM_KEYPAD_00]          = */ 0x0007002D, /* - and _ */
    /* [EAM_KEYPAD_2]           = */ 0x0007001F,
    /* [EAM_KEYPAD_5]           = */ 0x00070022,
    /* [EAM_KEYPAD_8]           = */ 0x00070025,
    /* [EAM_KEYPAD_DECIMAL]     = */ 0x0007002E, /* + and = */
    /* [EAM_KEYPAD_3]           = */ 0x00070020,
    /* [EAM_KEYPAD_6]           = */ 0x00070023,
    /* [EAM_KEYPAD_9]           = */ 0x00070026,
    /* Sensor                   = */ 0x0007002A /* Backspace */
};

static bool eam_impl_is_active_unit(struct eam *eam, uint8_t unit_no);
static void eam_impl_bind_keypad(struct eam *eam, uint8_t unit_no);
static bool eam_impl_autogen(struct eam_unit *unit, uint8_t *card_id);

struct eam *eam_impl_create(void)
{
    struct eam *eam;
    struct eam_unit *unit;
    size_t btn_no;
    uint8_t unit_no;

    eam = xmalloc(sizeof(*eam));

    InitializeCriticalSection(&eam->lock);

    for (unit_no = 0; unit_no < lengthof(eam->units); unit_no++) {
        unit = &eam->units[unit_no];

        unit->card_path = NULL;
        unit->hid = NULL;
        unit->alt_10k = false;

        for (btn_no = 0; btn_no < lengthof(unit->keypad_ctls); btn_no++) {
            unit->keypad_ctls[btn_no] = (size_t) -1;
        }

        unit->sensor_ctl = (size_t) -1;
        unit->bound_ctls = false;
        unit->drive_no = (uint8_t) -1;
        unit->sensor_time = 0;
    }

    eam->autogen = false;
    eam->scard = false;
    eam->mux_keypad = false;
    eam->mux_scard = false;

    return eam;
}

static bool eam_impl_is_active_unit(struct eam *eam, uint8_t unit_no)
{
    log_assert(lengthof(eam->units) == 2);

    if (eam->mux_keypad && eam_impl_do_keypads_conflict(eam)) {
        // Determine active unit by numlock state
        return (GetKeyState(VK_NUMLOCK) & 0x0001) == unit_no;
    } else {
        return true;
    }
}

bool eam_impl_get_scard(struct eam *eam)
{
    return eam->scard;
}
void eam_impl_set_scard(struct eam *eam, bool scard)
{
    eam->scard = scard != false;

    if (eam->scard) {
        eam_scard_init(eam->mux_scard);
    } else {
        eam_scard_fini();
    }
}

bool eam_impl_get_mux_keypad(struct eam *eam)
{
    return eam->mux_keypad;
}
void eam_impl_set_mux_keypad(struct eam *eam, bool mux_keypad)
{
    eam->mux_keypad = mux_keypad != false;
}

bool eam_impl_get_mux_scard(struct eam *eam)
{
    return eam->mux_scard;
}
void eam_impl_set_mux_scard(struct eam *eam, bool mux_scard)
{
    eam->mux_scard = mux_scard != false;

    if (eam->scard) {
        // reinit so we can remap the readers
        eam_scard_fini();
        eam_scard_init(eam->mux_scard);
    }
}

bool eam_impl_get_autogen(struct eam *eam)
{
    return eam->autogen;
}

void eam_impl_set_autogen(struct eam *eam, bool autogen)
{
    eam->autogen = autogen != false;
}

bool eam_impl_do_keypads_conflict(struct eam *eam)
{
    log_assert(lengthof(eam->units) == 2);

    // 1. To conflict, both must have an assigned hid device
    if (eam->units[0].hid == NULL || eam->units[1].hid == NULL) {
        return false;
    }

    // 2. To conflict, both must use the same keys (i.e. numpad vs top row)
    if (eam->units[0].alt_10k != eam->units[1].alt_10k) {
        // keypads are not using the same keys. mux not necessary
        return false;
    }

    // 3. To conflict, the hids must be the same or either be the naive kbd
    return (eam->units[0].hid == eam->units[1].hid ||
            kbd_is_naive(eam->units[0].hid) || 
            kbd_is_naive(eam->units[1].hid));
}

bool eam_impl_get_alt_10k(struct eam *eam, uint8_t unit_no)
{
    log_assert(unit_no < lengthof(eam->units));

    return eam->units[unit_no].alt_10k;
}

void eam_impl_set_alt_10k(struct eam *eam, uint8_t unit_no, bool alt_10k)
{
    log_assert(unit_no < lengthof(eam->units));

    eam->units[unit_no].bound_ctls = false;
    eam->units[unit_no].alt_10k = alt_10k != false;
}

struct hid_stub *eam_impl_get_keypad_device(struct eam *eam, uint8_t unit_no)
{
    log_assert(unit_no < lengthof(eam->units));

    return eam->units[unit_no].hid;
}

void eam_impl_set_keypad_device(
    struct eam *eam, uint8_t unit_no, struct hid_stub *hid)
{
    log_assert(unit_no < lengthof(eam->units));

    eam->units[unit_no].hid = hid;
    eam->units[unit_no].bound_ctls = false;
}

const char *eam_impl_get_card_path(struct eam *eam, uint8_t unit_no)
{
    log_assert(unit_no < lengthof(eam->units));

    return eam->units[unit_no].card_path;
}

void eam_impl_set_card_path(struct eam *eam, uint8_t unit_no, const char *path)
{
    log_assert(unit_no < lengthof(eam->units));

    EnterCriticalSection(&eam->lock);

    free(eam->units[unit_no].card_path);

    if (path != NULL) {
        eam->units[unit_no].card_path = str_dup(path);

        if (isalpha(path[0]) && path[1] == ':') {
            eam->units[unit_no].drive_no = toupper(path[0]) - 'A';
        } else {
            eam->units[unit_no].drive_no = (uint8_t) -1;
        }
    } else {
        eam->units[unit_no].card_path = NULL;
        eam->units[unit_no].drive_no = (uint8_t) -1;
    }

    LeaveCriticalSection(&eam->lock);
}

uint16_t eam_impl_get_keypad_state(struct eam *eam, uint8_t unit_no)
{
    struct eam_unit *unit;
    uint16_t result;
    int32_t value;
    size_t i;

    log_assert(unit_no < lengthof(eam->units));

    if (!eam_impl_is_active_unit(eam, unit_no)) {
        return 0;
    }

    eam_impl_bind_keypad(eam, unit_no);

    unit = &eam->units[unit_no];
    result = 0;

    if (unit->bound_ctls) {
        for (i = 0; i < lengthof(unit->keypad_ctls); i++) {
            if (unit->keypad_ctls[i] == (size_t) -1) {
                continue;
            }

            if (!hid_stub_get_value(unit->hid, unit->keypad_ctls[i], &value)) {
                continue;
            }

            if (value) {
                result |= 1 << i;
            }
        }
    }

    return result;
}

static void eam_impl_bind_keypad(struct eam *eam, uint8_t unit_no)
{
    const uint32_t *usages;
    struct hid_control *controls;
    struct eam_unit *unit;
    size_t ncontrols;
    size_t control_no;
    size_t btn_no;

    unit = &eam->units[unit_no];

    if (unit->bound_ctls || unit->hid == NULL ||
        !hid_stub_is_attached(unit->hid)) {
        return;
    }

    if (eam->units[unit_no].alt_10k) {
        usages = eam_keypad_usages_alt;
    } else {
        usages = eam_keypad_usages;
    }

    /* Set flag first: don't try again if this fails for whatever reason. */

    unit->bound_ctls = true;

    hid_mgr_lock();

    if (!hid_stub_get_controls(unit->hid, NULL, &ncontrols)) {
        goto size_fail;
    }

    controls = xmalloc(ncontrols * sizeof(*controls));

    if (!hid_stub_get_controls(unit->hid, controls, &ncontrols)) {
        goto content_fail;
    }

    for (control_no = 0; control_no < ncontrols; control_no++) {
        for (btn_no = 0; btn_no < EAM_IO_KEYPAD_COUNT; btn_no++) {
            if (controls[control_no].usage == usages[btn_no]) {
                unit->keypad_ctls[btn_no] = control_no;
            }
        }

        if (controls[control_no].usage == usages[EAM_IO_KEYPAD_COUNT]) {
            unit->sensor_ctl = control_no;
        }
    }

    free(controls);

    hid_mgr_unlock();

    return;

content_fail:
    free(controls);

size_fail:
    hid_mgr_unlock();
}

bool eam_impl_get_sensor_state(struct eam *eam, uint8_t unit_no)
{
    struct eam_unit *unit;
    uint32_t now;
    int32_t value;
    bool result;

    log_assert(unit_no < lengthof(eam->units));

    unit = &eam->units[unit_no];
    result = false;

    eam_impl_bind_keypad(eam, unit_no);

    if (!unit->bound_ctls) {
        return false;
    }

    if (unit->sensor_ctl == (size_t) -1) {
        return false;
    }

    if (!hid_stub_get_value(unit->hid, unit->sensor_ctl, &value)) {
        return false;
    }

    now = GetTickCount();

    EnterCriticalSection(&eam->lock);

    /* Bump cooldown as long as sensor button is held
       (cooldown timer might also be set by a USB hotplug event) */

    if (value != 0 && eam_impl_is_active_unit(eam, unit_no)) {
        unit->sensor_time = now + EAM_SENSOR_COOLDOWN;
        unit->sensor_hot = true;
    }

    if (unit->sensor_hot) {
        if ((int32_t) (unit->sensor_time - now) > 0) {
            result = true;
        } else {
            unit->sensor_time = 0;
            unit->sensor_hot = false;

            result = false;
        }
    }

    LeaveCriticalSection(&eam->lock);

    return result;
}

uint8_t eam_impl_read_card(
    struct eam *eam, uint8_t unit_no, uint8_t *card_id, uint8_t nbytes)
{
    char line[128];
    struct eam_unit *unit;
    size_t len;
    FILE *f;

    log_assert(unit_no < lengthof(eam->units));
    log_assert(card_id != NULL);
    log_assert(nbytes == EAM_CARD_NBYTES);

    unit = &eam->units[unit_no];

    if (unit->card_path == NULL) {
        goto path_fail;
    }

    f = fopen(unit->card_path, "r");

    if (f == NULL) {
        if (eam->autogen) {
            if (!eam_impl_autogen(unit, card_id)) {
                log_warning(
                    "Unit %d: Failed to generate card ID into %s",
                    unit_no,
                    unit->card_path);

                goto fopen_fail;
            }

            return true;
        } else {
            log_warning(
                "Unit %d: Card file at %s not present",
                unit_no,
                unit->card_path);

            goto fopen_fail;
        }
    }

    if (fgets(line, sizeof(line), f) == NULL) {
        log_warning("%s: fgets() failed", unit->card_path);

        goto fgets_fail;
    }

    str_trim(line);
    len = strlen(line);

    if (len != 2 * EAM_CARD_NBYTES) {
        log_warning(
            "%s: Expected %u chars (got %u)",
            unit->card_path,
            2 * EAM_CARD_NBYTES,
            (unsigned int) len);

        goto len_fail;
    }

    if (!hex_decode(card_id, nbytes, line, len)) {
        log_warning("%s: Invalid hex [%s]", unit->card_path, card_id);

        goto decode_fail;
    }

    log_misc(
        "Unit %d: Loaded card ID [%s] from file %s",
        unit_no,
        line,
        unit->card_path);

    fclose(f);

    if (card_id[0] == 0xe0 && card_id[1] == 0x04) {
        return EAM_IO_CARD_ISO15696;
    } else {
        return EAM_IO_CARD_FELICA;
    }

decode_fail:
len_fail:
fgets_fail:
    fclose(f);

fopen_fail:
path_fail:
    return EAM_IO_CARD_NONE;
}

static bool eam_impl_autogen(struct eam_unit *unit, uint8_t *card_id)
{
    char hex[2 * EAM_CARD_NBYTES + 1];
    FILE *f;
    size_t i;

    f = fopen(unit->card_path, "w");

    if (f == NULL) {
        return false;
    }

    srand(GetTickCount());

    card_id[0] = 0xE0;
    card_id[1] = 0x04;
    card_id[2] = 0x01;
    card_id[3] = 0x00;

    for (i = 4; i < 8; i++) {
        /* LSBit entropy of typical LFSR RNGs is usually poor */
        card_id[i] = rand() >> 7;
    }

    hex_encode_uc(card_id, EAM_CARD_NBYTES, hex, sizeof(hex));
    fwrite(hex, sizeof(hex) - 1, 1, f);

    fclose(f);

    log_info(
        "Generated random card ID [%s] into file %s", hex, unit->card_path);

    return true;
}

void eam_impl_notify_hotplug(struct eam *eam, uint8_t drive_no)
{
    struct eam_unit *unit;
    uint8_t unit_no;

    EnterCriticalSection(&eam->lock);

    for (unit_no = 0; unit_no < lengthof(eam->units); unit_no++) {
        if (eam->units[unit_no].drive_no == drive_no) {
            /* MMSYSTEM timeGetTime() is overkill, we don't exactly need super
               accurate timestamps here. */

            unit = &eam->units[unit_no];

            unit->sensor_time = GetTickCount() + EAM_SENSOR_COOLDOWN;
            unit->sensor_hot = true;
        }
    }

    LeaveCriticalSection(&eam->lock);
}

void eam_impl_destroy(struct eam *eam)
{
    int8_t unit_no;

    for (unit_no = lengthof(eam->units) - 1; unit_no >= 0; unit_no--) {
        free(eam->units[unit_no].card_path);
    }

    DeleteCriticalSection(&eam->lock);

    free(eam);
}
