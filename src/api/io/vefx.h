#ifndef BT_API_IO_VEFX_H
#define BT_API_IO_VEFX_H

/* IO emulation provider for beatmania IIDX Effector Panel. */

#include <stdbool.h>
#include <stdint.h>

typedef bool (*bt_io_vefx_init_t)();
typedef void (*bt_io_vefx_fini_t)();
typedef bool (*bt_io_vefx_recv_t)(uint64_t *ppad);
typedef uint8_t (*bt_io_vefx_slider_get_t)(uint8_t slider_no);
typedef bool (*bt_io_vefx_16seg_send_t)(const char *text);

typedef struct bt_io_vefx_api {
   uint16_t version;

   struct {
      // Required to be implemented
      bt_io_vefx_init_t init;
      bt_io_vefx_fini_t fini;
      bt_io_vefx_recv_t recv;
      bt_io_vefx_slider_get_t slider_get;
      bt_io_vefx_16seg_send_t _16seg_send;
   } v1;
} bt_io_vefx_api_t;

#endif
