# Release history
## 5.35
* Jubeat: Add jbio-p4io
* Nostalgia: Add nostio-panb
* Misc: Refactor aciodrv to support multiple devices using the same port (with aciomgr)
* Misc: Fix d3d9ex windowed hook not behaving correctly (size/framed)
* Misc: Fix docs not being included with release
* Various bugfixes

## 5.34
* IIDX: Support IO and card reader feature switches to disable emulation in iidxhook4-7
* Jubeat: jbio implementation for magicbox hardware
* Misc: Various documentation improvements
* IIDX: Turntable multiplier (can also be used as inverted) for iidxhook9
* IIDX: iidxhook9, add force screen resolution option to fix sometimes wrong auto res detection by
iidx27 causing inaccurate/wrong refresh rates on monitor check
* IIDX: iidxio BIO2 implementation -> Use BIO2 hardware with any iidxhook or other software
supporting BT5's iidxio interface
* IIDX: BIO2 exit hook, exit the game pressing Start P1 + Start P2, Effect and VEFX
* launcher: Allow overriding service URL from command line
* IIDX: vigem-iidxio, tool to allow using iidxio (ezusb, ezusb2 or bio2) to emulate XBOX controllers
to play other non arcade games that support xinput, e.g. Lunatic Rave (note: Infinitas does not
work with this since it only supports direct input)

## 5.33
* iidxhook9: Support for IIDX27 
* sdvxio-bio2: Driver for real sdvx5 cabinet IO hardware
* vigem-sdvxio: Tool to allow using sdvxio (kfca or bio2) to emulate an xbox controller
* Various bugfixes and refactoring for inject and launcher

## 5.32
* Various bugfixes

## 5.31
* DDR: Fix p3io and extio lights not working / being swapped when using geninput
* DDR: Add HDXS light support
* DDR: Add 64 bit support
* DDR: Add option to use COM4 as the p3io rs232 port instead of emulating (for use with acrealio etc.)
* Jubeat: Add network adapter hook
* Misc: Fix aciodrv sometimes hanging on boot (eamio-icca and sdvxio-kfca) 

## 5.30
* SDVX: sdvxhook2 headphone force and cursor confining config options
* DDR: Add light support for SMX gen 4 pads

## 5.29
* SDVX: Add option for monitor rotation
* SDVX: Add option to allow overriding network adapter IP
* IIDX: Add force display adapter and refresh rate configuration
* SDVX: Allow specifying display adapter to open for d3d9ex hook
* IIDX/SDVX camerahooks bugfix: Camera sometimes not detected on some older machines

## 5.28
* Improve documentation in various places
* Automatic code formatting in build pipeline
* Bugfix: Recursive config.bat script
* Enable automatic building on pushes to master
* Bugfix: Stop SDVX hanging on thankyou for playing
* sdvx2: Support for Vivid Wave using sdvxhook2
* sdvxio-kfca: Driver for real sdvx cabinet IO hardware

## 5.27
* Various documentation updates regarding development and how to contribute
* KFCA emulation accuracy fixes
* Fix passthrough of serial calls when using real hardware (e.g. card reader passthrough IIDX25+/SDVX5+)
* Add sdvxio-kfca driver to talk to real SDVX hardware
* Refactor d3d9 hook module to improve maintainability. Also fixes upscaling not working on IIDX20+.
* Remove d3d8 hook module to improve maintainability. Make old d3d8 based games, e.g. iidxhook1/2, use d3d9 instead.
Requires usage of d3d8to9 wrapper library to use gfx patching features.
* Add dev doc about IIDX rendering loops
* Add an initial draft of an architecture document (will be worked on in iterations)
* Refactor BIO2 emulation to make it re-usable by other games

## 5.26
* iidxio-ezusb: Reduce sleep time to Sleep(1) to avoid framerate issues on some versions of iidx.
* Bugfix: iidx d3d9 games, mainly the newer ones iidx 20-25, freezing. Happened on boot, during song selection or during the song.
* Bugfix: log_server_init deadlock on iidxhook4-7. This caused games like iidx24 to hang before even showing a render window.
* iidxhook: Add feature to allow GPU based up-/downscaling of rendered frame. This gives
you the possibility to upscale the resolution of old SD (640x480) games to your monitor's/TV's
native resolution which can have a few advantages: better image quality if the monitor's upscaler
is not doing a good job, especially on resolutions that are not a multiple of its native resolution;
Reduce display latency if the upscaler is slow, avoid over-/underscan which cannot always be fixed
entirely or at all (depending on your GPU and monitor model). See the iidxhook configuration file
for the new parameters available.
* Major readme cleanup. Add development documentation like style guide, guidelines, development setup.

## 5.25
* Bugfix: iidx14 and 15 crashing on Windows 10
* Bugfix: IO2 driver not using correct package sizes on reads/write -> iidxio-ezusb2.dll now working
* Improve ezusb2-boot.bat script to handle flashing of IO2 firmware
* Remove broken x64 builds of ezusb1/2 tools -> Just use the x86 tool versions instead
(use the x64 versions of iidxio-ezusb.dll and iidxio-ezusb2.dll with iidx25)
* iidxhook1-8: Allow floating point values for frame rate limiting, e.g. 59.95 (hz).
* Improve timing with ezusb (C02) driver

## v5.24
* Bugfix: iidxhook8 hangs very early on startup (race condition in log-server module)

