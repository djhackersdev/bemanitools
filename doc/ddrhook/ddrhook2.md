# ddrhook2

## Supported games

The following games are supported by this hook library:

* Dance Dance Revolution X2 (JP region)
* Dance Dance Revolution X3 vs. 2ndMIX
* Dance Dance Revolution 2013
* Dance Dance Revolution 2014
* Dance Dance Revolution A
* Dance Dance Revolution A20
* Dance Dance Revolution A20+

Note that different builds of the same hook library are required to run the
different versions. See different distribution packages, e.g. `ddr-12.zip`,
`ddr-13.zip` etc.

Depending on the game version, earlier versions are bootstrapped using
[inject](../inject.md) while later versions require [launcher](../launcher.md).

## Setup A20+

### Data and folder structure

The following assumes you are using vanilla and unpacked/decrypted data. Copy/unpack the data
to a destination of your choice. Expect to have the following root folder structure:

```
arkdata
com
data
dev
modules
prop
```

* Copy the contents of `modules` to the root folder that the dll-files are next to the folders
  `arkdata`, `com`, etc.
* Unpack the contents of `ddr-14-to-16.zip` to the root folder. `luncher.exe` should be located
  next to `arkdata`, `com` and all the dll-files

#### Configuring eamuse settings

You need to create a `prop/ea3-config.xml` configuration file which provides properties for
connecting to a network of your choice (and allows the game to boot).

Take the `prop/eamuse.xml` as a base and make a copy of it called `prop/ea3-config.xml`.

Add the software identifier as a child to the `<ea3>` parent node:

```xml
<soft>
  <model __type="str">MDX</model>
  <dest __type="str">J</dest>
  <spec __type="str">A</spec>
  <rev __type="str">A</rev>
  <ext __type="str">2022020200</ext>
</soft>
```

Replace the identifiers according to the version you are using.

Add your PCBID as a child to the `<ea3>` parent node:

```xml
<id>
  <pcbid __type="str">00010203040506070809</pcbid>
  <hardid __type="str">00010203040506070809</hardid>
</id>
```

Replace the values accordingly with your actual PCBID registered with your target network.

Add network settings (note this might already exist, replace existing or delete old one) as a child
to the `<ea3>` parent node:

```xml
<network>
  <sz_xrpc_buf __type="u32">102400</sz_xrpc_buf>
  <ssl __type="bool">0</ssl>
  <services>http://eamuse.konami.fun/service/services/services/</services>
</network>
```

Replace the properties accordingly with the settings provided by your network provider, e.g. the
URL, further settings like ssl etc.

## Running

Run `gamestart-XX.bat`, where `XX` corresponds to the version of the game you
want to run, as administrator. For the US version of X2, run
`gamestart-12-us.bat` instead. This can be done by either by double
clicking or running it from `cmd.exe`. The latter is recommended to have
any debug output kept on screen after closing the game.

This will run [inject](../inject.md) or [launcher](../launcher.md), depending
on the version of the game, with the `ddrhook2.dll`. On first run, if you don't
have a conguration file, e.g. `ddr-12.conf`, available in the same folder, a
default one will be created and the game exits. Simply re-run `gamestart-XX.bat`
again.

## Configure USB memory cards and edit data (DDR X2)

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

## Troubleshooting and FAQ

### Video codecs for background videos

For DDR 2014 and newer (maybe also earlier?), you need to register `k-clvsd.dll` and
`xactengine2_10.dll` to make background videos work. These files are included with
respective versions of the games.

Run the following commands either from a command line (`cmd.exe`) or from 
*Start* > *Run*. Adjust the path to files accordingly pointing to the correct files.

* Register `regsvr32 D:\MDX\contents\k-clvsd.dll`
* Register `regsvr32 D:\MDX\contents\xactengine2_10.dll`

The `gamestart-XX.bat` scripts should already take care of this by executing the listed commands
when launched.

Note: The one that comes with IIDX will hang DDR at startup. The opposite is not true:
IIDX works just fine with this CLVSD.

### Laggy audio

If you're running on Windows XP, go to the sound devices control panel, select your
sound card, click Advanced Options and turn Hardware Acceleration down to basic. This
will make the audio much less laggy..
