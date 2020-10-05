This library talks to a MagicBox cab's IO and implements the jbio API of BT5.
Thus, it allows you to use a MagicBox cab with *any* version of jubeat that is supported by BT5.

# Setup
* Rename `jbio-magicbox.dll` to `jbio.dll`.
* Make sure `jbio.dll` is in the same folder as both `jbhook.dll` and `jubeat.dll`
* Make sure that you have a copy of the `ch341dll.dll` that comes with MagicBox.
* Ensure that your `gamestart.bat` actually injects the appropriate jbhook dll
for example:
```
launcher -K jbhook.dll jubeat.dll ...*
```
or that the application otherwise uses `jbio.dll`
