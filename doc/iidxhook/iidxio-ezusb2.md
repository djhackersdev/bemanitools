This library drives the ezusb FX2 IO board, also known as IO2, and 
implements the iidxio API of BT5. Thus, it allows you to use this IO2 board with
*any* version of IIDX that is supported by BT5.

# Setup
* Rename iidxio-ezusb2.dll to iidxio.dll.
* Ensure that your gamestart.bat actually injects the appropriate iidxhook dll,
for example:
```
*inject iidxhook3.dll bm2dx.exe ...*
```
or
```
launcher -K iidxhook4.dll bm2dx.dll ...*
```
* Before running the game, you have to flash the appropriate firmware to your 
IO board. The iidxio-ezusb2.dll does NOT take care of this and only drives the 
hardware during gameplay. The binary image required is not included with BT5.
* Use the ezusb2-tool.exe binary included in the tools sub-package to first scan
for the device path of your connected hardware. Then, use the device path to 
flash the appropriate ezusb base firmware. Once the firmware is flashed
successfully, the status LEDs on the side of the board should show a blinking
pattern.
* There is a script called ezusb2-boot.bat which combines the two steps above
and can be integrated into the startup process of a dedicated setup.
* If you ignore these steps, your IO board won't work with our iidxio 
implementation.