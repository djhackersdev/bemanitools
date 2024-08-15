# Release history

Note for CI/CD: Ensure the version formatting in the sections is kept identical to the versions
given in tags. The pipeline will pick this up and cuts out the relevant section for release notes.

## 6.0.0-alpha.3

**THIS IS A HIGHLY WORK/DEVELOPMENT IN PROGRESS VERSION**

**THINGS ARE BROKEN AND EVERYTHING IS SUBJECT TO CHANGE**

Goals of this version:

* property (node) API implementation without AVS -> [mxml](https://github.com/michaelrsweet/mxml)
* inject refactoring similar to launcher
  * Re-structure modules
  * Use bemanitools config API backed by property (node) API with mxml (because no AVS2)
* SDK foundation for hook libraries bootstrapping with DllMain
* iidxhook1 refactoring to use new SDK hook foundation and bemanitools config API

This brings inject and launcher closer together regarding common functionality. inject is also
using a "bootstrap.xml" style approach with a XML configuration file. That configuration file
can also include further inlined or referenced configuration for any hooks to be loaded.

With a new module that will serve as a piece of a public SDK in the future, any library that just
bootstraps with DllMain can implement the Bemanitools 6 hook API transparently. This solution
isn't ideal, but it works with a bit of duct-tape fairly well. A future improve would be to have
improved hooking in inject to have a pre- and post-main function hook. However, that is a lot more
work as it requires our own PE loader to bootstrap the entire executable to get more fine grained
control.

Remarks regarding what's known to be broken: Inject command line args + overrides

## 6.0.0-alpha.2

**THIS IS A HIGHLY WORK/DEVELOPMENT IN PROGRESS VERSION**

**THINGS ARE BROKEN AND EVERYTHING IS SUBJECT TO CHANGE**

Goal of this version: Fix blocking logging which results in performance impact on many games using
both launcher and inject. The impact typically materialized as the game stuttering, briefly slowing
down, music de-syncing or occasional laggy input.

This was a fundamental issue to the previous logging architecture in bemanitools. With the pre-work
of having a more modular logging API that can be shared and used across internal components as well
as public APIs including hooks, unified logging can be applied throughout the bemanitools stack.

With this, all logging is channeled to a single logger, the bemanitools logger, where we have full
control over how we log any log output (but also full control of messing it up =)).

With this, we can also move away from the band aid of the log-server that's been used in iidx
versions 19 to 24 because non AVS threads logging through the AVS logging system were crashing (see
[this document](doc/dev/journal/2018-02-10-logging-breakdown-avs.md) for details).

A new async log sink takes care of de-coupling the non blocking producing/posting of messages to
a configurable queue by any number of threads concurrently, with a single producer thread consuming
these messages from the queue and forwarding them to a configured target sink, e.g. file, terminal.

With this, we ensure that threads with log messages don't have to wait for synchronization on 
expensive and slow IO.

These changes have been applied to to launcher and inject. The log-server module has been removed
entirely from any (iidx) games using it.

## 6.0.0-alpha.1

**THIS IS A HIGHLY WORK/DEVELOPMENT IN PROGRESS VERSION**

**THINGS ARE BROKEN AND EVERYTHING IS SUBJECT TO CHANGE**

First cut after massive refactoring with most critical and fundamental changes implemented.
We need to start somewhere, and this might be as good as anything else, so we can get started
with testing, bug fixing and iterating for the next releases.

The following list is non-exhaustive, does not guarantee anything does work, yet, and is supposed
to give a high level idea of what all of this is about. Updated documentation will reflect all of
this at some later point in time in more detail.

* A common "core" now abstracts logging, thread, property and configuration infrastructure and
  provides a common interface. This is used by bemanitools internally as well as all tools, hooks
  and APIs provided and don't depend on the game, version of the game or AVS version available
  anymore
* New bemanitools (public) API
  * Versioned API allowing for handling incremental API changes as well as breaking changes by
    providing a new/different version when necessary
  * Unified interfaces for bemanitools core API, i.e. logging, threads, configuration
  * SDK with examples (TBD)
  * Dogfooding approach: Bemanitools uses its own (public) API to implement and provide fundamental
    features like configurable keyboard implementations for IO or hooks for different games and
    versions
* All bemanitools hooks and IO libraries have been or are about to be re-worked to use the new APIs
* New hook API allows for more fine grained runtime control when stages of the hook are to be
  executed, i.e. pre AVS, before main game, iat hooking instead of relying purely on DllMain
  (which is still a compatible option though)
