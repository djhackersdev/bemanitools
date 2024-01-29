A hook library that can be used with BeatmaniaIIDX games that run the ezusb FX2 IO board (e.g.
14-24). It allows you to exit the game by pressing Start P1 + Start P2 + VEFX + Effect
simultaneously. This is very useful if you want an option to exit back to your desktop without
having a keyboard attached.

The exit hook lib must be loaded like any other hook lib you are already injecting to the game using
either *inject* or *launcher* (depending on the game version). The order for the hook libs is
important as they are loaded in the order specified for the inject/launcher call. The entry in the
*gamestart.bat* file should look like this for inject: *inject iidxhook3.dll -K
iidx-ezusb2-exit-hook.dll bm2dx.exe ...* ...and for launcher: *launcher -K iidxhook4.dll -K
iidx-ezusb2-exit-hook.dll bm2dx.dll ...*

Where iidxhook3 is used for 14-15 and iidxhook4 for 20-24.
