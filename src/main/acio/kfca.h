#ifndef AC_IO_KFCA_H
#define AC_IO_KFCA_H

#include <stdint.h>

#define AC_IO_CMD_KFCA_POLL 0x0113
#define AC_IO_CMD_KFCA_WATCHDOG 0x0120
#define AC_IO_CMD_KFCA_AMP_CONTROL 0x0128

#define AC_IO_KFCA_IN_GPIO_SYS_COIN 0x04
/*  ... AC_IO_KFCA_IN_GPIO_SYS_COIN2    0x08 (maybe?) */
#define AC_IO_KFCA_IN_GPIO_SYS_TEST 0x10
#define AC_IO_KFCA_IN_GPIO_SYS_SERVICE 0x20

#define AC_IO_KFCA_IN_GPIO_0_C 0x0001
#define AC_IO_KFCA_IN_GPIO_0_B 0x0002
#define AC_IO_KFCA_IN_GPIO_0_A 0x0004
#define AC_IO_KFCA_IN_GPIO_0_START 0x0008
#define AC_IO_KFCA_IN_GPIO_0_RECORDER 0x0010
#define AC_IO_KFCA_IN_GPIO_0_HEADPHONE 0x0020

#define AC_IO_KFCA_IN_GPIO_1_FX_R 0x0008
#define AC_IO_KFCA_IN_GPIO_1_FX_L 0x0010
#define AC_IO_KFCA_IN_GPIO_1_D 0x0020

#pragma pack(push, 1)

struct ac_io_kfca_poll_in {
    /* ADC data is 10 bits. Low 6 bits of ADCs 1 through 3 are zero, low 6 bits
       of ADC 0 is the system GPIOs (test, service, coin). */

    union {
        uint16_t gpio_sys;

        struct {
            uint16_t adc[4];
            uint16_t gpio[4];
        };
    };
};

struct ac_io_kfca_poll_out {
    uint32_t gpio;
    uint8_t pwm[20];
};

#pragma pack(pop)

#endif
