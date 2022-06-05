# ddrhook

ddrhook is a collection of hook libraries for "Dance Dance Revoluion" providing
emulation and various patches to run these games on non BemaniPC hardware and
newer Windows versions.

The hook libraries must be bootstrapped either using [inject](../inject.md) or
[launcher](../launcher.md) depending on the version you want to run. Further
instructions are given in dedicated readme files for each ddrhook version
(see below).

## Versions

ddrhook comes in a few different flavors. The game and its engine changed over
the years. Some game versions might require patches/parameters enabled which
others don't need or have different AVS versions. Here is the list of supported 
games:

* [ddrhook1](ddrhook1.md): X
* [ddrhook2](ddrhook2.md): X2, X3 vs. 2ndMIX, 2013, 2014, A

When building bemanitools, independent packages are created for each set of games
which are ready to be dropped on top of vanilla AC data dumps. We recommend
using pristine dumps to avoid any conflicts with other hardcoded hacks or
binary patches.

## How to run

To run your game with iidxhook, you have to use the inject tool to inject the
DLL to the game process. `dist/iidx` contains bat scripts with all the
important parameters configured. Further parameters can be added but might not
be required to run the game with default settings.
Further information on how to setup the data for each specific version are
elaborated in their dedicated readme files.

## Command line options

Add the argument *-h* when running inject with iidxhook to print help/usage
information with a list of parameters you can apply to tweak various things.

## ddrio API

Available implementations that can be swapped out depending on which kind of
IO hardware you want to use:

* `ddrio`: Default implementation supporting keyboard, mouse and USB
game controllers
* ddrio-mm: Support Minimaid custom interface
* [ddrio-smx](ddrhook/ddrio-smx.md): Support for StepManiaX dance platforms
