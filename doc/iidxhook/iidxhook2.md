# Game list

The following games are compatible with this version of iidxhook:
* DistorteD

# Data setup and running the game

Ensure your folder with your unpacked data looks like this:
- JAx (Game binary revision folder where 'x' can be A, B, C, D, E, F, G)
- data
- sidcode.txt

Any further files are optional and not required to just run the game.

Unpack the package containing iidxhook2 into the revision folder of your choice.
Most likely, you want to target the latest revision you have to run the latest
binary of the game with any bugfixes by developers.

If you don't run this on old hardware that uses an analog version of a Realtek 
integrated sound chip, you have to replace RtEffect.dll with a stubbed/patched
version (RtEffect_patched.dll). Otherwise, the game might crash instantly when
trying to start it.

Run the gamestart-13.bat file as admin.

# Configuring iidxhook

The hook library can be configured via cmd arguments or a configuration file.
The latter is generated (*iidxhook.conf* in the same directory) on the first 
start of the game using the gamestart-13.bat file. It contains default values
for all available parameters and comments explaining each parameter. Please
follow the comments when configuring your setup.

Add the argument *-h* when running gamestart-13.bat 
(e.g. *gamestart-13.bat -h*) to print help/usage information with a list of 
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

If you want to run the games online, you have to set a valid PCBID in the 
configuration file or as a command line argument. You also have to set the 
url of the eamuse server you want to connect to.

Run the game with the gamestart-13.bat file and enable network on the operator 
menu. When enabled, the game seems to hang and expects you to power
cycle the machine (i.e. quit the game and restart it).

# Real hardware support

## USB IO (ezusb)

Use the specific iidxio API implementations, e.g. iidxio-ezusb.dll to use
an old C02 EZUSB IO board, to run the game on real hardware. Thanks to a common
abstraction layer, you can also use more modern IO, e.g. IO2 boards with
iidxio-ezusb2.dll, even with old games that do not support them.

## Slotted/Wave pass card readers

Replace the default *eamio.dll* with the *eamio-icca.dll* and have either your
slotted (IIDX, DDR Supernova or GF/DM type) or new wave pass card readers 
conencted and and assigned to *COM1*.

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

# Known bugs
## USBIO (FM-DL TIMEOUT)
IIDX occasionally fails to boot with a "USBIO (FM-DL TIMEOUT)" error. If this 
happens, run the game again. 

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
* Use iidxhook's frame rate limiter feature (see further below) to software lock
the refresh rate. This might be necessary on Windows 7 and newer for D3D8 games,
e.g. iidx 9 to 12, which seem to ignore GPU side v-sync.
* Use iidxhook's auto timebase feature (see further below) or set a pre-determined
value to cut down start-up times.

### The game still stutters (randomly) and drifts off-sync
If this concerns a d3d8 based game, i.e. IIDX 9 to 13, use the d3d8to9 wrapper from
the bemanitools-supplement package (follow the included instructions).

## "NETWORK WARNING" instead of "NETWORK OK"
This can be caused by:
* Invalid PCBID
* Firewall blocking connections
* Invalid eamuse url or port specified
* Game is not run using the Administrator account
Make sure to check these things first

## My songs are offsync
* Make sure your machine's refresh rate is stable
* If you don't get a close to 59.94hz refresh rate, use the software monitor
check/auto timebase that's built into iidxhook (refer to help/config file)

## The game crashes instantly
Replace the original RtEffects.dll with the patched version 
RtEffects_patched.dll from utils (for explanation see above).

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

## All background videos are looking streched (starting with HAPPY SKY)
The game requires on a hardware feature that is not present on newer GPUs.
Refer to the help/config file and turn on the UV fix.

## I installed the CLVSD.ax codec but the game crashes or displays a message box that tells me to disable my debugger
If songs keep crashing upon start and you get an error message that says
```
DirectShow Texture3D Sample
Could not create source filter to graph! hr=0x80040266
```
despite having the codec (CLVSD.ax) installed, remove the debug flag (*-D*) 
from gamestart or use a CLVSD.ax codec which has the debugger checks removed.

## I used the auto timebase option and/or limited my refresh rate but the songs are still going offsync
There aren't many options left. The old games were developed for specific
hardware and are not guaranteed to work well on (especially) newer hardware.
Multiple monitor setups can also have a bad impact on a stable refresh rate.
Try a setup with just a single monitor you want to use for gameplay physically
connected. Furthermore, dedicated and tested/verified hardware by other users 
is recommended if you want to save yourself a lot of fiddling.

## Over-/underscan, bad image quality or latency caused by my monitor's/TV's upscaler
Many modern monitors/TVs cannot upscale 640x480 output properly. This can lead to
over-/underscan, bad image quality or even latency caused by the upscaler of the device
you are using.
If one or multiple of these issues apply, use the built in scaling options by setting
*gfx.scale_back_buffer_width* and *gfx.scale_back_buffer_height* to a target resolution
to scale to. Usually, you want to set this to the monitor's native resolution, e.g.
1920x1080 for full HD. You can play around with a few different filters using
*gfx.scale_back_buffer_filter* which impacts image quality/blurriness on upscaling.
If you want to use this with old d3d8 games (IIDX 9-13), you have to use the d3d8to9
library from bemanitools-supplement because the d3d8 hook module cannot support this
upscaling feature. Make sure to set *misc.use_d3d9_hooks=true*. 
