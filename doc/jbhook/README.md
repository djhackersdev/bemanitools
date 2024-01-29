# jbhook

jbhook is a collection of hook libraries for jubeat providing emulation and various patches to run
these games on non BemaniPC hardware and newer Windows versions.

The hook libraries must be bootstrapped either using [inject](../inject.md) or
[launcher](../launcher.md) depending on the version you want to run. Further instructions are given
in dedicated readme files for each jbhook version (see below).

# Versions

jbhook comes in a few different flavors. The game and its engine changed over the years. Some game
versions might require patches/parameters enabled which others don't need or have different AVS
versions. Here is the list of supported games:

- [jbhook1](jbhook1.md): jubeat, ripples
- [jbhook2](jbhook2.md): knit, copious
- [jbhook3](jbhook3.md): saucer, prop, qubell, clan, festo

I sure have trouble remembering which jubeat version is which, here's a handy guide:

- 01 - jubeat
- 02 - ripples
- 03 - knit
- 04 - copious
- 05 - saucer
- 06 - prop
- 07 - qubell
- 08 - clan
- 09 - festo

When building bemanitools, independent packages are created for each set of games which are ready to
be dropped on top of vanilla AC data dumps. We recommend using pristine dumps to avoid any conflicts
with other hardcoded hacks or binary patches.

# How to run

To run your game with jbhook, you have to use the inject tool to inject the DLL to the game process.
`dist/jb` contains bat scripts with all the important parameters configured. Further parameters can
be added but might not be required to run the game with default settings. Further information on how
to setup the data for each specific version are elaborated in their dedicated readme files.

# Data setup and running the game

Ensure your folder with your unpacked data looks like this:

- `data`
- `prop`
- Various dll files including `jubeat.dll` **OR** `jubeat.exe`

Unpack the package containing jbhook into the folder containing the jubeat binary file.

Run the `gamestart-XX.bat` file where `XX` denotes the version of the game you want to run.

# Eamuse network setup

Running jubeat or ripples? Modify jbhook-01.conf or jbhook-02.conf.

Running anything newer?

- Open the `prop/ea3-config.xml`
- Replace the `ea3/network/services` URL with network service URL of your choice (for example
  http://my.eamuse.com)
- Edit the `ea3/id/pcbid`

# Real hardware support

Run the launcher without the hook dll: `launcher jubeat.dll`

# Command line options

Add the argument *-h* when running inject with jbhook to print help/usage information with a list of
parameters you can apply to tweak various things.
