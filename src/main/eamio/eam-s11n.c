#include <stdio.h>
#include <stdlib.h>

#include "eamio/eam-impl.h"
#include "eamio/eam-s11n.h"

#include "util/fs.h"

struct eam *eam_impl_config_load(FILE *f)
{
    struct eam *eam;
    struct hid_stub *hid;
    int8_t nunits;
    int8_t unit_no;
    char *card_path;
    char *dev_node;
    bool autogen;
    bool alt_10k;
    bool has_card_path;
    bool has_hid;

    eam = eam_impl_create();

    if (!read_u8(f, &autogen)) {
        goto early_fail;
    }

    if (!read_u8(f, &alt_10k)) {
        goto early_fail;
    }

    eam_impl_set_autogen(eam, autogen);
    eam_impl_set_alt_10k(eam, alt_10k);

    if (!read_u8(f, &nunits) || nunits != EAM_UNIT_COUNT) {
        goto early_fail;
    }

    hid_mgr_lock();

    for (unit_no = 0; unit_no < EAM_UNIT_COUNT; unit_no++) {
        if (!read_u8(f, &has_card_path)) {
            goto late_fail;
        }

        if (!read_u8(f, &has_hid)) {
            goto late_fail;
        }

        if (has_card_path) {
            if (!read_str(f, &card_path)) {
                goto late_fail;
            }

            eam_impl_set_card_path(eam, unit_no, card_path);
            free(card_path);
        }

        if (has_hid) {
            if (!read_str(f, &dev_node)) {
                goto late_fail;
            }

            hid = hid_mgr_get_named_stub(dev_node);
            eam_impl_set_keypad_device(eam, unit_no, hid);

            free(dev_node);
        }
    }

    hid_mgr_unlock();

    return eam;

late_fail:
    hid_mgr_unlock();

early_fail:
    eam_impl_destroy(eam);

    return NULL;
}

void eam_impl_config_save(struct eam *eam, FILE *f)
{
    const char *card_path;
    struct hid_stub *hid;
    uint8_t nunits;
    uint8_t unit_no;
    bool autogen;
    bool alt_10k;
    bool has_card_path;
    bool has_hid;

    autogen = eam_impl_get_autogen(eam);
    alt_10k = eam_impl_get_alt_10k(eam);
    nunits = EAM_UNIT_COUNT;

    write_u8(f, &autogen);
    write_u8(f, &alt_10k);
    write_u8(f, &nunits);

    for (unit_no = 0; unit_no < EAM_UNIT_COUNT; unit_no++) {
        card_path = eam_impl_get_card_path(eam, unit_no);
        hid = eam_impl_get_keypad_device(eam, unit_no);

        has_card_path = card_path != NULL;
        has_hid = hid != NULL;

        write_u8(f, &has_card_path);
        write_u8(f, &has_hid);

        if (has_card_path) {
            write_str(f, card_path);
        }

        if (has_hid) {
            write_str(f, hid_stub_get_dev_node(hid));
        }
    }
}
