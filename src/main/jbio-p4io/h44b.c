#define LOG_MODULE "jbio-h44b"

#include <stdatomic.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "aciodrv/h44b.h"

#include "aciomgr/manager.h"

#include "core/log.h"

static int16_t h44b_node_id;

static atomic_bool running;

static struct aciomgr_port_dispatcher *acio_manager_ctx;

bool jb_io_h44b_init(const char *port, int32_t baud)
{
    acio_manager_ctx = aciomgr_port_init(port, baud);

    if (acio_manager_ctx == NULL) {
        log_info("Opening acio device on [%s] failed", port);
        return false;
    }

    log_info("Opening acio device successful");

    uint8_t node_count = aciomgr_get_node_count(acio_manager_ctx);
    log_info("Enumerated %d nodes", node_count);

    h44b_node_id = -1;

    for (uint8_t i = 0; i < node_count; i++) {
        char product[4];
        aciomgr_get_node_product_ident(acio_manager_ctx, i, product);
        log_info(
            "> %d: %c%c%c%c",
            i,
            product[0],
            product[1],
            product[2],
            product[3]);

        if (!memcmp(product, "H44B", 4)) {
            if (h44b_node_id != -1) {
                log_warning("Multiple H44B found! Using highest node id.");
            }
            h44b_node_id = i;
        }
    }

    if (h44b_node_id != -1) {
        log_warning("Using H44B on node: %d", h44b_node_id);

        bool init_result = aciodrv_h44b_init(
            aciomgr_port_checkout(acio_manager_ctx), h44b_node_id);
        aciomgr_port_checkin(acio_manager_ctx);

        if (!init_result) {
            log_warning("Unable to start H44B on node: %d", h44b_node_id);
            return false;
        }

        running = true;
        log_warning("jbio-h44b now running");
    } else {
        log_warning("No H44B device found");
    }

    return running;
}

bool jb_io_h44b_fini(void)
{
    aciomgr_port_fini(acio_manager_ctx);

    return true;
}

bool jb_io_h44b_write_lights(struct ac_io_h44b_output *lights)
{
    if (!running) {
        return false;
    }

    bool amp_result = aciodrv_h44b_lights(
        aciomgr_port_checkout(acio_manager_ctx), h44b_node_id, lights);
    aciomgr_port_checkin(acio_manager_ctx);

    if (!amp_result) {
        return false;
    }

    return true;
}
