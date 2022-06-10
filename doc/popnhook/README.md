# popnhook

popnhook is a collection of hook libraries for jubeat providing
emulation and various patches to run these games on non BemaniPC hardware and
newer Windows versions.

The hook libraries must be bootstrapped either using [inject](../inject.md) or
[launcher](../launcher.md) depending on the version you want to run. Further
instructions are given in dedicated readme files for each popnhook version
(see below).

# Versions

popnhook comes in a few different flavors. The game and its engine changed over
the years. Some game versions might require patches/parameters enabled which
others don't need or have different AVS versions. Here is the list of supported
games:
* [popnhook1](popnhook1.md): 15 ADVENTURE, 16 PARTY♪, 17 THE MOVIE, 18 せんごく列伝

When building bemanitools, independent packages are created for each set of games
which are ready to be dropped on top of vanilla AC data dumps. We recommend
using pristine dumps to avoid any conflicts with other hardcoded hacks or
binary patches.

# How to run

To run your game with popnhook, you have to use the inject tool to inject the
DLL to the game process. `dist/popn` contains bat scripts with all the
important parameters configured. Further parameters can be added but might not
be required to run the game with default settings.
Further information on how to setup the data for each specific version are
elaborated in their dedicated readme files.

# Eamuse network setup

Running pop'n music 15 through 18? Modify the appropriate popnhook-15.conf, popnhook-16.conf,
popnhook-17.conf, or popnhook-18.conf.

Running anything newer?
* Open the `prop/ea3-config.xml`
* Replace the `ea3/network/services` URL with network service URL of your
choice (for example http://my.eamuse.com)
* Edit the `ea3/id/pcbid`

# Command line options

Add the argument *-h* when running inject with popnhook to print help/usage
information with a list of parameters you can apply to tweak various things.
