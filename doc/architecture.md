# Bemanitools architecture
This document gives you an overview of the architecture of Bemanitools (5). Why do we need this? This helps document the
various design decisions and how everything comes together, in the end. At some point when things are evolving and other
developers want to pick up the project, this might give them answers to why some things were done well or not so well.
This allows them to iterate on the current design (document) to analyze if a change they want to apply might work out
or not.

Anway, enough preface...I guess you got the idea. This document will be split into several sections which address
different aspects of the architecture.

## The big picture
TODO create a graphic that presents the key modules and ideas.

## Detouring library functions, IAT hooking
One of Bemanitools's goals is to avoid patching of executables, libraries or any game data and instead rely on
intercepting calls to libraries to patch bugs or introduce new features. To support a game on a non native platform
transparently, e.g. your home desktop, this allows us to intercept I/O communication to emulate hardware or
files/filesystem features the game expects to be available.

We create a so called "hook library/hook.dll" which gets injected to the target process (the game) before the game
runs any of its application code. This is well known by the terms of dll injection and will not be further discussed
here (google it). The injected dll will replace the function addresses in the IATs to detour any calls to our own
handler functions before, eventually, calling the real function. This way, we can intercept the call and do cool things.

These features are covered by the modules in the following subfolders in src/main:
* hook: Essentially, this is capnhook: https://github.com/decafcode/capnhook. General tools for hooking Win32 API calls.
* hooklib: Some additional helper modules to take care of specific issues in Bemanitools, e.g. rs232 related stuff for
ACIO.

Check the modules for details.

## Hooking and IRP
IRP stands for "I/O request packet" and is a kernel mode structure used in Windows drivers for communication with the
OS. The data structure describes an I/O request with parameters for that request avoiding function calls with large
number of arguments to a driver.

We make use of that "IRP pattern" by creating a flexible and maintainable abstraction layer for the following hooking
modules:
* iohook: Hook I/O (e.g. file) related calls
* d3d9: Hook d3d9 graphics API calls

An IRP handler is implemented to handle selected IRP calls based on the specified operation type. IRP handlers can be
chained which allows splitting up different features/interceptors to different functions and modules allowing you to
create a clear structure. At the end of the forwarding chain, you the real API function that maps to the abstracted
operation is called. However, a handler can decide at any time to not forward calls which allows you to implement
emulation of access to selected files or I/O devices.

Modules that make use of this:
* iidxhook-util/d3d9
* ezusb-emu/device
* ezusb2-emu/device
* acioemu/emu

## Bemanitools's hook libraries, let's glue everything together 
Bemanitools dlls to be injected into target game processes are refered to as "hook dlls" and come in different flavours
targetting different games and often different versions of the same game (series), for example:
* ddrhook: Hook dll for Dance Dance Revolution games
* iidxhook1-8: Hook dlls for Beatmania IIDX, we are currently at 8 different implementations due to the various
iterations the game went through, related to software and hardware.
* sdvxhook: Hook dll for SoundVoltex series

The following sub-sections will give you some brief insights on each hook implementation and what modules were used.

### bsthook
TODO

### ddrhook
TODO

### iidxhook
IIDX went through so many hard- and software iterations, it's actually amazing that the development team(s) refactored
and improved parts of the game and hardware with each iteration. However, when facing emulation and supporting 
compatibility to legacy OS platforms, it can't get any worse. On the bright side, IIDX helped shaping Bemanitools a lot
and created a solid foundation other games can build on.

Because of that, we have 8 iidxhook implementations supporting sometimes different software features/fixes and
hardware. The following sub-sub-sections list the most relevant aspects and modules to point out common and different
higher level features. 

Essentially, the main module file of each iidxhook implementation just glues the APIs of the modules it requires
together. An additional configuration layer allows users to tweak some of the features.

#### iidxhook1 (9-12)
* Ezusb C02 I/O emulation
    * Setupapi emulation
    * Full security emulation with SRAM and round plugs
    * Full serial emulation for magstripe card readers
    * Full game essential I/O emulation 
* d3d8 patching and extended features (superseded by d3d9 hook module + d3d8to9 wrapper)
* clock patching
* Font patching for Japanese chars
* Network related patches to enable eamuse to custom servers
* Filesystem patches to detour E and F backup drives for settings

#### iidxhook2 (13)
* Ezusb C02 I/O emulation
    * Setupapi emulation
    * Full security emulation with SRAM and round plugs
    * Full game essential I/O emulation 
* ACIO ICCA card reader emulation, slotted readers
* d3d8 patching and extended features (superseded by d3d9 hook module + d3d8to9 wrapper)
* clock patching
* Font patching for Japanese chars
* Network related patches to enable eamuse to custom servers
* Filesystem patches to detour E and F backup drives for settings

#### iidxhook3 (14-17)
* Ezusb IO2 I/O emulation
    * Setupapi emulation
    * Full security emulation with SRAM and round plugs
    * Full game essential I/O emulation 
* ACIO ICCA card reader emulation, slotted readers
* d3d9 patching and extended features
* Font patching for Japanese chars
* Network related patches to enable eamuse to custom servers
* Filesystem patches to detour E and F backup drives for settings

#### iidxhook4 (18)
* Ezusb IO2 I/O emulation
    * Setupapi emulation
    * Full game essential I/O emulation 
* ACIO ICCA card reader emulation, slotted readers
* d3d9 patching and extended features
* Font patching for Japanese chars
* Filesystem patches to detour E and F backup drives for settings

#### iidxhook5 (19)
* Ezusb IO2 I/O emulation
    * Setupapi emulation
    * Full game essential I/O emulation 
* ACIO ICCA card reader emulation, wave pass readers
* d3d9 patching and extended features
* Font patching for Japanese chars
* Filesystem patches to detour E and F backup drives for settings

#### iidxhook6 (20)
* Ezusb IO2 I/O emulation
    * Setupapi emulation
    * Full game essential I/O emulation 
* ACIO ICCA card reader emulation, wave pass readers
* d3d9 patching and extended features
* Font patching for Japanese chars

#### iidxhook7 (21-24)
* Ezusb IO2 I/O emulation
    * Setupapi emulation
    * Full game essential I/O emulation 
* ACIO ICCA card reader emulation, wave pass readers
* d3d9 patching and extended features
* Font patching for Japanese chars

#### iidxhook8 (25-26)
* ACIO BIO2 I/O emulation
    * Setupapi emulation
    * Full game essential I/O emulation 
* ACIO ICCA card reader emulation, wave pass readers
* d3d9 patching and extended features
* Font patching for Japanese chars

### jbhook
TODO

### sdvxhook
TODO

## The ezusb (emulation) stack
TODO

## The ACIO (emulation) stack
TODO

## BT5 API
TODO

## AVS and launcher
TODO

## Inject
TODO