* launcher as a replacement for bootstrap: Bring it significantly closer to the original bootstrap
  by supporting completely vanilla data and bootstrap.xml configurations to run the games. Note
  that bemanitools does not include any code or means to run DRM'd data, only decrypted
* inject is also being reworked to use as much of the same "infrastructure" as launcher to provide
  a more seamless bootstrapping process for games that keeps pre-eapki data as vanilla as possible

### What works

* inject: Basic functionality, launches games iidx 09 to 17 with iidxhook1 to iidxhook4
* launcher: Basic functionality, launches games iidx 09 to 17 with iidxhook5 to iidxhook7
* iidxhook1: Boots, reaching in-game with keyboard inputs working
* iidxhook2: Boots, reaching in-game with keyboard inputs working
* iidxhook3: Boots, reaching in-game with keyboard inputs working
* iidxhook4: Boots, reaching in-game with keyboard inputs working
* iidxhook5: Boots, reaching in-game with keyboard inputs working
* iidxhook6: Boots, reaching in-game with keyboard inputs working
* iidxhook7: Boots, reaching in-game with keyboard inputs working

### What's broken

Since everything else is untested, consider it (very) broken

## 5.49

### Features

### Fixes

## 5.48

### Features

- feat(config): Add invert analog input option. Remark: The old configuration file format is
  upgraded but downgrading is not possible anymore. The new saves are using a new filename and it
  doesn't delete the old files, see `C:\Users\<YOUR_USERNAME>\AppData\Roaming\DJHACKERS`

### Fixes

## 5.47

### Features

N/A

### Fixes

- fix(ddr/p3ioemu): Handle unknown 2B command to fix DDR X IO errors
- fix(ddr/p3io): Crash on all supported DDR games due to incorrect p3io message size validation
- fix(geninput): Sony DualShock 4 not working

## 5.46

### Features

- feat(ddrio): Wrapper/shim library to drive another ddrio in a dedicated IO thread. Improves
  performance for highly IO bound ddrio implementations, e.g. ddrio-p3io
- feat(iidxiotest): bi2a-iidx support

### Fixes

- fix(jb03/04): Missing XML in gamestart script
- fix(jbhook1): Rotate error message box when screen is rotated
- fix(jbhook1): Improve/fix automatic config/nvram fs initialization

## 5.45

### Features

- feat(iidx 30): Added support
- feat(ddr): Add ddrio implementation with P3IO driver backend
- feat(ddr): Add p3io-ddr-tool for testing/debugging real P3IO devices with EXTIO
- feat(ddr): Add testing tool for real EXTIO devices
- feat(ddr): Add extio driver
- feat(ddr): Add p3io driver
- feat(iidx 20-26): New/fixed gfx up-/downscaling feature, old one was broken
- feat(ddr): Vigem driver for ddrio
- feat(iidx 25-30): The user can now set a custom camera device override of "SKIP" to leave that
  camera unassigned

### Fixes

- fix(iidx 09/10): Fix stretched BGAs on 1st and 3rd style videos

## 5.44

### Features

- feat(doc): ezusb dev journal entry, 10th style ezusb boot up and security setup
- feat(iidx 09/10): ezusb API call monitoring module and configuration flag
- feat(iidx 19): Add back btools monitor check for iidxhook5
- feat(iidx 9-19): Add configurable settings path hook. Redirect settings paths, i.e. `d/`, `e/` and
  `f/` drive "folders" to an arbitrary path on any partition, e.g. writable partition
- feat(iidx 9/10): Improve commandline script for irbeat-patch 09 and 10, add interactive mode

### Fixes

- fix(iidx 10): SQ-INIT error caused by incorrect security configuration
- fix(iidx 9-24): Wire up coin input in ezusb1/2 IO emulation layer
- fix(pnm 15-18): Fix IO buffer inconsistency causing random input misfiring
- fix(jb 1-8): Fix IO buffer inconsistency causing random input misfiring
- fix(iidx 9-24): Fix IO buffer inconsistency causing random input misfiring
- fix(iidx 19): imagefs override strategy with local file redir. Required to enable monitor-check on
  all chart files
