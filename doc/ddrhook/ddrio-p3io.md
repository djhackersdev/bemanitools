# ddrio API implementation with DDR P3IO (Dragon) and EXTIO

This implementation of BT5's ddrio API allows you to use the native DDR P3IO of a "Dragon PCB" plus
the EXTIO with anything the ddrio API supports.

This is not required to run the actual games supporting the hardware natively. However, there are
various 3rd party applications using the ddrio API where you might benefit from using actual SD
cabinet hardware, e.g. [vigem-ddrio](../vigem/README.md).

## Setup

For hooks, but likely applicable to 3rd party applications (consolidate their manuals).

- Driver: You must have the P3IO driver intalled on your system
  - Driver from
    [bemanitools-supplements](https://github.com/djhackersdev/bemanitools-supplement/blob/master/ddr/p3io/README.md)
- Have `ddrio-p3io.dll` in the same folder as your `ddrhookX.dll`
- Rename `ddrio-p3io.dll` to `ddrio.dll`
- Ensure that your `gamestart.bat` actually injects the appropriate ddrhook dll, for example:

```bat
inject ddrhook1.dll ddr.exe ...*
```

or

```bat
launcher -K ddrhook2.dll arkmdxp3.dll ...*
```
