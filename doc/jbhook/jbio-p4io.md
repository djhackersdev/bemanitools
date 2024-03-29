This library talks to a P4IO (panel) and H44B through ACIO (lights) of a jubeat cab and implements
the jbio API of BT5. Thus, it allows you to use a modern jubeat cab with *any* version of jubeat
that is supported by BT5.

# Setup

- Rename `jbio-p4io.dll` to `jbio.dll`.
- Make sure `jbio.dll` is in the same folder as both `jbhook.dll`, `aciomgr.dll` and `jubeat.dll`
- If you want IC card support via eamio-icca, ensure you also rename `eamio-icca.dll` to `eamio.dll`
- Ensure that your `gamestart.bat` actually injects the appropriate jbhook dll for example:

```
launcher -K jbhook.dll jubeat.dll ...*
```

or that the application otherwise uses `jbio.dll`

- If you have P4IO but no H44B, lights will be disabled
- You can change the port and baudrate of the H44B node by editing the `jbio-h44b.conf` that should
  be created by default
- If you are using IC card support, you can edit `eamio-icc.conf` to change the port and baudrate
- If you are playing on a real cab, the port and baud of H44B and ICCA should be identical
