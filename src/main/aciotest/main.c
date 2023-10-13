#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include <windows.h>

#include "aciodrv/device.h"

#include "aciotest/bi2a-sdvx.h"
#include "aciotest/bi2a-iidx.h"
#include "aciotest/handler.h"
#include "aciotest/icca.h"
#include "aciotest/kfca.h"
#include "aciotest/panb.h"
#include "aciotest/rvol.h"

#include "util/log.h"

static uint8_t aciotest_cnt = 0;
<<<<<<< HEAD
static uint8_t bi2a_mode = 255;
=======
static uint8_t bi2a_mode = -1;
>>>>>>> 3ef04ad6e6a3fd0bc7195c3fe2fa901137fd3105

/**
 * Enumerate supported ACIO nodes based on their product id.
 */
static bool aciotest_assign_handler(
    uint32_t product_type, struct aciotest_handler_node_handler *handler)
{
    if (product_type == AC_IO_NODE_TYPE_ICCA) {
        handler->init = aciotest_icca_handler_init;
        handler->update = aciotest_icca_handler_update;

        return true;
    }

    if (product_type == AC_IO_NODE_TYPE_KFCA) {
        handler->init = aciotest_kfca_handler_init;
        handler->update = aciotest_kfca_handler_update;

        return true;
    }

    if (product_type == AC_IO_NODE_TYPE_PANB) {
        handler->init = aciotest_panb_handler_init;
        handler->update = aciotest_panb_handler_update;

        return true;
    }

    if (product_type == AC_IO_NODE_TYPE_RVOL) {
        handler->init = aciotest_rvol_handler_init;
        handler->update = aciotest_rvol_handler_update;

        return true;
    }

    if (product_type == AC_IO_NODE_TYPE_BI2A) {
<<<<<<< HEAD
        if (bi2a_mode == 255) {
            printf(
                "Unknown BI2A mode specified, please check your command.\n"
                "Using bi2a-sdvx mode as default, press ENTER to continue\n"
                );
            bi2a_mode = 0;
            getchar();
        }

        switch (bi2a_mode)
        {
        case 0:
            handler->init = aciotest_bi2a_sdvx_handler_init;
            handler->update = aciotest_bi2a_sdvx_handler_update;
            break;
        case 1:
            handler->init = aciotest_bi2a_iidx_handler_init;
            handler->update = aciotest_bi2a_iidx_handler_update;
            break;

        default:
            break;
=======
        printf("Please assign bi2a mode, 0 for SDVX and 1 for IIDX, then press ENTER:");
        scanf("%hhd",&bi2a_mode);
        if (bi2a_mode == 0) {
            handler->init = aciotest_bi2a_sdvx_handler_init;
            handler->update = aciotest_bi2a_sdvx_handler_update;
            return true;
        } else if (bi2a_mode == 1) {
            handler->init = aciotest_bi2a_iidx_handler_init;
            handler->update = aciotest_bi2a_iidx_handler_update;
        } else {
            printf("Unknown BI2A device specified");
>>>>>>> 3ef04ad6e6a3fd0bc7195c3fe2fa901137fd3105
        }
        return true;
    }

    return false;
}

/**
 * Tool to test real ACIO hardware.
 */
int main(int argc, char **argv)
{
    if (argc < 3) {
        printf(
            "aciotest, build "__DATE__
            " " __TIME__
            "\n"
            "Usage: %s <com port str> <baud rate> <bi2a mode(optional)>\n"
            "Example:\n"
            "\"%s COM1 57600\" for generic acio device\n"
            "\"%s COM1 57600 bi2a-iidx\" for the iidx BI2A mode\n"
            "\"%s COM1 57600 bi2a-sdvx\" for the sdvx BI2A mode\n",
            argv[0],
            argv[0],
            argv[0],
            argv[0]);
        return -1;
    }

    if (argc == 4) {
        if(!strcmp(argv[3],"bi2a-iidx")) {
            bi2a_mode = 1;
        } else if(!strcmp(argv[3],"bi2a-sdvx")) {
            bi2a_mode = 0;
        }
    }

    log_to_writer(log_writer_stdout, NULL);

    struct aciodrv_device_ctx *device =
        aciodrv_device_open_path(argv[1], atoi(argv[2]));

    if (!device) {
        printf("Opening acio device failed\n");
        return -1;
    }

    printf("Opening acio device successful, press ENTER to continue\n");
    getchar();

    uint8_t node_count = aciodrv_device_get_node_count(device);
    printf("Enumerated %d nodes\n", node_count);

    struct aciotest_handler_node_handler handler[aciotest_handler_max];
    memset(
        &handler,
        0,
        sizeof(struct aciotest_handler_node_handler) * aciotest_handler_max);

    for (uint8_t i = 0; i < node_count; i++) {
        char product[4];
        uint32_t product_type = aciodrv_device_get_node_product_type(device, i);
        aciodrv_device_get_node_product_ident(device, i, product);

        printf(
            "> %d: %c%c%c%c (%08x)\n",
            i + 1,
            product[0],
            product[1],
            product[2],
            product[3],
            product_type);

        if (!aciotest_assign_handler(product_type, &handler[i])) {
            printf(
                "ERROR: Unsupported acio node product %08x on node %d\n",
                product_type,
                i);
        }
    }

    for (uint8_t i = 0; i < aciotest_handler_max; i++) {
        if (handler[i].init != NULL) {
            if (!handler[i].init(device, i, &handler[i].ctx)) {
                printf("ERROR: Initializing node %d failed\n", i);
                handler[i].update = NULL;
            }
        }
    }

    printf(">>> Initializing done, press enter to start update loop <<<\n");

    if (getchar() != '\n') {
        return 0;
    }

    while (true) {
        system("cls");
        printf("%d\n", aciotest_cnt++);

        for (uint8_t i = 0; i < aciotest_handler_max; i++) {
            if (handler[i].update != NULL) {
                if (!handler[i].update(device, i, handler[i].ctx)) {
                    printf("ERROR: Updating node %d, removed from loop\n", i);
                    handler[i].update = NULL;
                    Sleep(5000);
                }
            }
        }

        /* avoid cpu banging */
        Sleep(30);
    }

    return 0;
}