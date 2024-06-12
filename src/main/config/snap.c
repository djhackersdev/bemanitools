#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "config/snap.h"

#include "geninput/hid-mgr.h"

#include "iface-core/log.h"

#include "util/defs.h"
#include "util/mem.h"

enum snap_control_heuristic { CONTROL_CENTERING_AXIS, CONTROL_MULTISWITCH };

struct snap_known_control {
    uint32_t usage;
    enum snap_control_heuristic heuristic;
};

static struct snap_known_control snap_known_controls[] = {
    /* X axis */
    {0x00010030, CONTROL_CENTERING_AXIS},

    /* Y axis */
    {0x00010031, CONTROL_CENTERING_AXIS},

    /* Z axis */
    {0x00010032, CONTROL_CENTERING_AXIS},

    /* X rotation */
    {0x00010033, CONTROL_CENTERING_AXIS},

    /* Y rotation */
    {0x00010034, CONTROL_CENTERING_AXIS},

    /* Z rotation */
    {0x00010035, CONTROL_CENTERING_AXIS},

    /* I don't have an adapter that presents a slider/dial/wheel so I don't
       know how to deal with those things right now */

    /* Hat switch */
    {0x00010039, CONTROL_MULTISWITCH}};

static bool snap_check_for_edge(
    const struct hid_control *ctl,
    int32_t val,
    int32_t other_val,
    struct mapped_action *ma);

void snap_init(struct snap *snap)
{
    size_t i;
    size_t j;
    size_t ncontrols;
    size_t ndevs;
    struct hid_stub *pos;

    hid_mgr_lock();

    ndevs = 0;

    for (pos = hid_mgr_get_first_stub(); pos != NULL;
         pos = hid_mgr_get_next_stub(pos)) {
        ndevs++;
    }

    snap->ndevs = ndevs;
    snap->devs = xcalloc(ndevs * sizeof(*snap->devs));

    i = 0;

    for (pos = hid_mgr_get_first_stub(), i = 0; pos != NULL && i < ndevs;
         pos = hid_mgr_get_next_stub(pos), i++) {
        if (!hid_stub_is_attached(pos)) {
            continue;
        }

        if (!hid_stub_get_controls(pos, NULL, &ncontrols)) {
            continue;
        }

        snap->devs[i].hid = pos;
        snap->devs[i].ncontrols = ncontrols;
        snap->devs[i].controls =
            xcalloc(ncontrols * sizeof(struct hid_control));
        snap->devs[i].states = xcalloc(ncontrols * sizeof(int32_t));

        hid_stub_get_controls(pos, snap->devs[i].controls, &ncontrols);

        for (j = 0; j < ncontrols; j++) {
            hid_stub_get_value(pos, j, &snap->devs[i].states[j]);
        }
    }

    hid_mgr_unlock();
}

bool snap_find_edge(
    const struct snap *snap,
    const struct snap *other_snap,
    struct mapped_action *ma)
{
    const struct hid_control *ctl;
    int32_t val;
    int32_t other_val;
    size_t i;
    size_t j;

    /* Most of these checks are to ensure some other device didn't sneak in
       between these two snapshots. If it has, then detecting a state
       transition becomes a bit awkward so we just say that nothing happened
       and wait for the next poll period.

       Yes, creating a total input snapshot 60ish times per second while
       attempting to bind a key isn't exactly the most efficient thing in the
       world. We don't care, it's just a config program. */

    if (snap->ndevs != other_snap->ndevs) {
        return false;
    }

    for (i = 0; i < snap->ndevs; i++) {
        if (snap->devs[i].hid != other_snap->devs[i].hid) {
            return false;
        }

        if (snap->devs[i].ncontrols != other_snap->devs[i].ncontrols) {
            return false;
        }

        if (memcmp(
                snap->devs[i].controls,
                other_snap->devs[i].controls,
                snap->devs[i].ncontrols * sizeof(struct hid_control)) != 0) {
            return false;
        }

        for (j = 0; j < snap->devs[i].ncontrols; j++) {
            ctl = &snap->devs[i].controls[j];
            val = snap->devs[i].states[j];
            other_val = other_snap->devs[i].states[j];

            if (snap_check_for_edge(ctl, val, other_val, ma)) {
                ma->hid = snap->devs[i].hid;
                ma->control_no = j;

                return true;
            }
        }
    }

    return false;
}

static bool snap_check_for_edge(
    const struct hid_control *ctl,
    int32_t val,
    int32_t other_val,
    struct mapped_action *ma)
{
    size_t i;
    int32_t range;
    int32_t dz_min;
    int32_t dz_max;

    if (val != other_val) {
        log_misc("Value change on %08x: %d -> %d", ctl->usage, other_val, val);
    }

    if (ctl->value_max - ctl->value_min == 1) {
        /* Button-like thing, deal with it the obvious way */
        if (val == ctl->value_max && val != other_val) {
            ma->value_min = ctl->value_max;
            ma->value_max = ctl->value_max;

            return true;
        }
    } else {
        /* Here we kinda have to take things on a case by case basis */

        for (i = 0; i < lengthof(snap_known_controls); i++) {
            if (snap_known_controls[i].usage != ctl->usage) {
                continue;
            }

            switch (snap_known_controls[i].heuristic) {
                case CONTROL_CENTERING_AXIS:
                    /* Dead zones keep giving me grief, so I'm going to be
                       super-aggressive and assume a 50%-ish dead zone

                       (yes I know the rounding here is a little iffy) */

                    range = ctl->value_max - ctl->value_min;
                    dz_min = ctl->value_min + ((range * 1) / 4);
                    dz_max = ctl->value_min + ((range * 3) / 4);

                    if (val <= dz_min && other_val > dz_min) {
                        ma->value_min = ctl->value_min;
                        ma->value_max = dz_min;

                        return true;
                    } else if (val >= dz_max && other_val < dz_max) {
                        ma->value_min = dz_max;
                        ma->value_max = ctl->value_max;

                        return true;
                    }

                    return false;

                case CONTROL_MULTISWITCH:
                    /* Assume positions are discrete. Precisely match any
                       transitioned-to value that isn't a null state. */

                    if (val >= ctl->value_min && val <= ctl->value_max &&
                        val != other_val) {
                        ma->value_min = val;
                        ma->value_max = val;

                        return true;
                    }

                    return false;
            }
        }

        /* Still haven't found it? Well, we don't know how to determine if this
           control is being actively pressed or not, so the user will just have
           to manually key in an advanced binding. */
    }

    return false;
}

void snap_fini(struct snap *snap)
{
    size_t i;

    for (i = 0; i < snap->ndevs; i++) {
        free(snap->devs[i].states);
        free(snap->devs[i].controls);
    }

    free(snap->devs);
}
