/* This is the source code for the JBIO.DLL that ships with Bemanitools 5.

   If you want to add on some minor functionality like a custom RGB LED setup
   then feel free to extend this code with support for your custom device.

   If you want to make a completely custom IO board that handles all input and
   lighting then you'd be better off writing your own from scratch. Consult
   the "bemanitools" header files included by this source file for detailed
   information about the API you'll need to implement. */

#include <mmsystem.h>
#include <windows.h>

#include "bemanitools/input.h"
#include "bemanitools/jbio.h"

static uint16_t jb_io_panels;
static uint8_t jb_io_sys_buttons;

/* Uncomment these if you need them. */

#if 0
static log_formatter_t jb_io_log_misc;
static log_formatter_t jb_io_log_info;
static log_formatter_t jb_io_log_warning;
static log_formatter_t jb_io_log_fatal;
#endif

void jb_io_set_loggers(
    log_formatter_t misc,
    log_formatter_t info,
    log_formatter_t warning,
    log_formatter_t fatal)
{
    /* Pass logger functions on to geninput so that it has somewhere to write
       its own log output. */

    input_set_loggers(misc, info, warning, fatal);

    /* Uncomment this block if you have something you'd like to log.

       You should probably return false from the appropriate function instead
       of calling the fatal logger yourself though. */

#if 0
    jb_io_log_misc = misc;
    jb_io_log_info = info;
    jb_io_log_warning = warning;
    jb_io_log_fatal = fatal;
#endif
}

bool jb_io_init(
    thread_create_t thread_create,
    thread_join_t thread_join,
    thread_destroy_t thread_destroy)
{
    timeBeginPeriod(1);

    input_init(thread_create, thread_join, thread_destroy);
    mapper_config_load("jb");

    /* Initialize your own IO devices here. Log something and then return
       false if the initialization fails. */

    return true;
}

void jb_io_fini(void)
{
    /* This function gets called as JB shuts down after an Alt-F4. Close your
       connections to your IO devices here. */

    input_fini();
    timeEndPeriod(1);
}

bool jb_io_read_inputs(void)
{
    uint32_t buttons;
    /* Sleep first: input is timestamped immediately AFTER the ioctl returns.

       Which is the right thing to do, for once. We sleep here because
       the game polls input in a tight loop. Can't complain, at there isn't
       an artificial limit on the poll frequency. */

    Sleep(1);

    /* Update all of our input state here. */

    buttons = (uint32_t) mapper_update();

    /* Mask out the stuff provided by geninput and store the panel/button state
       for later retrieval via jb_io_get_buttons() */

    jb_io_panels = buttons & 0xFFFF;
    jb_io_sys_buttons = (buttons >> 16) & 0x03;

    return true;
}

bool jb_io_write_outputs(void)
{
    /* The generic input stack currently initiates lighting sends and input
       reads simultaneously, though this might change later. Perform all of our
       I/O immediately before reading out the inputs so that the input state is
       as fresh as possible. */

    return true;
}

uint8_t jb_io_get_sys_inputs(void)
{
    return jb_io_sys_buttons;
}

uint16_t jb_io_get_panel_inputs(void)
{
    return jb_io_panels;
}

void jb_io_set_rgb_led(enum jb_io_rgb_led unit, uint8_t r, uint8_t g, uint8_t b)
{
    mapper_write_light(unit * 3, r);
    mapper_write_light(unit * 3 + 1, g);
    mapper_write_light(unit * 3 + 2, b);
}