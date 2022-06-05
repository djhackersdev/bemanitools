# ddrhook1

## Supported games

The following games are supported with this hook library:

* Dance Dance Revolution X

The games must be bootstrapped using [inject](../inject.md).

## Data setup

Ensure your folder with your unpacked data looks like this:

* `conf`
* `data`
* `ddr`
* `ddr_YYYYMMDDRR` where `YYYYMMDDRR` corresponds to different datecodes of
  different versions. Multiple folders of these possible

`DDR.exe` files should be in `ddr` and `ddr_YYYYMMDDRR` folders.

Unpack the distribution package `ddr-11.zip` into one of the folders containing
a `DDR.exe` file. We recommend using the one with the latest datecode which
denotes the latest version of the game including bugfixes etc.

`gamestart-11.bat` as well as `ddrhook1.dll` are now expected to be located
in the same folder as (one) `DDR.exe` file.

## Running

Run `gamestart-11.bat` as administrator. For the US version of the game, run
`gamestart-11-us.bat` instead. This can be done by either by double
clicking or running it from `cmd.exe`. The latter is recommended to have
any debug output kept on screen after closing the game.

This will run [inject](../inject.md) with the `ddrhook1.dll`. On first run,
if you don't have a conguration file, e.g. `ddr-11.conf`, available in the
same folder, a default one will be created and the game exits. Simply re-run
`gamestart-11.bat` again.

## Configuration ddrhook

The hook library can be configured via cmd arguments or a configuration file.
The latter is generated (*ddr-11.conf* in the same directory) on the first 
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
gamestart-11.bat -p gfx.windowed=true
```

The syntax for the "key=value" is the same as in the config file. Make sure
to have a pre-ceeding "-p" for every parameter added.

However, if a parameter is specifed in the configuration file and as a command
line argument, the command line argument overrides the config file's value.

## Configure USB memory cards and edit data

You can setup actual USB thumb drives mapped to a drive letter, e.g. `E:\` for
player 1 and `F:\` for player 2, or just have them point to any local directory,
e.g. `usbmem_p1` and `usbmem_p2`.

Set-up your folder mappings in the
[conf file or via command line args](#configuration-ddrhook) like follows:

* Enable USB memory data emulation: `ddrhook1.usbmem_enabled=true`
* Set P1 USB memory data path pointing to local folder `usbmem_p1` next to `DDR.EXE`:
  `ddrhook1.usbmem_path_p1=usbmem_p1`
* Set P1 USB memory data path pointing to local folder `usbmem_p2` next to `DDR.EXE`:
  `ddrhook1.usbmem_path_p1=usbmem_p2`
* Have/create a subfolder called `DDR_EDIT` on any location/USB drive you want to use
* Name your edit data file either `DDR_EDIT_J.DAT` (for JP version) or `DDR_EDIT_US.DAT`
  (for US version) and place it in the `DDR_EDIT` directory.

For the example setup above, the full relative path of the edit file should be
`usbmem_p1\DDR_EDIT\DDR_EDIT_J.DAT` for player 1.

Note that USB memory cards are not detected by the game and the game stays
silent about that if they do not contain edit data, your path mapping does not resolve
or you misplaced or named your edit data file incorrectly.

## Grey and glitchy arrows

This is a known issue with many GPUs, typically non-Radeon GPUs.

There is currently not patch available in Bemanitools for that. We assume this is an
issue with incompatible shader code.

There are solutions to hard-patch the code available elsewhere.

## Troubleshooting and FAQ

### Issues with background videos (not) playing

The game expects you have the `CLVSD.ax` file registered for decoding videos.

Grab the `CLVSD.ax` file and go to *Start* > *Run* > enter `regsvr32 clvsd.ax` and
execute. Make sure to run as Administrator, otherwise you will get errors due to
invalid permissions.

### Game crashes during boot

If you have played a newer version of DDR, e.g. DDR 2014+, you might have the
`k-clvsd.dll` codec registered which crashes DDR X and likely X2 (EU/US) as well.

Unregister `k-clvsd.dll`, e.g. `regsvr32 /u k-clvsd.dll`, and
[registering `CLVSD.ax`](#issues-with-background-videos-not-working).

Note that `CLVSD.ax` will likely hang any newer/more recent DDR versions on startup
and require you to use the `k-clvsd.dll` that came with the respective version
instead. See [this section](ddrhook2.md#video-codecs-for-background-videos) for
further details.

### Black screen/render window without a response

This symptom might have many causes, here is a list of known issues and what can be done:

* If not done already, try installing
  [DirectX Redist (June 2010)](https://www.microsoft.com/en-us/download/details.aspx?id=8109)
* If you have an integrated/second GPU, ensure it is disabled
* External (USB) audio devices could cause issues. Ensure you are running on on-board/integrated
  sound cards and unplug any external audio devices.
* [Disable fullscreen optimizations](https://devblogs.microsoft.com/directx/demystifying-full-screen-optimizations/):
  Right click on `DDR.exe`, *Properties* > *Compatibility* > *Disable fullscreen optimizations*
* Check if you can run `DDR.exe` without Bemanitools by simply launching it. It should at least
  boot into a startup screen. If that isn't even possible, then it's likely not an issue with
  Bemanitools. Note that your data path needs to be `D:\HDX` for that to work though.
