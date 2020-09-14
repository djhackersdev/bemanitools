
#include <stdio.h>
#include <windows.h>

#include "ViGEm/Client.h"

#include "vigemstub/helper.h"

PVIGEM_CLIENT vigem_helper_setup(void)
{
    PVIGEM_CLIENT client = vigem_alloc();

    if (client == NULL) {
        printf("vigem_alloc failed\n");
        return NULL;
    }

    VIGEM_ERROR retval = vigem_connect(client);

    if (!VIGEM_SUCCESS(retval)) {
        printf("ViGEm Bus connection failed with error code: 0x%x\n", retval);
        return NULL;
    }

    return client;
}

PVIGEM_TARGET vigem_helper_add_pad(PVIGEM_CLIENT client)
{
    PVIGEM_TARGET pad = vigem_target_x360_alloc();

    if (pad == NULL) {
        printf("vigem_target_x360_alloc failed\n");
        return NULL;
    }

    VIGEM_ERROR pir = vigem_target_add(client, pad);

    if (!VIGEM_SUCCESS(pir)) {
        printf("Target plugin failed with error code: 0x%x\n", pir);
        return NULL;
    }

    return pad;
}