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