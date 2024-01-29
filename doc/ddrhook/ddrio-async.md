# Asynchronous proxy wrapper for other ddrio implementations

This implementation of the Bemanitools API is not implementing support for any specific IO hardware.
It is a proxy/shim library that loads another ddrio library, e.g. ddrio-p3io, and drives the entire
backend in a dedicated IO thread. By implementing this behind the ddrio API, it is fully transparent
to any existing application using it.

The main benefit is the improved IO polling performance depending on how expensive the synchronous
calls of the actual hardware are. For example, *ddrio-p3io* has very expensive write calls with ~12
ms duration while read calls take ~4 ms. Therefore, a full update cycle is already about as costly
as rendering an entire frame (at 60 fps).

## Setup

For hook libraries, i.e. ddrhookX, but likely applicable to 3rd party applications (consolidate
their manuals).

- Have `ddrio-async.dll` in the same folder as your `ddrhookX.dll`
- Rename `ddrio-async.dll` to `ddrio.dll`
- Pick another ddrio library as the backend of your choice, e.g. `ddrio-p3io.dl` and put it next to
  the async `ddrio.dll`
- Rename it to `ddrio-async-child.dll`, ddrio-async is looking for that filename in the same folder
- Ensure that your `gamestart.bat` actually injects the appropriate `ddrhook.dll`, for example:

```bat
inject ddrhook1.dll ddr.exe ...*
```

or

```bat
launcher -K ddrhook2.dll arkmdxp3.dll ...*
```