- fix(iidx 9-19): Code synchronization issue causing deadlock resulting in game hang/stuck on boot
- fix(iidx 9-19): Fix broken settings path handling with mixed / and \\
- fix(iidx 27): Align disabled cam connection config defaults with 28/29
- fix(inject): Fix missing forwarding of exit code
- fix(inject): Getting stuck when game exits cleanly due to locked up debugger thread.
- fix(iidx 18/19 CN): gamestart.bat creating dev/nvram and dev/raw folders

## 5.43

- iidx29: (Officially) support IIDX CASTHOUR
- Code structure maintenance
- Various documentation improvements

## 5.42

- Bugfix: Fix diagonal texture tearing on IIDX 18 and 19 based games
- Feature: Add support for IIDX Resort Anthem CN using iidxhook4-cn
- Feature/bugfix: Enable "nvidia fix" for iidxhook4, 5 and 5-cn. Should fix crashing on some/all
  nvidia cards
- Various minor fixes

## 5.41

- Feature: Support pop'n music 15-18 using popnhook1
- Feature: Support IIDX tricoro CN using iidxhook5-cn
- Various documentation improvements

## 5.40

- Feature: Support DDR X and X2 US/EU using ddrhook1 including memory card emulation
- Fix: sdvxhook2 crashing on some OS versions
- Feature: Support sdvx generator lights in sdvxio

## 5.39

- jbhook3: Fix window mode, remove window frame option, add show cursor on window option
- Code simplification and cleanup round plug modules
- Code structure improvements p3io jubeat

## 5.38

- sdvxhook2: Support sound voltex exceed gear

## 5.37

- jbhook1: Support jubeat (1) and ripples
- jbhook2: Support jubeat knit and copious
- jbhook3: Support jubeat saucer, prop, qubell, clan and festo

## 5.36

- iidxhook9: Support for IIDX28
- Various bugfixes

## 5.35

- Jubeat: Add jbio-p4io
- Nostalgia: Add nostio-panb
- Misc: Add wavepass support to aciodrv-icca (and eamio-icca)
- Misc: Refactor aciodrv to support multiple devices using the same port (with aciomgr)
- Misc: Fix d3d9ex windowed hook not behaving correctly (size/framed)
- Misc: Fix docs not being included with release
- Various bugfixes

## 5.34

- IIDX: Support IO and card reader feature switches to disable emulation in iidxhook4-7
- Jubeat: jbio implementation for magicbox hardware
- Misc: Various documentation improvements
- IIDX: Turntable multiplier (can also be used as inverted) for iidxhook9
- IIDX: iidxhook9, add force screen resolution option to fix sometimes wrong auto res detection by
  iidx27 causing inaccurate/wrong refresh rates on monitor check
- IIDX: iidxio BIO2 implementation -> Use BIO2 hardware with any iidxhook or other software
  supporting BT5's iidxio interface
- IIDX: BIO2 exit hook, exit the game pressing Start P1 + Start P2, Effect and VEFX
- launcher: Allow overriding service URL from command line
- IIDX: vigem-iidxio, tool to allow using iidxio (ezusb, ezusb2 or bio2) to emulate XBOX controllers
  to play other non arcade games that support xinput, e.g. Lunatic Rave (note: Infinitas does not
  work with this since it only supports direct input)

## 5.33

- iidxhook9: Support for IIDX27
- sdvxio-bio2: Driver for real sdvx5 cabinet IO hardware
- vigem-sdvxio: Tool to allow using sdvxio (kfca or bio2) to emulate an xbox controller
- Various bugfixes and refactoring for inject and launcher

## 5.32

- Various bugfixes

## 5.31

- DDR: Fix p3io and extio lights not working / being swapped when using geninput
- DDR: Add HDXS light support
- DDR: Add 64 bit support
- DDR: Add option to use COM4 as the p3io rs232 port instead of emulating (for use with acrealio
  etc.)
- Jubeat: Add network adapter hook
- Misc: Fix aciodrv sometimes hanging on boot (eamio-icca and sdvxio-kfca)

## 5.30

- SDVX: sdvxhook2 headphone force and cursor confining config options
- DDR: Add light support for SMX gen 4 pads

## 5.29

- SDVX: Add option for monitor rotation
- SDVX: Add option to allow overriding network adapter IP
- IIDX: Add force display adapter and refresh rate configuration
- SDVX: Allow specifying display adapter to open for d3d9ex hook
- IIDX/SDVX camerahooks bugfix: Camera sometimes not detected on some older machines

## 5.28

