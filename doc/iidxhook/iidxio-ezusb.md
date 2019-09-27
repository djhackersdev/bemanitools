This library drives a "legacy" ezusb IO board, also known as C02 IO, and 
implements the iidxio API of BT5. Thus, it allows you to use this IO board with
*any* version of IIDX that is supported by BT5.

# Setup
* Rename iidxio-ezusb.dll to iidxio.dll.
* Ensure that your gamestart.bat actually injects the appropriate iidxhook dll,
for example:
```
*inject iidxhook3.dll bm2dx.exe ...*
```
or
```
launcher -K iidxhook4.dll bm2dx.dll ...*
```
* Before running the game, you have to flash a set of binaries to your IO board
(base firmware and FPGA). The iidxio-ezusb.dll does NOT take care of this and
only drives the hardware during gameplay. The binary images required are not 
included with BT5.
* Use the ezusb-tool.exe binary included in the tools sub-package to flash the
appropriate ezusb base firmware. Once the firmware is flashed successfully,
the status LEDs on the side of the board should show a blinking pattern.
* Use the ezusb-iidx-fpga-flash.exe binary to flash the appropriate FPGA binary
dump to the FPGA.
* There is a script called ezusb-boot.bat which combines the two steps above
and can be integrated into the startup process of a dedicated setup.
* If you ignore these steps, you will either run into errors or parts of the
IO board won't work (e.g. lights).