#ifndef GENINPUT_DEV_LIST_H
#define GENINPUT_DEV_LIST_H

#include <setupapi.h>
#include <windows.h>

#include <stdbool.h>
#include <wchar.h>

struct dev_list {
    GUID *class_guid;
    HDEVINFO infolist;
    SP_DEVINFO_DATA dev;
    SP_DEVICE_INTERFACE_DATA iface;
    SP_DEVICE_INTERFACE_DETAIL_DATA *detail;
    wchar_t *name;
    unsigned int dev_no;
    unsigned int iface_no;
};

void dev_list_init(struct dev_list *devs, const GUID *class_guid);
bool dev_list_next(struct dev_list *devs);
const char *dev_list_get_dev_node(struct dev_list *devs);
const wchar_t *dev_list_get_dev_name(struct dev_list *devs);
void dev_list_fini(struct dev_list *devs);

#endif
