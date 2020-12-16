# IIDXIO API implementation with BIO2 driver
This implementation of BT5's iidxio API allows you to use native BIO2 hardware with anything that
supports BT5's iidxio API, e.g. all iidxhooks of BT5. Thus, you can play your favorite old IIDX
games with the latest generation of hardware.5.

## Setup
* Have `iidxio-bio2.dll` in the same folder as your `iidxhookX.dll`
* Rename `iidxio-bio2.dll` to `iidxio.dll`
* Ensure that your `gamestart.bat` actually injects the appropriate iidxhook dll, for example:
```bat
inject iidxhook3.dll bm2dx.exe ...*
```
or
```bat
launcher -K iidxhook4.dll bm2dx.dll ...*
```
* This assumes that the BIO2 is already flashed to the correct firmware. The firmware perists once
flashed and does not need to be re-flashed after a power cycle.

## Driver notes
See [here](iidxhook9.md#driver-notes).