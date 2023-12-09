#include <math.h>
#include <stdbool.h>

#include "geninput/hid-mgr.h"
#include "geninput/mapper.h"

#include "util/array.h"
#include "util/log.h"
#include "util/mem.h"

struct action_iter {
    struct mapper *mapper;
    size_t i;
};

struct action_mapping {
    struct mapped_action src;
    uint8_t action;
    uint8_t page;
    uint8_t bit;
};

struct analog_mapping {
    struct mapped_analog src;
    int32_t sensitivity;
    bool bound;
    bool valid;
    bool absolute;
    bool invert;
    double analog_min;
    double analog_max;
    double inv_analog_range;
    uint8_t pos;
};

struct light_iter {
    struct mapper *mapper;
    size_t i;
};

struct light_mapping {
    struct mapped_light dest;
    uint8_t game_light;
    bool bound;
    bool valid;
    double affine_scale;
    int32_t affine_bias;
};

struct mapper {
    struct array actions;
    struct analog_mapping *analogs;
    uint8_t nanalogs;
    struct array light_maps;
    uint8_t *lights;
    uint8_t nlights;
};

static uint64_t action_mapping_update(struct action_mapping *am);
static void analog_mapping_bind(struct analog_mapping *am);
static void analog_mapping_update(struct analog_mapping *am);
static void light_mapping_bind(struct light_mapping *lm);
static void
light_mapping_send(struct light_mapping *lm, const struct mapper *m);

struct mapper *mapper_inst;

void action_iter_get_mapping(action_iter_t iter, struct mapped_action *ma)
{
    struct action_mapping *am;

    log_assert(action_iter_is_valid(iter));

    am = array_item(struct action_mapping, &iter->mapper->actions, iter->i);
    *ma = am->src;
}

uint8_t action_iter_get_action(action_iter_t iter)
{
    struct action_mapping *am;

    log_assert(action_iter_is_valid(iter));

    am = array_item(struct action_mapping, &iter->mapper->actions, iter->i);

    return am->action;
}

uint8_t action_iter_get_page(action_iter_t iter)
{
    struct action_mapping *am;

    log_assert(action_iter_is_valid(iter));

    am = array_item(struct action_mapping, &iter->mapper->actions, iter->i);

    return am->page;
}

uint8_t action_iter_get_bit(action_iter_t iter)
{
    struct action_mapping *am;

    log_assert(action_iter_is_valid(iter));

    am = array_item(struct action_mapping, &iter->mapper->actions, iter->i);

    return am->bit;
}

bool action_iter_is_valid(action_iter_t iter)
{
    return iter->i < iter->mapper->actions.nitems;
}

void action_iter_next(action_iter_t iter)
{
    iter->i++;
}

void action_iter_free(action_iter_t iter)
{
    free(iter);
}

static uint64_t action_mapping_update(struct action_mapping *am)
{
    int32_t value;

    if (am->src.hid == NULL) {
        return 0;
    }

    if (!hid_stub_get_value(am->src.hid, am->src.control_no, &value)) {
        return 0;
    }

    if (value < am->src.value_min || value > am->src.value_max) {
        return 0;
    }

    return 1ULL << am->bit;
}

static void analog_mapping_bind(struct analog_mapping *am)
{
    const struct hid_control *ctl;
    struct hid_control *controls;
    size_t ncontrols;

    am->bound = true;
    am->valid = false;

    if (am->src.hid == NULL) {
        goto unbound_fail;
    }

    if (!hid_stub_get_controls(am->src.hid, NULL, &ncontrols)) {
        goto size_fail;
    }

    if (am->src.control_no >= ncontrols) {
        goto bounds_fail;
    }

    controls = xmalloc(sizeof(*controls) * ncontrols);

    if (!hid_stub_get_controls(am->src.hid, controls, &ncontrols)) {
        goto read_fail;
    }

    ctl = &controls[am->src.control_no];

    am->analog_min = ctl->value_min;
    am->analog_max = ctl->value_max;
    am->inv_analog_range = 1.0 / ((int64_t)ctl->value_max - ctl->value_min);
    am->absolute = !(ctl->flags & HID_FLAG_RELATIVE);
    am->valid = true;

read_fail:
    free(controls);

bounds_fail:
size_fail:
unbound_fail:
    return;
}

static void analog_mapping_update(struct analog_mapping *am)
{
    double tmp;
    int32_t value;
    int8_t delta;

    if (!am->bound) {
        analog_mapping_bind(am);
    }

    if (!am->valid) {
        return;
    }

    if (!hid_stub_get_value(am->src.hid, am->src.control_no, &value)) {
        return;
    }

    if (am->absolute) {

        if (am->invert) {
            tmp = am->analog_max - value;
        } else {
            tmp = value - am->analog_min;
        }

        // Scale the input value to [0,1] range
        tmp *= am->inv_analog_range;
        am->pos = (uint8_t) ((tmp + 0.5) * 256.0);
    } else {
        delta = (int8_t) (value * exp(am->sensitivity / 256.0));

        if (am->invert) {
            delta *= -1;
        }

        am->pos += delta;
    }
}

