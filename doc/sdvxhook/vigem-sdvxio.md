This application allows you to use any sdvxio backend to drive an XB360 controller.
Thus, it allows you to use a real cab with *any* game that supports xinput.

# Setup
* Install ViGEmBus (https://github.com/ViGEm/ViGEmBus/releases)
* Place the following in the same folder as vigem-sdvxio:
  * Get a copy of ViGEmClient.dll (https://bin.jvnv.net/file/ZgMJK/ViGEmClient.zip)
  * Rename your corresponding sdvxio-device.dll to sdvxio.dll.
* Run vigem-sdvxio.exe so that the config file gets created
* Edit vigem-sdvxio.conf so that the config file gets created as needed

# Usage
* Run vigem-sdvxio.exe
* To quit the program, hit the TEST + SERVICE button at the same time

# Mapping
* BT ABCD are mapped to ABXY
* FX LR are mapped to LB/RB
* VOL LR are mapped to L/R thumbstick X

# Additional Notes For Cabinets:
* Make sure that you follow the instructions exactly from the release page (Prerequisites for Windows 7)
* If you get an error while trying to install KB3033929, re-enable windows update
* Make sure to ewfmgr C: -commit and reboot after installing the drivers (this only needs to be done once)
