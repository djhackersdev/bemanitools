#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

#include <windows.h>

#include "aciodrv/device.h"

#include "aciotest/handler.h"
#include "aciotest/icca.h"
#include "aciotest/kfca.h"

#include "util/log.h"

static uint8_t aciotest_cnt = 0;

/**
 * Enumerate supported ACIO nodes based on their product id.
 */
static bool aciotest_assign_handler(char product[4],
        struct aciotest_handler_node_handler* handler)
{
    if (!memcmp(product, "ICCA", 4)
        || !memcmp(product, "ICCB", 4)
        || !memcmp(product, "ICCC", 4)
    ){
        handler->init = aciotest_icca_handler_init;
        handler->update = aciotest_icca_handler_update;

        return true;
    }

    if (!memcmp(product, "KFCA", 4)){
        handler->init = aciotest_kfca_handler_init;
        handler->update = aciotest_kfca_handler_update;

        return true;
    }

    return false;
}

/**
 * Tool to test real ACIO hardware.
 */
int main(int argc, char** argv)
{
    if (argc < 3) {
        printf(
            "aciotest, build "__DATE__ " " __TIME__ "\n"
            "Usage: %s <com port str> <baud rate>\n"
            "Example for two slotted readers: %s COM1 57600\n",
            argv[0], argv[0]);
        return -1;
    }

    log_to_writer(log_writer_stdout, NULL);

    if (!aciodrv_device_open(argv[1], atoi(argv[2]))) {
        printf("Opening acio device failed\n");
        return -1;
    }

    printf("Opening acio device successful\n");

    uint8_t node_count = aciodrv_device_get_node_count();
    printf("Enumerated %d nodes\n", node_count);

    struct aciotest_handler_node_handler handler[aciotest_handler_max];
    memset(&handler, 0,
        sizeof(struct aciotest_handler_node_handler) * aciotest_handler_max);

    for (uint8_t i = 0; i < node_count; i++) {
        char product[4];
        aciodrv_device_get_node_product_ident(i, product);
        printf("> %d: %c%c%c%c\n", i + 1,
            product[0], product[1], product[2], product[3]);

        if (!aciotest_assign_handler(product, &handler[i])) {
            printf("ERROR: Unsupported acio node product %c%c%c%c on node %d\n",
                product[0], product[1], product[2], product[3], i);
        }

    }

    for (uint8_t i = 0; i < aciotest_handler_max; i++) {

        if (handler[i].init != NULL) {

            if (!handler[i].init(i, &handler[i].ctx)) {
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

                if (!handler[i].update(i, handler[i].ctx)) {
                    printf("ERROR: Updating node %d, removed from loop\n", i);
                    handler[i].update = NULL;
                    Sleep(5000);
                }

            }

        }

        /* avoid cpu banging */
        Sleep(20);
    }

    return 0;
}