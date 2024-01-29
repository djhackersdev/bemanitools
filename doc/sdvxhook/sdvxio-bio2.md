This library talks to a BIO2 BI2A node and implements the sdvxio API of BT5. Thus, it allows you to
use a BIO2 SDVX device board with *any* version of SDVX that is supported by BT5.

# Setup

- Make sure that your BIO2 is flashed with a SDVX firmware (if you can boot SDVX5 with it, it's
  already the right firmware)
  - The firmware persists between boots, so as long as it's flashed to the correct firmware, you no
    longer need to do this step
- Rename sdvxio-kfca.dll to sdvxio.dll.
- Ensure that your gamestart.bat actually injects the appropriate sdvxhook dll for example:

```
launcher -K sdvxhook.dll soundvoltex.dll ...*
```

or that the application otherwise uses sdvxio.dll

- sdvxio-bio2 implements BIO2 device autodetection
  - if this fails please make a bug report, and set the port manually in the config
- You can change the port and baudrate by editing the sdvxio-bio2.conf that should be created by
  default
- Please connect any additional acio devices to their own port (ex: SDVX5 expects an ICCA by itself
  on COM2)
