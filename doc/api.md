# Bemanitools API

Bemanitools introduces interfaces abstracting the IO hardware of many games. This is used to implement support for
non-intended IO devices from simple keyboard support, standard gamecontrollers to custom IO boards or using real
hardware with the games (e.g. support for real legacy hardware).

For a list of already supported and included hardware by game, see the next section.

The BT5 API separates main game IO hardware like buttons, turn tables, spinners, lights etc. (bstio, iidxio, ...) from
eamuse hardware like 10-key pads and card readers (eamio).

If you want to write an implementation for your own custom piece of hardware, check out the SDK (*bemanitools*
sub-folder) in the source code (src.zip).

## Implementations

### IO boards

The following implementations are already shipped with BT5.

* BeatStream
    * bstio.dll (default): Keyboard, joystick and mouse input
* Dance Dance Revolution
    * ddrio.dll (default): Keyboard, joystick and mouse input
    * [ddrio-p3io.dll](ddrhook/ddrio-p3io.md): DDR P3IO (Dragon PCB) + EXTIO hardware
    * ddrio-mm.dll: Minimaid hardware
    * [ddrio-smx.dll](ddrhook/ddrio-smx.md): StepManiaX platforms
* Beatmania IIDX
    * iidxio.dll (default): Keyboard, joystick and mouse input
    * [iidxio-bio2.dll](iidxhook/iidxio-bio2.md): BIO2 driver
    * [iidxio-ezusb.dll](iidxhook/iidxio-ezusb.md): Ezusb (C02 IO) driver
    * [iidxio-ezusb2.dll](iidxhook/iidxio-ezusb2.md): Ezusb FX2 (IO2) driver
* jubeat
    * jbio.dll (default): Keyboard, joystick and mouse input
* pop'n music
    * popnio.dll (default): Keyboard, joystick and mouse input
* SOUND VOLTEX
    * sdvxio.dll (default): Keyboard, joystick and mouse input
    * [sdvxio-bio2.dll](sdvxhook/sdvxio-bio2.md): BIO2 driver
    * [sdvxio-kfca.dll](sdvxhook/sdvxio-kfca.md): KFCA IO board driver

### Eamuse readers

Eamuse hardware support is implemented separately:

* eamio.dll (default): Keyboard and joystick input
* eamio-icca.dll: Slotted/wave pass readers, required for old games with magnetic stripe cards

#### ICCA readers for IIDX and port config in device manager

The COM port for the card readers needs to be configured correctly in device manager. Otherwise,
communication with the readers will fail if the settings do not align with how the game or
bemanitools wants to operate them.

Use built-in ports on your mainboard if available but an external USB to serial port dongle also
works. 

* Assign `COM1` to the COM port the card readers are connected to.
* Ensure that the following settings for the COM port you are going to use are 
set
  * BAUD rate 57600
  * Data bits 8
  * Parity None
  * Stop bits 1
  * Flow control None.

## Development notes

A DEF file for geninput.dll is included. To convert the DEF into an import library suitable for use with Visual C++, run
```
lib /machine:i386 /def:geninput.def
```
from the Visual C++ command line. If you're using mingw then use dlltool:
```
dlltool -d geninput.def -l geninput.a
```