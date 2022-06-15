# iidxhook
iidxhook is a collection of hook libraries for BeatmaniaIIDX providing
emulation and various patches to run these games on non BemaniPC hardware and
newer Windows versions.

The hook libraries must be bootstrapped either using [inject](../inject.md) or
[launcher](../launcher.md) depending on the version you want to run. Further
instructions are given in dedicated readme files for each iidxhook version
(see below).

## Versions
iidxhook comes in a few different flavors. The game and its engine changed over
the years. Some game versions might require patches/parameters enabled which
others don't need or have different AVS versions. Here is the list of supported 
games:
* [iidxhook1](iidxhook1.md): 9th, 10th, RED, HAPPY SKY
* [iidxhook2](iidxhook2.md): DistorteD
* [iidxhook3](iidxhook3.md): GOLD, DJ TROOPERS, EMPRESS, SIRIUS
* [iidxhook4](iidxhook4.md): Resort Anthem
* [iidxhook4-cn](iidxhook4-cn.md): Resort Anthem CN (狂热节拍 IIDX)
* [iidxhook5](iidxhook5.md): Lincle
* [iidxhook5-cn](iidxhook5-cn.md): tricoro CN (狂热节拍 IIDX 2) 
* [iidxhook6](iidxhook6.md): Tricoro
* [iidxhook7](iidxhook7.md): SPADA, PENDUAL, copula, SINOBUZ
* [iidxhook8](iidxhook8.md): CANNON BALLERS, Rootage
* [iidxhook8](iidxhook9.md): Heroic Verse

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

## iidxio API
Available implementations that can be swapped out depending on which kind of
IO hardware you want to use:
* `iidxio`: Default implementation supporting keyboard, mouse and USB
game controllers
* [iidxio-bio2](iidxhook/iidxio-bio2.md): Support BIO2 hardware
* [iidxio-ezusb](iidxhook/iidxio-ezusb.md): Support C02 ezusb FX hardware
* [iidxio-ezusb2](iidxhook/iidxio-ezusb2.md): Support IO2 ezusb FX2 hardware
