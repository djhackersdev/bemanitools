#include <stdlib.h>

#include "geninput/mapper-s11n.h"
#include "geninput/mapper.h"

#include "util/fs.h"
#include "util/mem.h"

static bool mapper_impl_config_load_actions(struct mapper *m, FILE *f);
static bool mapper_impl_config_load_analogs(struct mapper *m, FILE *f);
static bool mapper_impl_config_load_lights(struct mapper *m, FILE *f);
static void mapper_impl_config_save_actions(struct mapper *m, FILE *f);
static void mapper_impl_config_save_analogs(struct mapper *m, FILE *f);
static void mapper_impl_config_save_lights(struct mapper *m, FILE *f);

struct mapper *mapper_impl_config_load(FILE *f)
{
    struct mapper *m;

    hid_mgr_lock(); /* Need to convert from HID objects to /dev nodes */

    m = mapper_impl_create();

    if (!mapper_impl_config_load_actions(m, f) ||
        !mapper_impl_config_load_analogs(m, f) ||
        !mapper_impl_config_load_lights(m, f)) {
        mapper_impl_destroy(m);
        m = NULL;
    }

    hid_mgr_unlock();

    return m;
}

static bool mapper_impl_config_load_actions(struct mapper *m, FILE *f)
{
    char *dev_node;
    struct mapped_action ma;
    uint8_t action;
    uint8_t page;
    uint8_t bit;
    uint32_t count;
    uint32_t i;

    if (!read_u32(f, &count)) {
        return false;
    }

    for (i = 0; i < count; i++) {
        memset(&ma, 0, sizeof(ma));

        if (!read_str(f, &dev_node)) {
            return false;
        }

        ma.hid = hid_mgr_get_named_stub(dev_node);
        free(dev_node);

        if (!read_u32(f, &ma.control_no) || !read_u32(f, &ma.value_min) ||
            !read_u32(f, &ma.value_max) || !read_u8(f, &action) ||
            !read_u8(f, &page) || !read_u8(f, &bit)) {
            return false;
        }

        mapper_impl_set_action_map(m, action, page, bit, &ma);
    }

    return true;
}

static bool mapper_impl_config_load_analogs(struct mapper *m, FILE *f)
{
    char *dev_node;
    struct mapped_analog ma;
    int32_t sensitivity;
    uint8_t nanalogs;
    uint8_t i;

    if (!read_u8(f, &nanalogs)) {
        return false;
    }

    mapper_impl_set_nanalogs(m, nanalogs);

    for (i = 0; i < nanalogs; i++) {
        memset(&ma, 0, sizeof(ma));

        if (!read_str(f, &dev_node)) {
            return false;
        }

        if (dev_node[0] != '\0') {
            ma.hid = hid_mgr_get_named_stub(dev_node);

            if (!read_u32(f, &ma.control_no)) {
                return false;
            }

            if (!read_u32(f, &sensitivity)) {
                return false;
            }

            mapper_impl_set_analog_map(m, i, &ma);
            mapper_impl_set_analog_sensitivity(m, i, sensitivity);
        }

        free(dev_node);
    }

    return true;
}

static bool mapper_impl_config_load_lights(struct mapper *m, FILE *f)
{
    char *dev_node;
    struct mapped_light ml;
    uint8_t game_light;
    uint8_t nlights;
    uint32_t count;
    uint32_t i;

    if (!read_u8(f, &nlights)) {
        return false;
    }

    mapper_impl_set_nlights(m, nlights);

    if (!read_u32(f, &count)) {
        return false;
    }

    for (i = 0; i < count; i++) {
        memset(&ml, 0, sizeof(ml));

        if (!read_str(f, &dev_node)) {
            return false;
        }

        ml.hid = hid_mgr_get_named_stub(dev_node);
        free(dev_node);

        if (!read_u32(f, &ml.light_no)) {
            return false;
        }

        if (!read_u8(f, &game_light)) {
            return false;
        }

        mapper_impl_set_light_map(m, &ml, game_light);
    }

    return true;
}

void mapper_impl_config_save(struct mapper *m, FILE *f)
{
    hid_mgr_lock();

    mapper_impl_config_save_actions(m, f);
    mapper_impl_config_save_analogs(m, f);
    mapper_impl_config_save_lights(m, f);

    hid_mgr_unlock();
}

static void mapper_impl_config_save_actions(struct mapper *m, FILE *f)
{
    action_iter_t pos;
    struct mapped_action ma;
    uint32_t count;
    uint8_t action;
    uint8_t page;
    uint8_t bit;

    /* Count up bindings */

    count = 0;

    for (pos = mapper_impl_iterate_actions(m); action_iter_is_valid(pos);
         action_iter_next(pos)) {
        count++;
    }

    action_iter_free(pos);
    write_u32(f, &count);

    /* Write out bindings */

    for (pos = mapper_impl_iterate_actions(m); action_iter_is_valid(pos);
         action_iter_next(pos)) {
        action_iter_get_mapping(pos, &ma);
        action = action_iter_get_action(pos);
        page = action_iter_get_page(pos);
        bit = action_iter_get_bit(pos);

        write_str(f, hid_stub_get_dev_node(ma.hid));
        write_u32(f, &ma.control_no);
        write_u32(f, &ma.value_min);
        write_u32(f, &ma.value_max);
        write_u8(f, &action);
        write_u8(f, &page);
        write_u8(f, &bit);
    }

    action_iter_free(pos);
}

static void mapper_impl_config_save_analogs(struct mapper *m, FILE *f)
{
    struct mapped_analog ma;
    int32_t sensitivity;
    uint8_t nanalogs;
    uint8_t i;

    nanalogs = mapper_impl_get_nanalogs(m);
    write_u8(f, &nanalogs);

    for (i = 0; i < nanalogs; i++) {
        mapper_impl_get_analog_map(m, i, &ma);

        if (ma.hid == NULL) {
            write_str(f, "");
        } else {
            sensitivity = mapper_impl_get_analog_sensitivity(m, i);

            write_str(f, hid_stub_get_dev_node(ma.hid));
            write_u32(f, &ma.control_no);
            write_u32(f, &sensitivity);
        }
    }
}

static void mapper_impl_config_save_lights(struct mapper *m, FILE *f)
{
    light_iter_t pos;
    struct mapped_light ml;
    uint8_t game_light;
    uint8_t nlights;
    uint32_t count;

    nlights = mapper_impl_get_nlights(m);
    write_u8(f, &nlights);

    count = 0;

    for (pos = mapper_impl_iterate_lights(m); light_iter_is_valid(pos);
         light_iter_next(pos)) {
        count++;
    }

    write_u32(f, &count);
    light_iter_free(pos);

    for (pos = mapper_impl_iterate_lights(m); light_iter_is_valid(pos);
         light_iter_next(pos)) {
        light_iter_get_mapping(pos, &ml);
        game_light = light_iter_get_game_light(pos);

        write_str(f, hid_stub_get_dev_node(ml.hid));
        write_u32(f, &ml.light_no);
        write_u8(f, &game_light);
    }

    light_iter_free(pos);
}
