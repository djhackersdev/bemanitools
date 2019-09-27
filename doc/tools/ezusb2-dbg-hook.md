A hook library for debugging and dumping usb requests of games that use the
ezusb IO board (IIDX C02, IIDX IO2, Pop'n Music IO2). The library creates
a log file *ezusbdbg.log* in the same directory as your library/executable
which contains data dumps of the usb device's traffic.

Example usage:
*inject iidxhook1.dll ezusbdbg-hook.dll bm2dx.exe ...*
*launcher -K ezusbdbg-hook.dll bm2dx.dll ...*

Make sure to provide the following additional arguments:
*--ezusbdbg_path <device path>*
The device path points to the path to open the device, e.g. on the old IIDX
games that was *"\\\\.\\Ezusb-0"*. If you don't know the path, you can run
the hook with dummy data, e.g. *--ezusbdbg_path asdfgqwer* and check the log
for any logged open calls with paths to find your ezusb device.

--ezusbdbg_type <1 or 2>
Specify the type of device to debug. *1* is for the legacy ezusb device, e.g.
IIDX C02, and *2* for the FX2 type device, e.g. IIDX IO2, Pop'n IO2.

Both parameters must be specified, otherwise the hook will error. Make sure
to check the logfile for any errors or warnings as well.