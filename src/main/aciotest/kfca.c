#include "aciotest/kfca.h"

#include "acio/acio.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "aciodrv/kfca.h"

bool aciotest_kfca_handler_init(
    struct aciodrv_device_ctx *device, uint8_t node_id, void **ctx)
{
    *ctx = malloc(sizeof(uint32_t));
    *((uint32_t *) *ctx) = 0;

    return aciodrv_kfca_init(device, node_id);
}

bool aciotest_kfca_handler_update(
    struct aciodrv_device_ctx *device, uint8_t node_id, void *ctx)
{
    struct ac_io_kfca_poll_in pin;
    struct ac_io_kfca_poll_out pout;

    static int gpio_test_pin = 0;
    static int gpio_test_counter = 0;

    memset(&pout, 0, sizeof(pout));
    pout.gpio |= 1 << gpio_test_pin;
    pout.gpio = ac_io_u32(pout.gpio);

    if (!aciodrv_kfca_poll(device, node_id, &pout, &pin)) {
        return false;
    }

    pin.adc[0] = ac_io_u16(pin.adc[0]);
    pin.adc[1] = ac_io_u16(pin.adc[1]);

    pin.gpio[0] = ac_io_u16(pin.gpio[0]);
    pin.gpio[1] = ac_io_u16(pin.gpio[1]);

    printf(
        ">>> KFCA %d: GPIO ON %d\n"
        "BTN A B C D: %d %d %d %d\n"
        "FX-L R: %d %d\n"
        "VOL L: %d\n"
        "VOL R: %d\n"
        "START COIN TEST SERV REC HP: %d %d %d %d %d %d\n",
        node_id,
        gpio_test_pin,
        pin.gpio[0] & AC_IO_KFCA_IN_GPIO_0_A,
        pin.gpio[0] & AC_IO_KFCA_IN_GPIO_0_B,
        pin.gpio[0] & AC_IO_KFCA_IN_GPIO_0_C,
        pin.gpio[1] & AC_IO_KFCA_IN_GPIO_1_D,
        pin.gpio[1] & AC_IO_KFCA_IN_GPIO_1_FX_L,
        pin.gpio[1] & AC_IO_KFCA_IN_GPIO_1_FX_R,
        (pin.adc[0] >> 6) & 0x3FF,
        (pin.adc[1] >> 6) & 0x3FF,
        pin.gpio[0] & AC_IO_KFCA_IN_GPIO_0_START,
        (pin.gpio_sys) & AC_IO_KFCA_IN_GPIO_SYS_COIN,
        (pin.gpio_sys) & AC_IO_KFCA_IN_GPIO_SYS_TEST,
        (pin.gpio_sys) & AC_IO_KFCA_IN_GPIO_SYS_SERVICE,
        pin.gpio[0] & AC_IO_KFCA_IN_GPIO_0_RECORDER,
        pin.gpio[0] & AC_IO_KFCA_IN_GPIO_0_HEADPHONE);

    ++gpio_test_counter;
    if (gpio_test_counter >= 4) {
        gpio_test_counter = 0;
        ++gpio_test_pin;

        if (gpio_test_pin >= 16) {
            gpio_test_pin = 0;
        }
    }

    return true;
}
