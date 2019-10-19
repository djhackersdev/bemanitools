#include <windows.h>

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#include "imports/avs.h"

#include "launcher/avs-context.h"

#include "util/log.h"

static void *avs_heap;

#ifdef AVS_HAS_STD_HEAP
static void *std_heap;
#endif

void avs_context_init(
    struct property_node *config,
    uint32_t avs_heap_size,
    uint32_t std_heap_size,
    avs_log_writer_t log_writer,
    void *log_writer_ctx)
{
    avs_heap = VirtualAlloc(
        NULL, avs_heap_size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

    if (avs_heap == NULL) {
        log_fatal(
            "Failed to VirtualAlloc %d byte AVS heap: %08x",
            avs_heap_size,
            (unsigned int) GetLastError());
    }

#ifdef AVS_HAS_STD_HEAP
    std_heap = VirtualAlloc(
        NULL, std_heap_size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

    if (std_heap == NULL) {
        log_fatal(
            "Failed to VirtualAlloc %d byte \"std\" heap: %08x",
            std_heap_size,
            (unsigned int) GetLastError());
    }
#endif

#ifdef AVS_HAS_STD_HEAP
    avs_boot(
        config,
        std_heap,
        std_heap_size,
        avs_heap,
        avs_heap_size,
        log_writer,
        log_writer_ctx);
#else
    /* AVS v2.16.xx and I suppose onward uses a unified heap */
    avs_boot(config, avs_heap, avs_heap_size, NULL, log_writer, log_writer_ctx);
#endif
}

void avs_context_fini(void)
{
    avs_shutdown();

#ifdef AVS_HAS_STD_HEAP
    VirtualFree(std_heap, 0, MEM_RELEASE);
#endif

    VirtualFree(avs_heap, 0, MEM_RELEASE);
}
