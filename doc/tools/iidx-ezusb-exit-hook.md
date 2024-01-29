A hook library that can be used with BeatmaniaIIDX games that run the old ezusb IO board (e.g.
9-13). It allows you to exit the game by pressing Start P1 + Start P2 + VEFX + Effect
simultaneously. This is very useful if you want an option to exit back to your desktop without
having a keyboard attached.

The exit hook lib must be loaded like any other hook lib you are already injecting to the game using
*inject*. The order for the hook libs is important as they are loaded in the order specified for the
inject call. The entry in the *gamestart.bat* file should look like this: *inject iidxhook1.dll
iidx-ezusb-exit-hook.dll bm2dx.exe ...*

Where iidxhook1 is used for version 9-12. Use iidxhook2 for version 13.