void light_iter_get_mapping(light_iter_t iter, struct mapped_light *ml)
{
    struct light_mapping *lm;

    log_assert(light_iter_is_valid(iter));

    lm = array_item(struct light_mapping, &iter->mapper->light_maps, iter->i);
    *ml = lm->dest;
}

uint8_t light_iter_get_game_light(light_iter_t iter)
{
    struct light_mapping *lm;

    log_assert(light_iter_is_valid(iter));

    lm = array_item(struct light_mapping, &iter->mapper->light_maps, iter->i);

    return lm->game_light;
}

bool light_iter_is_valid(light_iter_t iter)
{
    return iter->i < iter->mapper->light_maps.nitems;
}

void light_iter_next(light_iter_t iter)
{
    iter->i++;
}

void light_iter_free(light_iter_t iter)
{
    free(iter);
}

static void light_mapping_bind(struct light_mapping *lm)
{
    const struct hid_light *light;
    struct hid_light *light_defs;
    size_t nlights;

    lm->bound = true;
    lm->valid = false;

    if (lm->dest.hid == NULL) {
        goto unbound_fail;
    }

    if (!hid_stub_get_lights(lm->dest.hid, NULL, &nlights)) {
        goto size_fail;
    }

    if (lm->dest.light_no >= nlights) {
        goto bounds_fail;
    }

    light_defs = xmalloc(sizeof(*light_defs) * nlights);

    if (!hid_stub_get_lights(lm->dest.hid, light_defs, &nlights)) {
        goto read_fail;
    }

    light = &light_defs[lm->dest.light_no];

    lm->affine_bias = light->value_min;
    lm->affine_scale = light->value_max - light->value_min;
    lm->valid = true;

read_fail:
    free(light_defs);

bounds_fail:
size_fail:
unbound_fail:
    return;
}

static void light_mapping_send(struct light_mapping *lm, const struct mapper *m)
{
    double tmp;
    uint32_t value;
    uint8_t intensity;

    if (!lm->bound) {
        light_mapping_bind(lm);
    }

    if (!lm->valid) {
        return;
    }

    if (lm->game_light >= m->nlights) {
        return;
    }

    intensity = m->lights[lm->game_light];
    tmp = (intensity / 256.0) * lm->affine_scale;
    value = (int32_t) (tmp + 0.5) + lm->affine_bias;

    hid_stub_set_light(lm->dest.hid, lm->dest.light_no, value);
}

struct mapper *mapper_impl_create(void)
{
    struct mapper *m;

    m = xmalloc(sizeof(*m));

    array_init(&m->actions);

    m->analogs = NULL;
    m->nanalogs = 0;

    array_init(&m->light_maps);

    m->lights = NULL;
    m->nlights = 0;

    return m;
}

void mapper_impl_clear_action_map(
    struct mapper *m, uint8_t action, uint8_t page)
{
    struct action_mapping *am;
    size_t i;

    for (i = 0; i < m->actions.nitems; i++) {
        am = array_item(struct action_mapping, &m->actions, i);

        if (am->action == action && am->page == page) {
            array_remove(struct action_mapping, &m->actions, i);

            return;
        }
    }
}

void mapper_impl_clear_light_map(
    struct mapper *m, const struct mapped_light *ml)
{
    struct light_mapping *lm;
    size_t i;

    for (i = 0; i < m->light_maps.nitems;) {
        lm = array_item(struct light_mapping, &m->light_maps, i);

        if (memcmp(&lm->dest, ml, sizeof(*ml)) == 0) {
            array_remove(struct light_mapping, &m->light_maps, i);
        } else {
            i++;
        }
    }
}

bool mapper_impl_get_action_map(
    struct mapper *m, uint8_t action, uint8_t page, struct mapped_action *ma)
{
    const struct action_mapping *am;
    size_t i;

    for (i = 0; i < m->actions.nitems; i++) {
        am = array_item(struct action_mapping, &m->actions, i);

        if (am->action == action && am->page == page) {
            *ma = am->src;

            return true;
        }
    }

    return false;
}

bool mapper_impl_get_analog_map(
    struct mapper *m, uint8_t analog, struct mapped_analog *ma)
{
    memset(ma, 0, sizeof(*ma));

    if (analog >= m->nanalogs) {
        return false;
    }

    *ma = m->analogs[analog].src;

    return true;
}

int32_t mapper_impl_get_analog_sensitivity(struct mapper *m, uint8_t analog)
{
    if (analog >= m->nanalogs) {
        return 0;
    }

    return m->analogs[analog].sensitivity;
}

bool mapper_impl_get_analog_invert(struct mapper* m, uint8_t analog)
{
    if (analog >= m->nanalogs) {
        return 0;
    }

    return m->analogs[analog].invert;
}

uint8_t mapper_impl_get_nanalogs(struct mapper *m)
{
    return m->nanalogs;
}

uint8_t mapper_impl_get_nlights(struct mapper *m)
{
    return m->nlights;
}

