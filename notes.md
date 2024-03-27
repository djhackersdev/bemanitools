* document life cycle of hooks, when they happen, what gets called, what to do and what not to do -> document this in the API + overview with references from some sdk docs
  *  dll load: DllMain
  *  ... refer to hook API calls
* documentation as a rendered page on bemanitools.github.io domain
  * Design and style like sapling documentation
* support command lines with paths, i.e. -P launcher.hooks.bla.gfx.windowed
  as well as allowing hooks and other things to define arbitrary short versions
  e.g. -w for windowed
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