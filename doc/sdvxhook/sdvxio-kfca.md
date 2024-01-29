This library talks to a KFCA node through ACIO and implements the sdvxio API of BT5. Thus, it allows
you to use a KFCA device board with *any* version of SDVX that is supported by BT5.

# Setup

- Rename sdvxio-kfca.dll to sdvxio.dll.
- Ensure that your gamestart.bat actually injects the appropriate sdvxhook dll for example:

```
launcher -K sdvxhook2.dll soundvoltex.dll ...*
```

or that the application otherwise uses sdvxio.dll

- sdvxio-kfca expects a KFCA device on COM3 by default
- You can change the port and baudrate by editing the sdvxio-kfca.conf that should be created by
  default
- If there is also an ICCA device on the same ACIO bus, it cannot be used with eamio-icca at this
  time
- Please connect any additional acio devices to their own port (ex: SDVX5 expects an ICCA by itself
  on COM2)
