#include <stdlib.h>

#include "geninput/hid.h"
#include "geninput/kbd-naive.h"
#include "geninput/kbd.h"

static struct hid_stub *kbd_naive_stub = NULL;
static struct hid_ri *kbd_naive_hid_ri = NULL;

static const char *kbd_naive_dev_node = "kbd-naive";
static const wchar_t *kbd_naive_dev_name = L"Any (naive binding)";

void kbd_naive_init(void)
{
    kbd_naive_stub = hid_mgr_get_named_stub(kbd_naive_dev_node);
        
    kbd_create(&kbd_naive_hid_ri, kbd_naive_dev_node, kbd_naive_dev_name);

    hid_stub_attach(kbd_naive_stub, (struct hid *) kbd_naive_hid_ri);
}

void kbd_naive_fini(void)
{
    free(kbd_naive_stub);
    kbd_naive_stub = NULL;
}

void kbd_naive_event_notify(RAWINPUT *ri)
{
    hid_ri_handle_event(kbd_naive_hid_ri, ri);
}

bool kbd_is_naive(struct hid_stub *stub)
{
    return (stub == kbd_naive_stub);
}