## v5.23
* Refactored configurion (file) handling for iidxhooks, RE-READ THE DOCUMENTATION.
This gets rid of the "short cmd parameters", e.g. -w for windowed mode, and replaces
them with full name parameters, e.g. -p gfx.windowed=true, which improves handling
of configuration files/values. 
* Add a lot of unit tests to the codebase
* Refactored round plug security infrastructure, shared with IIDX and jubeat (1)
* Move shared utility modules between iidxhook modules to a separate utilty module 'iidxhook-util'
* Add experimental jubeat (1) support (buggy IO emulation) 
* Various fixes to improve all iidxhooks when running on Windows 10
* Bugfix: iidxio-ezusb getting stuck on newer Windows platforms
* Update iidxhook docs, e.g. how to get old IIDX versions sync on Windows 10
* Various documentation updates
* Various code cleanup
* Various other minor bugfixes

## v5.22
* Added a lot of documentation and readme stuff (READ IT!!11)
* Re-numbering iidxhook implementations to make room for missing games
* Support for IIDX 18 -> iidxhook4
* Support for IIDX 19 -> iidxhook5
* Support for IIDX 20 -> iidxhook6
* A lot of code refactoring and cleanup
* Refactor p3io emulation
* Remove obsolete BT4 DDR stuff
* iidxhook: Refactored software monitor check and bugfixes. You can use the
auto monitor check to determine your machine's refresh rate or (new) set the
refresh rate yourself in the configuration file, e.g. when you already have
determined it using one of the newer IIDX games and want to skip that monitor
check or if BT5's software monitor check doesn't work properly (e.g. on Win 7).
* iidxhook1-3: Check if eamuse server is reachable and log a warning otherwise.
This should make debugging invalid URLs or connection issues easier.
* iidxhook: Revised ezusb and ezusb2 emulation layer. Translucent support
removed, all IO emulation goes through BT5's iidxio interface.
* iidxio: Add implementations for ezusb (C02 IO) and ezusb2 (IO2) hardware. This
allows you to run _ANY_ IIDX game supported by BT5 either with real C02 or IO2
hardware.
* iidxhook: Remove translucent card reader feature. Again, to create a unified
interface for _ALL_ versions, use the eamio-icca.dll if you want to run on 
real ICCA (slotted or wavepass) readers. 
* Various other bugfixes

Again, read the various markdown (.md) readme files. We tried to document
everything to the best of our knowledge. If you are missing something, please
contribute by adding that information and submitting a patch to us.

## v5.21
* Camera hook for IIDX 25 (use any UVC webcam in-game), by Xyen
* *deep breath* Source code release

## v5.20
* Support for the new IIDX 25 IO board (xyen)
* New IO hook system with better multi-threading behavior
* Add a replaceable "vefxio" backend dll for IIDX (xyen)
* Card reader emulation can now be disabled in iidxhook (xyen)
* Add jbhook (xyen, mon)
* Add ddrhook (ported from Bemanitools 4 by mon)
* Various ICCA emulation improvements (xyen, mon)
* QoL improvements to config.exe (xyen, mon)
* Other bug fixes (various contributors)

## a19
* iidx 17 support
* Bugfix: forums.php?action=viewthread&threadid=51257&postid=1425861#post1425861
* iidx 14-17: Improved monitor check and new monitor check screen which shows 
the current frame rate instead of just a white screen
* iidx 09-13: Improved monitor check (but still white screen when in progress. 
d3d8 doesn't offer any text render out of the box)
* iidxfx(2)-exit-hook: Switch off lights on shutdown
* Various other bugfixes

## a18
* Bugfix: forums.php?action=viewthread&threadid=51063
* Various other bugfixes

## a17
* IIDX 16 support
* Fix broken debug output to file (for iidxhook1-3 and all games using launcher)
* Improve debug output
* Various minor bugfixes

## a16
* Add tools.zip which contains various tools for development: ezusb IO related, 
bemanitools API testing, acio related
* Add documentation (.md files) for tools
* Add iidxfx(2)-exit-hook.dll: Hook this using either inject or launcher 
(depending on the game version) and exit the game by pressing Start P1 + Start 
P2 + VEFX + Effect simultaneously
* Bugfix iidxio API: 16seg not working on IIDX games with FX2 emulation
* Bugfix iidxio API: return value of init call not getting checked in hook 
libraries
* SDVX input emulation fixes
* Various other bugfixes

## a15
* Select best network adapter if having multiple
* Add Felica card detection
* Fix SDVX HID lighting
* Fix IIDX FX2 deck lighting

## a13
* iidx 15 (DJ Troopers) support
* Fix BG video triangle seam on old games
* New options handling: cmd args and options file

## a11
* Fix nVidia crash on GOLD

## a10
* Adds IO2 emulation for Gold
* Add IO2 translucent mode for Gold ONLY atm (untested due to lack of hardware)
* Random input bug resolved (also kinda untested, so maybe?)

## a09
* Add IIDX 14 support

## a08
* Add experimental KFCA (SDVX PCB) support
* Add Sound Voltex and BeatStream builds

## a07
* Add IIDX 13 DistorteD support
* Add option to use real card readers with IIDX 13

## a06
* Fixes bug in chart data loader interception code

## a05
* Fix broken card reader emulation on Copula
* Add monitor check/auto timebase for old IIDX games (9-12) -> refer to the 
readme file on how to use it

## a04
* Add software frame rate limiter for all D3D8 based games (-g option).

## a03
* Add support for IIDX 9-12
* Translucent mode: Use real C02 EZUSB IO hardware
* eamio-real.dll: Use real slotted or wave pass card readers
* Setup guide, advanced features and FAQ: see readme file iidxhook1.md

## a02
* Fonts are always correct irrespective of system locale! (Make sure you 
install East Asian fonts tho)
* No longer crashes shitty gaming mice that don't follow the USB spec! (will 
backport this to bt4)
* launcher.exe now has a UAC manifest! (because 2006 called and told me to get 
with the fucking program)

## a01
* Initial Alpha, only supports IIDX 21 and 22 for now.