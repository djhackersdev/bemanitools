#ifndef ACIOTEST_HANDLER_H
#define ACIOTEST_HANDLER_H

static const uint8_t aciotest_handler_max = 16;

/**
 * Handler interface for an ACIO device.
 */
struct aciotest_handler_node_handler
{
    void* ctx;
    bool (*init)(uint8_t node_id, void** ctx);
    bool (*update)(uint8_t node_id, void* ctx);
};

#endif