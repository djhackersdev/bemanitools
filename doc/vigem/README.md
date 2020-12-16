# ViGEm: Virtual Gamepad Emulation Framework
Virtual game controller emulation library. Allows us to create in code emulated XBOX controllers.

This is used to hook up BT5's various IO API libraries allowing you to use any of their
implemtations as a game controller on Windows.

References:
* [ViGEmBus](https://github.com/ViGEm/ViGEmBus)
* [ViGEmClient](https://github.com/ViGEm/ViGEmClient)

Available ViGEm client implementations:
* [vigem-iidxio](vigem-iidxio.md): Exposes inputs provided by the iidxio API as two XBOX gamepads
* [vigem-sdvxio](vigem-sdvxio.md): Exposes inputs provided by the sdvxio API as one XBOX gamepad