uint8_t mapper_impl_get_npages(struct mapper *m)
{
    const struct action_mapping *am;
    size_t i;
    int max_page;

    max_page = -1;

    for (i = 0; i < m->actions.nitems; i++) {
        am = array_item(struct action_mapping, &m->actions, i);

        if (max_page < am->page) {
            max_page = am->page;
        }
    }

    return (uint8_t) (max_page + 1);
}

action_iter_t mapper_impl_iterate_actions(struct mapper *m)
{
    struct action_iter *iter;

    iter = xmalloc(sizeof(*iter));
    iter->mapper = m;
    iter->i = 0;

    return iter;
}

light_iter_t mapper_impl_iterate_lights(struct mapper *m)
{
    struct light_iter *iter;

    iter = xmalloc(sizeof(*iter));
    iter->mapper = m;
    iter->i = 0;

    return iter;
}

bool mapper_impl_is_analog_absolute(struct mapper *m, uint8_t analog)
{
    struct analog_mapping *am;

    if (analog >= m->nanalogs) {
        return false;
    }

    am = &m->analogs[analog];

    if (!am->bound) {
        analog_mapping_bind(am);
    }

    return m->analogs[analog].absolute;
}

void mapper_impl_set_action_map(
    struct mapper *m,
    uint8_t action,
    uint8_t page,
    uint8_t bit,
    const struct mapped_action *ma)
{
    struct action_mapping *am;
    size_t i;

    for (i = 0; i < m->actions.nitems; i++) {
        am = array_item(struct action_mapping, &m->actions, i);

        if (am->action == action && am->page == page) {
            am->src = *ma;

            return;
        }
    }

    am = array_append(struct action_mapping, &m->actions);

    am->src = *ma;
    am->action = action;
    am->page = page;
    am->bit = bit;
}

bool mapper_impl_set_analog_map(
    struct mapper *m, uint8_t analog, const struct mapped_analog *ma)
{
    struct analog_mapping *am;

    if (analog >= m->nanalogs) {
        return false;
    }

    am = &m->analogs[analog];

    am->src = *ma;
    am->bound = false;

    return true;
}

bool mapper_impl_set_analog_sensitivity(
    struct mapper *m, uint8_t analog, int32_t sensitivity)
{
    if (analog >= m->nanalogs) {
        return false;
    }

    m->analogs[analog].sensitivity = sensitivity;

    return true;
}

bool mapper_impl_set_analog_invert(
    struct mapper *m, uint8_t analog, bool invert)
{
    if (analog >= m->nanalogs) {
        return false;
    }

    m->analogs[analog].invert = invert;

    return true;
}

void mapper_impl_set_light_map(
    struct mapper *m, const struct mapped_light *ml, uint8_t game_light)
{
    struct light_mapping *lm;
    size_t i;

    for (i = 0; i < m->light_maps.nitems; i++) {
        lm = array_item(struct light_mapping, &m->light_maps, i);

        if (memcmp(&lm->dest, ml, sizeof(*ml)) == 0) {
            lm->game_light = game_light;

            return;
        }
    }

    lm = array_append(struct light_mapping, &m->light_maps);

    memset(lm, 0, sizeof(*lm));

    lm->dest = *ml;
    lm->game_light = game_light;
}

void mapper_impl_set_nanalogs(struct mapper *m, uint8_t nanalogs)
{
    free(m->analogs);
    m->analogs = xcalloc(sizeof(*m->analogs) * nanalogs);
    m->nanalogs = nanalogs;
}

void mapper_impl_set_nlights(struct mapper *m, uint8_t nlights)
{
    free(m->lights);
    m->lights = xcalloc(sizeof(*m->lights) * nlights);
    m->nlights = nlights;
}

uint8_t mapper_impl_read_analog(struct mapper *m, uint8_t analog)
{
    if (analog >= m->nanalogs) {
        return 0;
    }

    return m->analogs[analog].pos;
}

uint64_t mapper_impl_update(struct mapper *m)
{
    struct action_mapping *am;
    struct light_mapping *lm;
    size_t i;
    uint64_t result;

    hid_mgr_lock();

    for (i = 0; i < m->light_maps.nitems; i++) {
        lm = array_item(struct light_mapping, &m->light_maps, i);
        light_mapping_send(lm, m);
    }

    for (i = 0; i < m->nanalogs; i++) {
        analog_mapping_update(&m->analogs[i]);
    }

    result = 0;

    for (i = 0; i < m->actions.nitems; i++) {
        am = array_item(struct action_mapping, &m->actions, i);
        result |= action_mapping_update(am);
    }

    hid_mgr_unlock();

    return result;
}

void mapper_impl_write_light(struct mapper *m, uint8_t light, uint8_t intensity)
{
    if (light >= m->nlights) {
        return;
    }

    m->lights[light] = intensity;
}

void mapper_impl_destroy(struct mapper *m)
{
    free(m->lights);
    array_fini(&m->light_maps);
    free(m->analogs);
    array_fini(&m->actions);
    free(m);
}
