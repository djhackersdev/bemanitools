# Game list

The following games are compatible with this version of iidxhook:
* Heroic Verse

The games must be bootstrapped using [launcher](../launcher.md).

# Data setup and running the game

## Supported versions of Windows

This version requires at least Win 7 x64 and will not run, like the
former versions, on Win XP x86!

## Dependencies

Make sure to have the following dependencies installed:
* DirectX 9
* Visual C++ 2010 Redistributable Package (x64)

## Data setup

We assume that you are using a clean/vanilla data dump. Ensure your ("concents")
folder with your unpacked data looks like this:
- data
- modules
- prop

* Copy/Move all files from the *modules* directory to the root folder, so they
are located next to the *data* and *prop* folders.
* Copy all files from *prop/defaults* to the *prop* folder.
* Setup proper paths for *dev/nvram* and *dev/raw* in *prop/avs-config.xml* by
replacing the *<fs>*-block in that file with the following block:
```
<fs>
    <root>
        <device __type="str">.</device>
    </root>
    <mounttable>
        <vfs name="boot" fstype="fs" src="dev/raw" dst="/dev/raw" opt="vf=1,posix=1"/>
        <vfs name="boot" fstype="fs" src="dev/nvram" dst="/dev/nvram" opt="vf=0,posix=1"/>
    </mounttable>
    <nr_mountpoint __type="u16">256</nr_mountpoint>
    <nr_filedesc __type="u16">256</nr_filedesc>
</fs>
```
* Unpack the package containing iidxhook9 into the root folder so iidxhook9.dll
and all other files are located in the same folder as *data*, *prop*, 
*bm2dx.dll*, etc.
* Run the gamestart-XX.bat file as admin. Where XX matches the version you 
want to run.

* Note: if you know what you're doing and would like to keep all the dlls in the
*modules* folder then you extract iidxhook9 into that directory, move
gamestart-XX.bat up, and edit the paths.

# Configuring iidxhook

The hook library can be configured via cmd arguments or a configuration file.
The latter is generated (*iidxhook-XX.conf* in the same directory) on the first 
start of the game using the gamestart-XX.bat file (again, XX matches your target
game version). It contains default values for all available parameters and 
comments explaining each parameter. Please follow the comments when configuring 
your setup.

Add the argument *-h* when running gamestart-XX.bat 
(e.g. *gamestart-XX.bat -h*) to print help/usage information with a list of 
all available parameters. Every parameter can be either set as command line
argument or using a configuration file.

To set a parameter from the command line, just add it as an argument after
the bat file like this
```
gamestart-09.bat -p gfx.windowed=true -p gfx.framed=true
```

The syntax for the "key=value" is the same as in the config file. Make sure
to have a pre-ceeding "-p" for every parameter added.

However, if a parameter is specifed in the configuration file and as a command
line argument, the command line argument overrides the config file's value.

# Eamuse network setup

If you want to run the games online, you need a valid PCBID and the service URL.
Open *prop/ea3-config.xml* and set the values of the *ea3/id/pcbid* and
*ea3/network/services* nodes accordingly.

Run the game with the gamestart-XX.bat file and enable network on the operator 
menu. When enabled, the game seems to hang and expects you to power
cycle the machine (i.e. quit the game and restart it).

# Real hardware support

### BIO2 hardware

Set `io.disable_bio2_emu=true` in the `iidxhook.conf` file to not hook the BIO2
communication with an emulation layer. The game will directly talk to the IO.
In this mode, tthe game supports the BIO2 board only.

### Ezusb and other

Set `io.disable_bio2_emu=false` in the `iidxhook.conf` file.
Use the specific iidxio API implementations, e.g. `iidxio-ezusb2.dll` to use
the IO2 EZUSB board. The common `iidxio` abstraction layer also allows you to
use custom IO boards or whatever Konami hardware is going to be available in
the future. Obviously, someone has to write an imlementation of the `iidxio`
API, first.

## Slotted/Wave pass card readers

You have the following options:
* Set `io.disable_card_reader_emu=true` in the `iidxhook.conf` file to not hook
card reader communication with an emulation layer. The game will directly talk
to the real readers though this only supports whatever readers the game 
directly supports (wave pass readers)
* Set `io.disable_card_reader_emu=false` in the `iidxhook.conf` file.
Replace the default `eamio.dll` with the `eamio-icca.dll` and have either your
slotted (IIDX, DDR Supernova or GF/DM type) or new wave pass card readers 
conencted and and assigned to `COM1`. Other custom implementations of of the
`eamio` API also work.

### ICCA device settings (device manager)
* Port: COM1
* BAUD rate: 57600
* Data bits: 8
* Parity: None
* Stop bits: 1
* Flow control: None

If you encounter issues after the game opened the device, e.g. application
stuck, try a USB <-> COM dongle instead of using one of the COM ports of the
mainboard.

# Troubleshooting and FAQ

## The game does not run "well" (frame drops, drifting offsync etc)
This can be related to various issues:
* Make sure to run the game as (true) Administrator especially on Windows 7 and
newer. This will also get rid of various other errors (see below) that are 
related to permission issues.
* Run the game's process with a higher priority:
```
start "" /relatime "gamestart.bat"
```
* Enforce v-sync enabled in your GPU settings.
* Ensure that you have a constant refresh rate around the 60 hz (59.9xx or 60.0xx)
that is not jumping around. Use the timebase feature of one of the newer games to
check that or enable iidxhook's timebase and check the log output for the
determined value. Run this a few times and check if the results differ.

## "NETWORK WARNING" instead of "NETWORK OK"
This can be caused by:
* Invalid PCBID
* Firewall blocking connections
* Invalid eamuse url or port specified
* Game is not run using the Administrator account
Make sure to check these things first

## My songs are offsync
From IIDX 20 (or Lincle very final revision) onwards, the game comes with 
a built-in auto timebase option ("monitor check" on startup) which
dynamically, detects the refresh rate of your current setup. Thus, BT5's 
timebase option is not included from this hook version onwards, anymore.
Ensure that refresh rate displayed is very stable, e.g. 60.00x hz, and the
game should be able to provide you with a smooth and sync game experience.

## My game runs too fast
iidxhook can limit the frame rate for you (refer to help/config file)

## My game crashes when I try fullscreen
Use dxwnd and set settings like "Acquire admin caps" and "Fullscreen only"

## Background videos aren't working. When starting a song, windows is playing the error sound and a message box appears
You are missing a codec to decode and play the videos. There are different 
methods available to get background videos working. Probably, the easiest 
solution: grab the CLVSD.ax file and go to Start -> Run -> regsvr32 clvsd.ax
Make sure to run cmd.exe as Administrator, otherwise you will get errors caused
by invalid permissions.

## I installed the CLVSD.ax codec but the game crashes or displays a message box that tells me to disable my debugger
If songs keep crashing upon start and you get an error message that says
```
DirectShow Texture3D Sample
Could not create source filter to graph! hr=0x80040266
```
despite having the codec (CLVSD.ax) installed, remove the debug flag (*-D*) 
from gamestart or use a CLVSD.ax codec which has the debugger checks removed.
