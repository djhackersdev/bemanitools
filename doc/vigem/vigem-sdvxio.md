This application allows you to use any sdvxio backend, e.g. `sdvxio-kfca.dll`, to be available as a XBOX 360 game controller on windows.
Thus, it allows you to use a real cab with *any* game that supports xinput.

# Setup
* Install [ViGEmBus](https://github.com/ViGEm/ViGEmBus/releases)
* Place the following in the same folder as vigem-sdvxio:
  * Get a copy of [ViGEmClient.dll](https://bin.jvnv.net/file/ZgMJK/ViGEmClient.zip) (or from bemanitools-supplements)
  * Rename your corresponding `sdvxio-XXX.dll`, e.g. `sdvxio-kfca.dll`, to `sdvxio.dll`.
* Run `vigem-sdvxio.exe` so that the config file gets created
* Edit `vigem-sdvxio.conf` so that the config file gets created as needed

# Usage
* Run `vigem-sdvxio.exe`
* To quit the program, hit the TEST + SERVICE button at the same time

# Mapping
* BT ABCD are mapped to ABXY
* FX LR are mapped to LB/RB
* VOL LR are mapped to L thumbstick X/Y (either in absolute or relative mode depending on the config)

# Additional Notes For Cabinets (Running on embedded Windows 7):
* Make sure that you follow the instructions exactly from the release page
([Prerequisites for Windows 7](https://github.com/ViGEm/ViGEmBus/wiki/Prerequisites-for-Windows-7))
* If you get an error while trying to install KB3033929, re-enable windows update
* If you can't find some of these files, see bemanitools-supplements
* Make sure to ewfmgr C: -commit and reboot after installing the drivers (this only needs to be done once)
