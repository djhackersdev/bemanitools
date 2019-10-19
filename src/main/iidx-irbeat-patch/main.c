#include <windows.h>

#include <stdio.h>
#include <stdlib.h>

#include "util/fs.h"

static const uint32_t offset_size_data_area = 4;
static const uint32_t offset_data_area_checksum = 8;
static const uint32_t offset_data_area = 12;
static const uint32_t offset_file_header_checksum = 0;
static const uint32_t size_file_header = 8;

static uint32_t checksum_settings_data_09(const uint8_t *buffer)
{
    uint32_t sum = 0;

    for (uint32_t i = 0; i < 10; i++) {
        sum += ((uint8_t) i) ^ buffer[i];
    }
    sum += 10;

    for (uint32_t i = 0; i < 0x34; i++) {
        sum += ((uint8_t) i) ^ buffer[12 + i];
    }
    sum += 0x34;

    for (uint32_t i = 0; i < 4; i++) {
        sum += ((uint8_t) i) ^ buffer[64 + i];
    }
    sum += 4;

    for (uint32_t i = 0; i < 0x1E; i++) {
        sum += ((uint8_t) i) ^ buffer[68 + i];
    }
    sum += 0x1E;

    for (uint32_t i = 0; i < 4; i++) {
        sum += ((uint8_t) i) ^ buffer[100 + i];
    }
    sum += 4;

    for (uint32_t i = 0; i < 0xA; i++) {
        sum += ((uint8_t) i) ^ buffer[108 + i];
    }
    sum += 0xA;

    for (uint32_t i = 0; i < 0x438; i++) {
        sum += ((uint8_t) i) ^ buffer[120 + i];
    }
    sum += 0x438;

    for (uint32_t i = 0; i < 0x690; i++) {
        sum += ((uint8_t) i) ^ buffer[1200 + i];
    }
    sum += 0x690;

    return sum;
}

static uint32_t checksum_settings_data_10(const uint8_t *buffer)
{
    uint32_t sum = 0;

    for (uint32_t i = 0; i < 10; i++) {
        sum += ((uint8_t) i) ^ buffer[i];
    }
    sum += 10;

    for (uint32_t i = 0; i < 0x34; i++) {
        sum += ((uint8_t) i) ^ buffer[12 + i];
    }
    sum += 0x34;

    for (uint32_t i = 0; i < 4; i++) {
        sum += ((uint8_t) i) ^ buffer[64 + i];
    }
    sum += 4;

    for (uint32_t i = 0; i < 0x1E; i++) {
        sum += ((uint8_t) i) ^ buffer[68 + i];
    }
    sum += 0x1E;

    for (uint32_t i = 0; i < 4; i++) {
        sum += ((uint8_t) i) ^ buffer[100 + i];
    }
    sum += 4;

    for (uint32_t i = 0; i < 0xA; i++) {
        sum += ((uint8_t) i) ^ buffer[108 + i];
    }
    sum += 0xA;

    for (uint32_t i = 0; i < 0x49C; i++) {
        sum += ((uint8_t) i) ^ buffer[120 + i];
    }
    sum += 0x49C;

    for (uint32_t i = 0; i < 0x690; i++) {
        sum += ((uint8_t) i) ^ buffer[1300 + i];
    }
    sum += 0x690;

    return sum;
}

uint32_t checksum(const uint8_t *buffer, size_t length)
{
    uint32_t sum = 0;
    const uint32_t *ptr = (const uint32_t *) buffer;

    for (uint32_t i = 0; i < length / 4; i++) {
        sum += *ptr ^ i;
        ptr++;
    }

    return sum;
}

static void patch_09(uint8_t *buffer, uint32_t size_data_area, uint8_t irbeat)
{
    /* patch beat phase */
    buffer[0x49] = irbeat;

    // settings data checksum */
    *((uint32_t *) &buffer[0x74]) =
        checksum_settings_data_09(&buffer[offset_data_area]);

    /* update data area checksum */
    *((uint32_t *) &buffer[offset_data_area_checksum]) =
        checksum(&buffer[offset_data_area], size_data_area);

    /* update file checksum */
    *((uint32_t *) &buffer[offset_file_header_checksum]) =
        checksum(&buffer[offset_size_data_area], size_file_header);
}

static void patch_10(uint8_t *buffer, uint32_t size_data_area, uint8_t irbeat)
{
    /* patch beat phase */
    buffer[0x49] = irbeat;

    // settings data checksum */
    *((uint32_t *) &buffer[0x74]) =
        checksum_settings_data_10(&buffer[offset_data_area]);

    /* update data area checksum */
    *((uint32_t *) &buffer[offset_data_area_checksum]) =
        checksum(&buffer[offset_data_area], size_data_area);

    /* update file checksum */
    *((uint32_t *) &buffer[offset_file_header_checksum]) =
        checksum(&buffer[offset_size_data_area], size_file_header);
}

int main(int argc, char **argv)
{
    uint8_t version;
    uint8_t irbeat;
    const char *settings;
    uint8_t *buffer;
    size_t size;
    uint32_t size_data_area;

    if (argc < 4) {
        printf("Patches the IRBeat phase of 9th or 10th Style\n");
        fprintf(
            stderr, "Usage: %s <version> <irbeat> <settings.bin>\n", argv[0]);
        return -1;
    }

    version = atoi(argv[1]);
    irbeat = atoi(argv[2]);
    settings = argv[3];

    if (version != 9 && version != 10) {
        fprintf(stderr, "Invalid game version %d\n", version);
        return -2;
    }

    if (irbeat > 2) {
        fprintf(stderr, "Invalid IR beat phase %d\n", irbeat);
        return -3;
    }

    printf(
        "Setting IRBeat phase %d on settings file %s\n", irbeat + 1, settings);

    if (!file_load(settings, (void **) &buffer, &size, false)) {
        fprintf(stderr, "Loading file %s failed\n", settings);
        return -4;
    }

    size_data_area = *((uint32_t *) &buffer[offset_size_data_area]);

    switch (version) {
        case 9:
            patch_09(buffer, size_data_area, irbeat);
            break;
        case 10:
            patch_10(buffer, size_data_area, irbeat);
            break;
        default:
            break;
    }

    if (!file_save(settings, buffer, size)) {
        fprintf(stderr, "Saving to file %s failed\n", settings);
        return -5;
    }

    printf("Patching successful\n");
    return 0;
}