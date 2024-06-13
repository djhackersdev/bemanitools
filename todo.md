# Bemanitools 6 TODOs

Lose list of TODOs/notes/thoughts that might or might not need to worked on. Some of these are
already sorted and moved to separate sub-sections outlining some kind of "roadmap".

## Alpha 2

goal: fix inefficient logging causing stuttering and unify across launcher/inject

new plan:
* bt-log as its own async logging engine with ring buffer
* log sink needs to be setup with a mutex to ensure log messages aren't split
  * since AVS and btools log engines are async then, this is fine
* can also simplify the whole logging API stuff since there is only one API impl
  needed

* avs log server in iidx-utils (and also for other games where needed?)
  move to bt-core and make it a generic and improved async log sink
* replace inject logging through debug api with unified logging through bemanitools 6 api
  * also solves issue with blocking logging on OutputDebugStr that needs synchronized

## Alpha 3

goal: migrate and cleanup iidxhooks

* use new bemanitools hook api in iidxhooks
* iat hook impl in launcher not complete and working, yet

## Alpha 4

goal: configuration unification and cleanup, iidxhooks

* deprecate cconfig and replace with core-config everywhere, e.g. in IO libs
* command line tooling unification: command arg parsing, common command args like setting log level/enabling/disabling log file
* Support config api, required implementation for modules such as IO once being used, see modules/io-ext -> TODOs next to api inits
* command line overrides for hook (and other?) configurations managed in
  inject and launcher, transparent to hook which just gets the final config
* improve command line tool/usage docs by following the man page style format with sections name, synopsis description, options etc.
* improve config defaults, e.g. on old iidx games: have various gfx fixes enabled by default, cause disabling is rather the exception, e.g. running on actual old hardware that doesn't have these issues
* have a library that provides global options for any
  command line tool such as setting log level, log file etc.
  and have a piece of code allowing for quick and simply usage everywhere -> unify command line args
* support command lines with paths, i.e. -P launcher.hooks.bla.gfx.windowed
  as well as allowing hooks and other things to define arbitrary short versions
  e.g. -w for windowed

## TBD/unsorted

* dynamically load AVS to get rid of all the static linking and different AVS
  version builds -> cleans up build process significantly and also makes
  switching to a new/different build system easier
* document life cycle of hooks, when they happen, what gets called, what to do and what not to do -> document this in the API + overview with references from some sdk docs
  *  dll load: DllMain
  *  ... refer to hook API calls
* documentation as a rendered page on bemanitools.github.io domain
  * Design and style like sapling documentation
* think about splitting mono hooks into smaller ones. iidx is probably a good
  example because stuff is being re-used everywhere, e.g. gfx stuff?
* group all hooks in its own subfolder -> name them all with a "hook" postfix?
* procmon: lib avs monitor
* make vtables for core thread and log
* for io api
  * use structs with bit fields everywhere to avoid all the bit shifting etc
  * align several IO interfaces regarding separate polling (read/write) calls etc
  * make them work closer to the actual device IOs, e.g. ddr with different sensor modes polling vs individual 
  * have all IO APIs in the style of
    * setters + getters: modify  internal state, but don't transfer to hardware/device, yet
    * transactional functions: read/write (or send/recv) or update to execute a transaction to the hardware. this also can be a stubbed function that doesn't do anything if the backend is async driven but the API is not exposing that and looks like a sync API 
  * split iidxio into vefx and control deck parts -> have this as a separate library that loads two libs and composes them to the iidxio one?
* btapi design choice: have return types always be a status code and use parameter pointers to return actual data, e.g. for getting IO values
* io/popn: doesn't have read/write (poll) calls to explicitly drive the hardware
* have command in build script/justfile to create a package that can be used as a standalone project
  to develop for bemanitools using the sdk
* Have a version.h/version.c for every dll/exe in bemanitools, see launcher. Print these at the first possible occasion when logging is available to console
* have shim dlls in sdk (?) that create backward compatibility with
  BT5 equivalent DLLs but only support the api v1 of BT6
* make vefx.dll optional in iidxio.dll with geninput.dll
* iidx io emulation is not getting values "atomic" from iidxio API
  see the various msg implementations which hook the API
  for example
  msg_resp.inverted_pad = ((iidx_io_ep2_get_keys() & 0x3FFF) << 16) |
        (iidx_io_ep2_get_panel() & 0x0F) |
        ((iidx_io_ep2_get_sys() & 0x07) << 4) |
        (((iidx_io_ep2_get_sys() >> 2) & 0x01) << 30);
      -> iidx_io_ep2_get_sys is called twice
* notes for migration guide from bemanitools 5 to 6
  * Delete config.exe configurations in appdata: C:\Users\<USERNAME>\AppData\Roaming\DJHACKERS\iidx.bin
  * incompatibility list to BT5
    * All IO libs
    * hooks?
    * configuration files .conf
    * previous setups with eapki data, keep stuff stock now and use bootstrap.xml provided by btools or stock bootstrap.xml    
* config tool broken -> various functions from geninput need
  to be loaded in addition to the input interface
* rteffects stubs don't seem to work, iidxhook1+2 
* split all games and versions for distribution files to distribution packages for each game version. avoids having a mixed bag of multiple versions with different startup scripts and sometimes oneofs, e.g. one specific lib for one version.
* for iidx e,f settings folders (old games), have default configuration values allocate them outside of the revision dirs, e.g. under settings/save next to the bemanitools folder to keep revision folders clean
* similar to trace wrapper for core api, have performance measuring around IO api
* reverse engineer NvDisplayConfigLDJ and make bemanitools version of that
* bemanitools performance monitoring of IO and rendering loops, measure and expose metrics alert on flaky performance
* tool to easily read and change xml properties configs from command line, similar to jq, but a lot simpler. no query language, just simple get and put
* moving build system to meson once AVS libs are dynamically loaded