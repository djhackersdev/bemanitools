#ifndef CAMHOOK_CAM_DETECT_H
#define CAMHOOK_CAM_DETECT_H

#include <stdbool.h>
#include <stdint.h>
#include <wchar.h>

#define CAMERA_DATA_STRING_SIZE 0x100

struct camera_data {
    bool setup;
    char name[CAMERA_DATA_STRING_SIZE];
    char deviceInstancePath[CAMERA_DATA_STRING_SIZE];
    wchar_t deviceSymbolicLink[CAMERA_DATA_STRING_SIZE];
    char extra_upper[CAMERA_DATA_STRING_SIZE];
    int address;
    char parent_name[CAMERA_DATA_STRING_SIZE];
    char parent_driverKey[CAMERA_DATA_STRING_SIZE];
    char parent_deviceInstancePath[CAMERA_DATA_STRING_SIZE];
    int parent_address;

    int16_t vid;
    int16_t pid;

    bool fake_addressed;
    int fake_address;

    bool fake_located;
    size_t fake_located_node;
};

void fill_cam_struct(struct camera_data *data, const char *devid);

#endif
