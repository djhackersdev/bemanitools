# Game list

The following games are compatible with this version of iidxhook:
* GOLD
* DJ Troopers
* EMPRESS
* SIRIUS

The games must be bootstrapped using [inject](../inject.md).

# Data setup and running the game

Ensure your folder with your unpacked data looks like this:
- yyyymmddrr (y = year digit, m = month digit, d = day digit, r = revision digit) 
revision folder containing game binary and libraries
- data
- sidcode.txt

Any further files are optional and not required to just run the game.

Unpack the package containing iidxhook3 into the revision folder of your choice.
Most likely, you want to target the latest revision you have to run the latest
binary of the game with any bugfixes by developers.

Run the gamestart-XX.bat file as admin where XX is the version of your choice 
that's supported by this hook.

# Configuring iidxhook

The hook library can be configured via cmd arguments or a configuration file.
The latter is generated (*iidxhook.conf* in the same directory) on the first 
start of the game using the gamestart-XX.bat file. It contains default values
for all available parameters and comments explaining each parameter. Please
follow the comments when configuring your setup.

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

If you want to run the games online, you have to set a valid PCBID and EAMID 
(use the PCBID as the EAMID) in the configuration file or as a command line 
argument. You also have to set the url of the eamuse server you want to 
connect to.

Additional note regarding EAMID: This is new compared to the prior games and
is provided as the identifier of the "eamuse license" to the server. Depending
on the implementation of the server, this can lead to authentication failure
resulting in
[a network error on boot or warning during gameplay](#network-warning-instead-of-network-ok).

Run the game with the gamestart-XX.bat file and enable network on the operator 
menu. When enabled, the game seems to hang and expects you to power
cycle the machine (i.e. quit the game and restart it).

# Real hardware support

## USB IO (ezusb)

Use the specific iidxio API implementations, e.g. iidxio-ezusb2.dll to use
the IO2 EZUSB board, to run the game on real hardware. Thanks to a common
abstraction layer, you can also use custom IO boards or whatever Konami hardware
is going to be available in the future. Obviously, someone has to write a
driver, first.

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
## The game does not start at all
If you try to run iidx16 or iidx17 and you see one of the following outputs when
running the gamestart-XX.bat script:
```
Failed to launch hook EXE: 00000002
```
or
```
Injecting: iidxhook3.dll
Debug active process
Resuming remote process...
```

Make sure to check if you are using non htpac'd executables on iidx16 and iidx17
and also non htpac'd libraries/dll files on iidx17. These won't work with BT
and injecting the hook libraries.

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

## "NETWORK WARNING" instead of "NETWORK OK"
This can be caused by:
* Invalid PCBID/EAMID: Ensure both are [set to the same ID](#eamuse-network-setup)
* Firewall blocking connections
* Invalid eamuse url or port specified
* Game is not run using the Administrator account
Make sure to check these things first

## My songs are offsync
The built-in monitor check just determines if the game should sync to either
59.94 hz (S-Video setting) or 60.04 hz (VGA setting). If you don't have a setup
that runs on (as close as possible) these values: 
* Make sure your machine's refresh rate is stable, e.g. 60.00x hz.
* If you don't get a close to 59.94hz (S-Video setting) or 60.04 hz 
(VGA setting) refresh rate, go an set the output mode in the operator menu
to "VGA" to enforce the game to run chart syncing on 60.04 hz refresh
rate (even if your setup does not have that value). Next, use the software 
monitor check/auto timebase that's built into iidxhook (refer to cmd 
help/configfile).

## My game runs too fast
iidxhook can limit the frame rate for you (refer to help/config file)

## My game crashes when I try fullscreen
Use dxwnd and set settings like "Acquire admin caps" and "Fullscreen only"

## Background videos aren't working. When starting a song, windows is playing the error sound and a message box appears
If you are running in window mode, you can see an error pop-up window with the title
`DirectShow Texture3D Sample` and error message
`Could not create source filter to graph! hr=<some number>`.

When running fullscreen, you only hear a windows error sound and the game appears to be frozen
when trying to play a background video.

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

## I am getting a message box with a japanese error message and a black window immediately after starting the game
The game checks the vendor and product ID of your GPU installed. If it doesn't
match a hardcoded whitelist, the game won't boot. Use the option *gfx.pci_id*
either in the config file or as a cmd argument to spoof these IDs. See the
help message for instructions and possible IDs.

## Over-/underscan, bad image quality or latency caused by my monitor's/TV's upscaler
Many modern monitors/TVs cannot upscale 640x480 output properly. This can lead to
over-/underscan, bad image quality or even latency caused by the upscaler of the device
you are using.
If one or multiple of these issues apply, use the built in scaling options by setting
*gfx.scale_back_buffer_width* and *gfx.scale_back_buffer_height* to a target resolution
to scale to. Usually, you want to set this to the monitor's native resolution, e.g.
1920x1080 for full HD. You can play around with a few different filters using
*gfx.scale_back_buffer_filter* which impacts image quality/blurriness on upscaling.