- Improve documentation in various places
- Automatic code formatting in build pipeline
- Bugfix: Recursive config.bat script
- Enable automatic building on pushes to master
- Bugfix: Stop SDVX hanging on thankyou for playing
- sdvx2: Support for Vivid Wave using sdvxhook2
- sdvxio-kfca: Driver for real sdvx cabinet IO hardware

## 5.27

- Various documentation updates regarding development and how to contribute
- KFCA emulation accuracy fixes
- Fix passthrough of serial calls when using real hardware (e.g. card reader passthrough
  IIDX25+/SDVX5+)
- Add sdvxio-kfca driver to talk to real SDVX hardware
- Refactor d3d9 hook module to improve maintainability. Also fixes upscaling not working on IIDX20+.
- Remove d3d8 hook module to improve maintainability. Make old d3d8 based games, e.g. iidxhook1/2,
  use d3d9 instead. Requires usage of d3d8to9 wrapper library to use gfx patching features.
- Add dev doc about IIDX rendering loops
- Add an initial draft of an architecture document (will be worked on in iterations)
- Refactor BIO2 emulation to make it re-usable by other games

## 5.26

- iidxio-ezusb: Reduce sleep time to Sleep(1) to avoid framerate issues on some versions of iidx.
- Bugfix: iidx d3d9 games, mainly the newer ones iidx 20-25, freezing. Happened on boot, during song
  selection or during the song.
- Bugfix: log_server_init deadlock on iidxhook4-7. This caused games like iidx24 to hang before even
  showing a render window.
- iidxhook: Add feature to allow GPU based up-/downscaling of rendered frame. This gives you the
  possibility to upscale the resolution of old SD (640x480) games to your monitor's/TV's native
  resolution which can have a few advantages: better image quality if the monitor's upscaler is not
  doing a good job, especially on resolutions that are not a multiple of its native resolution;
  Reduce display latency if the upscaler is slow, avoid over-/underscan which cannot always be fixed
  entirely or at all (depending on your GPU and monitor model). See the iidxhook configuration file
  for the new parameters available.
- Major readme cleanup. Add development documentation like style guide, guidelines, development
  setup.

## 5.25

- Bugfix: iidx14 and 15 crashing on Windows 10
- Bugfix: IO2 driver not using correct package sizes on reads/write -> iidxio-ezusb2.dll now working
- Improve ezusb2-boot.bat script to handle flashing of IO2 firmware
- Remove broken x64 builds of ezusb1/2 tools -> Just use the x86 tool versions instead (use the x64
  versions of iidxio-ezusb.dll and iidxio-ezusb2.dll with iidx25)
- iidxhook1-8: Allow floating point values for frame rate limiting, e.g. 59.95 (hz).
- Improve timing with ezusb (C02) driver

## v5.24

- Bugfix: iidxhook8 hangs very early on startup (race condition in log-server module)

## v5.23

- Refactored configurion (file) handling for iidxhooks, RE-READ THE DOCUMENTATION. This gets rid of
  the "short cmd parameters", e.g. -w for windowed mode, and replaces them with full name
  parameters, e.g. -p gfx.windowed=true, which improves handling of configuration files/values.
- Add a lot of unit tests to the codebase
- Refactored round plug security infrastructure, shared with IIDX and jubeat (1)
- Move shared utility modules between iidxhook modules to a separate utilty module 'iidxhook-util'
- Add experimental jubeat (1) support (buggy IO emulation)
- Various fixes to improve all iidxhooks when running on Windows 10
- Bugfix: iidxio-ezusb getting stuck on newer Windows platforms
- Update iidxhook docs, e.g. how to get old IIDX versions sync on Windows 10
- Various documentation updates
- Various code cleanup
- Various other minor bugfixes

## v5.22

- Added a lot of documentation and readme stuff (READ IT!!11)
- Re-numbering iidxhook implementations to make room for missing games
- Support for IIDX 18 -> iidxhook4
- Support for IIDX 19 -> iidxhook5
- Support for IIDX 20 -> iidxhook6
- A lot of code refactoring and cleanup
- Refactor p3io emulation
- Remove obsolete BT4 DDR stuff
- iidxhook: Refactored software monitor check and bugfixes. You can use the auto monitor check to
  determine your machine's refresh rate or (new) set the refresh rate yourself in the configuration
  file, e.g. when you already have determined it using one of the newer IIDX games and want to skip
  that monitor check or if BT5's software monitor check doesn't work properly (e.g. on Win 7).
