# Game list

The following games are compatible with this version of iidxhook:

- CANNON BALLERS
- Rootage

The games must be bootstrapped using [launcher](../launcher.md).

# Data setup and running the game

## Supported versions of Windows

This version requires at least Win 7 x64 and will not run, like the former versions, on Win XP x86!

## Dependencies

Make sure to have the following dependencies installed:

- DirectX 9
- Visual C++ 2010 Redistributable Package (x64)

## Data setup

We assume that you are using a clean/vanilla data dump. Ensure your ("concents") folder with your
unpacked data looks like this:

- data
- modules
- prop

* Copy/Move all files from the *modules* directory to the root folder, so they are located next to
  the *data* and *prop* folders.
* Copy all files from *prop/defaults* to the *prop* folder.
* Create a new file *app-config.xml* in the *prop* folder with the following content:

```
<?xml version="1.0"?>
<param></param>
```

- Setup proper paths for *dev/nvram* and *dev/raw* in *prop/avs-config.xml* by replacing the
  *<fs>*-block in that file with the following block:

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

- Unpack the package containing iidxhook8 into the root folder so iidxhook8.dll and all other files
  are located in the same folder as *data*, *prop*, *bm2dx.dll*, etc.
- Run the gamestart-XX.bat file as admin. Where XX matches the version you want to run.

# Configuring iidxhook

The hook library can be configured via cmd arguments or a configuration file. The latter is
generated (*iidxhook-XX.conf* in the same directory) on the first start of the game using the
gamestart-XX.bat file (again, XX matches your target game version). It contains default values for
all available parameters and comments explaining each parameter. Please follow the comments when
configuring your setup.

Add the argument *-h* when running gamestart-XX.bat (e.g. *gamestart-XX.bat -h*) to print help/usage
information with a list of all available parameters. Every parameter can be either set as command
line argument or using a configuration file.

To set a parameter from the command line, just add it as an argument after the bat file like this

```
gamestart-09.bat -p gfx.windowed=true -p gfx.framed=true
```

The syntax for the "key=value" is the same as in the config file. Make sure to have a pre-ceeding
"-p" for every parameter added.

However, if a parameter is specifed in the configuration file and as a command line argument, the
command line argument overrides the config file's value.

# Eamuse network setup

If you want to run the games online, you need a valid PCBID and the service URL. Open
*prop/ea3-config.xml* and set the values of the *ea3/id/pcbid* and *ea3/network/services* nodes
accordingly.

Run the game with the gamestart-XX.bat file and enable network on the operator menu. When enabled,
the game seems to hang and expects you to power cycle the machine (i.e. quit the game and restart
it).

# Real hardware support

### BIO2 hardware

Set `io.disable_bio2_emu=true` in the `iidxhook.conf` file to not hook the BIO2 communication with
an emulation layer. The game will directly talk to the IO. In this mode, tthe game supports the BIO2
board only.

### Ezusb and other

Set `io.disable_bio2_emu=false` in the `iidxhook.conf` file. Use the specific iidxio API
implementations, e.g. `iidxio-ezusb2.dll` to use the IO2 EZUSB board. The common `iidxio`
abstraction layer also allows you to use custom IO boards or whatever Konami hardware is going to be
available in the future. Obviously, someone has to write an imlementation of the `iidxio` API,
first.

## Slotted/Wave pass card readers

You have the following options:

- Set `io.disable_card_reader_emu=true` in the `iidxhook.conf` file to not hook card reader
  communication with an emulation layer. The game will directly talk to the real readers though this
  only supports whatever readers the game directly supports (wave pass readers)
- Set `io.disable_card_reader_emu=false` in the `iidxhook.conf` file. Replace the default
  `eamio.dll` with the `eamio-icca.dll` and have either your slotted (IIDX, DDR Supernova or GF/DM
  type) or new wave pass card readers conencted and and assigned to `COM1`. Other custom
  implementations of of the `eamio` API also work.

### ICCA device settings (device manager)

- Port: COM1
- BAUD rate: 57600
- Data bits: 8
- Parity: None
- Stop bits: 1
- Flow control: None

If you encounter issues after the game opened the device, e.g. application stuck, try a USB \<-> COM
dongle instead of using one of the COM ports of the mainboard.

# Troubleshooting and FAQ

## The monitor check is showing high fps and I am using a monitor with high refresh rate features, e.g. 120/144hz

Sync and timing might be screwed up since the game was never meant to run on such refresh rates. Try
setting the option `gfx.forced_refresh_rate` to either `59` or `60`.

Note: Your GPU driver settings must be configured to allow application overrides on vertical
sync/refresh rate options.

On AMD GPUs, set the "V-Sync" option to `On, unless application specifies`.

## The game does not run "well" (frame drops, drifting offsync etc)

This can be related to various issues:

- Make sure to run the game as (true) Administrator especially on Windows 7 and newer. This will
  also get rid of various other errors (see below) that are related to permission issues.
- Run the game's process with a higher priority:

```
start "" /relatime "gamestart.bat"
```

Also refer to the [iidx-syncbook](iidx-syncbook.md).

## "NETWORK WARNING" instead of "NETWORK OK"

This can be caused by:

- Invalid PCBID
- Firewall blocking connections
- Invalid eamuse url or port specified
- Game is not run using the Administrator account Make sure to check these things first

## My songs are offsync

Refer to the [iidx-syncbook](iidx-syncbook.md).

## My game runs too fast

iidxhook can limit the frame rate for you (refer to help/config file)

## My game crashes when I try fullscreen

Use dxwnd and set settings like "Acquire admin caps" and "Fullscreen only"

## Background videos aren't working. When starting a song, windows is playing the error sound and a message box appears

If you are running in window mode, you can see an error pop-up window with the title
`DirectShow Texture3D Sample` and error message
`Could not create source filter to graph! hr=<some number>`.

When running fullscreen, you only hear a windows error sound and the game appears to be frozen when
trying to play a background video.

You are missing a codec to decode and play the videos. There are different methods available to get
background videos working. Probably, the easiest solution: grab the CLVSD.ax file and go to Start ->
Run -> regsvr32 clvsd.ax Make sure to run cmd.exe as Administrator, otherwise you will get errors
caused by invalid permissions.

## I installed the CLVSD.ax codec but the game crashes or displays a message box that tells me to disable my debugger

If songs keep crashing upon start and you get an error message that says

```
DirectShow Texture3D Sample
Could not create source filter to graph! hr=0x80040266
```

despite having the codec (CLVSD.ax) installed, remove the debug flag (*-D*) from gamestart or use a
CLVSD.ax codec which has the debugger checks removed.

## I am getting a message box with a japanese error message and a black window immediately after starting the game

The game checks the vendor and product ID of your GPU installed. If it doesn't match a hardcoded
whitelist, the game won't boot. Use the option *gfx.pci_id* either in the config file or as a cmd
argument to spoof these IDs. See the help message for instructions and possible IDs.

## Over-/underscan, bad image quality or latency caused by my monitor's/TV's upscaler

Many modern monitors/TVs cannot upscale some lower resolutions, e.g. 640x480, properly. This can
lead to over-/underscan, bad image quality or even latency caused by the upscaler of the device you
are using. If one or multiple of these issues apply, use the built in scaling options by setting
*gfx.scale_back_buffer_width* and *gfx.scale_back_buffer_height* to a target resolution to scale to.
Usually, you want to set this to the monitor's native resolution, e.g. 1920x1080 for full HD. You
can play around with a few different filters using *gfx.scale_back_buffer_filter* which impacts
image quality/blurriness on upscaling.
