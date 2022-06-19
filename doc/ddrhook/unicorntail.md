# Unicorntail

Unicorntail is a hook library to run the DDR games on a hybrid type of hardware referred to as a
"Chimera" or "Unicorn". Compared to a "Dragon", which refers to the official stock hardware by
Konami, a Chimera mixes the P3IO board from a PCB used with GF&DM V4-V8, Jubeat, Jubeat Ripples or
Jubeat Knit with the motherboard from a GF&DM XG, XG2 or XG3 PCB, or a Reflec Beat PCB.

Actual build instructions for such hardware are out of scope here.

## Context

Dragon PCBs are hard to come by compared to finding the parts for a Chimera PCB.

A Chimera PCB however is different in many aspects to the Dragon PCB and requires the use of the
`unicorntail.dll` hook library.

The Dragon's P3IO has two serial ports that the regular P3IOs do not, and one of these serial ports
is used to communicate with the card readers. Note that these are actual serial ports driven
directly by the P3IO microcontroller, these are NOT just passed through to the motherboard like the
proprietary COM1 and COM2 connectors. Unicorn Tail intercepts serial port IO commands being sent to
the P3IO and redirects them to COM2 instead, since COM2 is unused on SD cabinets (on HD cabinets it
connects to a second ACIO bus which drives the extra lighting glitz like the light spikes and maybe
the LED strip).

## Drivers

You need the P3IO driver installed on your target system. These can be acquired from an actual stock
HDD or grabbed from
[bemanitools-supplements](https://dev.s-ul.net/djhackers/bemanitools-supplement/-/blob/master/gfdm/p3io/README.md).

## Setup and run

Edit the `gamestart.bat` file that you are using and append `unicorntail.dll` to the list of hooks,
e.g.:

```shell
launcher.exe -H 33554432 -K unicorntail.dll arkmdxp3.dll %*
```

If you want to use any ddrhook library in addition, you have to have the hooks in the right order
to make everything work correctly, e.g.:

```shell
launcher.exe -H 33554432 -K unicorntail.dll -K ddrhook2.dll arkmdxp3.dll %*
```
