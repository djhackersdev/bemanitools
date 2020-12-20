# IIDX BIO2 exit hook
A hook library that can be used with BeatmaniaIIDX games that run the BIO2 IO board either natively,
e.g. 25+, or using the [iidxio-bio2 backend](../iidxhook/iidx-bio2.md). It allows the player to
exit the game by pressing the buttons `Start P1 + Start P2 + VEFX + Effect` simultaneously. This can
be useful if you want an option to exit back to your desktop without having a keyboard attached.

The exit hook lib must be loaded like any other hook lib you are already injecting to the game
using either `inject` or `launcher` (depending on the game version). The order for the hook libs is important as they are loaded in the order specified for the inject/launcher call. The entry in the `gamestart.bat` file should look like this for inject:
```bat
inject iidxhook3.dll -K iidx-bio2-exit-hook.dll bm2dx.exe ...
```
...and for launcher:
```bat
launcher -K iidxhook8.dll -K iidx-bio2-exit-hook.dll bm2dx.dll ...
```

Where `iidxhook3.dll` is used for versions 14-15 and `iidxhook8.dll` for versions 25-26.