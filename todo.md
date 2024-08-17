# Bemanitools 6 TODOs

Lose list of TODOs/notes/thoughts that might or might not need to worked on. Some of these are
already sorted and moved to separate sub-sections outlining some kind of "roadmap".

## Alpha 4

* rename bt_module_configure_do to bt_module_configure_load as we don't want to imply to actually configure anything, but just load configuration
* use config-ext helpers in iidxconfig for dongle mcodes, pcbid, eamid
* have private verify functions in all config modules that check valid values and ranges
* update security/mcode with knowledge from arcade-docs -> see macros
* rethink a bunch of configuration values and if they even must be exposed, e.g. dongle mcode on ddrhook1 which
  must even have the same value to boot correctly
* have separate verify parameter functions in all config modules, see iidxhook-util/gfx-config
* have cleaner config defaults included in dist package here targeted for "living room arcade" use
  with sane defaults that have a high likelihood that the game just runs on most off-the-shelf hardware
* implement fini function in iidxhook2, 3, 4 etc. -> cleanup hook modules
* inline iidxhook-util d3d9 config stuff -> config class + module
* reflect optional in configuration API? it's quite a mess of using -1 and stuff to reflect that something is not there. with xml, the value can be kept empty simply to reflect that or leaving out the node entirely

goal: migrate and cleanup all old/inject (iidx)hooks

* deprecate cconfig and replace with core-config everywhere, e.g. in IO libs
* Support config api, required implementation for modules such as IO once being used, see modules/io-ext -> TODOs next to api inits

## Alpha 5 - command line stuff

* have some unified tooling in core for this? -> works well together with existing config API
* command line tooling unification: command arg parsing, common command args like setting log level/enabling/disabling log file
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

## Alpha ??? - cleanup pass + stabilize iidxhook

go through all iidxhook modules and check for stuff like TODOs or anything that 
could benefit from a little tidying up

* dissolve iidxhook-util and instead have earliest appearance and share with successive
modules
* consolidate loading of iidxio and eamio code in separate modules iidxio-module, eamio-module
in iidxhook1, then re-use on later iidxhooks

## Alpha ??? - inject native bemanitools 6 hook API support

* inject
  * Modify loading of hooks -> need to call btapi thread + log set functions somehow
  * Modify hooking to hook main_init and main_fini of hook before and after the main function of the exe
    * Load exe (PE) file, resolve references but don't call the start() function, yet
    * Run the start() function with hook main_init and main_finit before and after that -> issue if exe has nothing, not even heap etc. initialized?
    * Which functions can i use from hooklib for that? 
      * Requires writing our own PE loader for executables?
      * See hook/process -> process_hijack_startup and hook/pe 

bemanitools: new tool called “loader” that’s a PE loader with additional btools 6 api. also supports generic hooks with just DllMain as it supports loading of arbitrary DLLs. should work trapping BT ApI before start() function because DllMain methods were called even earlier already. just need to make sure BT hook stuff is called after the entire process is set up and all dependencies loaded

push back loader idea as inject still works fine with the debugger idea and everything else.
also check my own references and read up to refresh my mind how windows loads executables/PE files
regarding the stages/steps it takes before writing the bemanitools loader that replaces inject

* use new bemanitools hook api in iidxhooks 
  * replace inject logging through debug api with unified logging through bemanitools 6 api

## TBD/unsorted

* jbhook1: gfx hooks partially broken due to change in hooking setup/bootstrapping, see TODO in jbhook1
* sidcode backpropagation required and currently broken for sdvxhook2 and jbhook (?) at least (marked with TODO)
* meson as build system: requires getting rid of all the special AVS sauce in the current build system -> planned to load and resolve AVS libs dynamically, pre-requisite
* iat hook impl in launcher not complete and working, yet
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