- iidxhook1-3: Check if eamuse server is reachable and log a warning otherwise. This should make
  debugging invalid URLs or connection issues easier.
- iidxhook: Revised ezusb and ezusb2 emulation layer. Translucent support removed, all IO emulation
  goes through BT5's iidxio interface.
- iidxio: Add implementations for ezusb (C02 IO) and ezusb2 (IO2) hardware. This allows you to run
  _ANY_ IIDX game supported by BT5 either with real C02 or IO2 hardware.
- iidxhook: Remove translucent card reader feature. Again, to create a unified interface for _ALL_
  versions, use the eamio-icca.dll if you want to run on real ICCA (slotted or wavepass) readers.
- Various other bugfixes

Again, read the various markdown (.md) readme files. We tried to document everything to the best of
our knowledge. If you are missing something, please contribute by adding that information and
submitting a patch to us.

## v5.21

- Camera hook for IIDX 25 (use any UVC webcam in-game), by Xyen
- *deep breath* Source code release

## v5.20

- Support for the new IIDX 25 IO board (xyen)
- New IO hook system with better multi-threading behavior
- Add a replaceable "vefxio" backend dll for IIDX (xyen)
- Card reader emulation can now be disabled in iidxhook (xyen)
- Add jbhook (xyen, mon)
- Add ddrhook (ported from Bemanitools 4 by mon)
- Various ICCA emulation improvements (xyen, mon)
- QoL improvements to config.exe (xyen, mon)
- Other bug fixes (various contributors)

## a19

- iidx 17 support
- Bugfix: forums.php?action=viewthread&threadid=51257&postid=1425861#post1425861
- iidx 14-17: Improved monitor check and new monitor check screen which shows the current frame rate
  instead of just a white screen
- iidx 09-13: Improved monitor check (but still white screen when in progress. d3d8 doesn't offer
  any text render out of the box)
- iidxfx(2)-exit-hook: Switch off lights on shutdown
- Various other bugfixes

## a18

- Bugfix: forums.php?action=viewthread&threadid=51063
- Various other bugfixes

## a17

- IIDX 16 support
- Fix broken debug output to file (for iidxhook1-3 and all games using launcher)
- Improve debug output
- Various minor bugfixes

## a16

- Add tools.zip which contains various tools for development: ezusb IO related, bemanitools API
  testing, acio related
- Add documentation (.md files) for tools
- Add iidxfx(2)-exit-hook.dll: Hook this using either inject or launcher (depending on the game
  version) and exit the game by pressing Start P1 + Start P2 + VEFX + Effect simultaneously
- Bugfix iidxio API: 16seg not working on IIDX games with FX2 emulation
- Bugfix iidxio API: return value of init call not getting checked in hook libraries
- SDVX input emulation fixes
- Various other bugfixes

## a15

- Select best network adapter if having multiple
- Add Felica card detection
- Fix SDVX HID lighting
- Fix IIDX FX2 deck lighting

## a13

- iidx 15 (DJ Troopers) support
- Fix BG video triangle seam on old games
- New options handling: cmd args and options file

## a11

- Fix nVidia crash on GOLD

## a10

- Adds IO2 emulation for Gold
- Add IO2 translucent mode for Gold ONLY atm (untested due to lack of hardware)
- Random input bug resolved (also kinda untested, so maybe?)

## a09

- Add IIDX 14 support

## a08

- Add experimental KFCA (SDVX PCB) support
- Add Sound Voltex and BeatStream builds

## a07

- Add IIDX 13 DistorteD support
- Add option to use real card readers with IIDX 13

## a06

- Fixes bug in chart data loader interception code

## a05

- Fix broken card reader emulation on Copula
- Add monitor check/auto timebase for old IIDX games (9-12) -> refer to the readme file on how to
  use it

## a04

- Add software frame rate limiter for all D3D8 based games (-g option).

## a03

- Add support for IIDX 9-12
- Translucent mode: Use real C02 EZUSB IO hardware
- eamio-real.dll: Use real slotted or wave pass card readers
- Setup guide, advanced features and FAQ: see readme file iidxhook1.md

## a02

- Fonts are always correct irrespective of system locale! (Make sure you install East Asian fonts
  tho)
- No longer crashes shitty gaming mice that don't follow the USB spec! (will backport this to bt4)
- launcher.exe now has a UAC manifest! (because 2006 called and told me to get with the fucking
  program)

## a01

- Initial Alpha, only supports IIDX 21 and 22 for now.
