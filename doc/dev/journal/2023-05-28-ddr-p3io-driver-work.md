# DDR p3io driver work, various notes

Date: 2023-05-28 Author: icex2

Notes about my work on writing a DDR P3io driver.

## Python IO board hardware breakout interface

PCB breakout interfaces/connectors on the side of the boards.

### P2IO DDR

Descriptions assume cabinet hardware of an upgraded DDR "black SD cabinet"

- `LINE OUT 1`: Primary audio out to amplifier
- `LINE OUT 2`: N.C.
- `RGB`: Video out to monitor, supports 15khz/31khz based on hardware switch for video freq/mode
  selection
- `COM1`: Virtual com port that connects to the EXTIO
- `COM2`: Virtual com port that connects to a pair of ICCA card readers
- `LAN`: Network connection
- `PWR`: 100V power in
- `ANALOG`: N.C.
- `PORT 1`: Lights output for cabinet lights, e.g. menu button lights, top header lights
- `PORT 2`: N.C.
- `DIPSW`
  - `1`: ???
  - `2`: On = force all sensor polling mode and allow running the game without an EXTIO
  - `3`: ???
  - `4`: On = force 15 khz monitor output
- `PLUG 1`: Black dongle
- `PLUG 2`: White dongle

### P3IO GF/DM

Descriptions assume usage of a P3IO from a GF/DM in a "chimera style PCB build" on an upgraded DDR
"black SD cabinet"

- `PWR`: 100V power in
- `12V-OUT`: +12V out for external devices
- `PORT 1`: Lights output for cabinet lights, e.g. menu button lights, top header lights
- `PORT 2`: N.C.
- `COM1`: To EXTIO
- `COM2`: To card readers
- `RGB`: Video out to monitor, supports 15khz/31khz based on hardware switch for video freq/mode
  selection
- `LINE OUT 1`: Primary audio out to amplifier
- `LINE OUT 2`: N.C.
- `DIPSW`
  - `1`:
  - `2`:
  - `3`:
  - `4`: On = force 15 khz monitor output
- `PLUG 1`: Roundplug black dongle
- `PLUG 2`: Roundplug white dongle

### P3IO DDR(X)

Descriptions assume cabinet hardware of an upgraded DDR "black SD cabinet"

- `PWR`: 100V power in
- `USB`: USB memory card readers on cabinet
- `PORT 1`: Lights output for cabinet lights, e.g. menu button lights, top header lights
- `COM3-4`: Virtual COM ports
  - COM3 (VCOM1): Pins 1,3,5 on connector -> To card readers
  - COM4 (VCOM0): Pins 2,4,6 on connector -> N.C.
- `PLUG`: Breakout to security round plugs (black and white dongles)
- `COM1`: To EXTIO
  - Pinout (pins left to right)
    - 1: TXD1
    - 2: RXD1
    - 3: N.C.
    - 4: N.C.
    - 5: GND
- `COM2`: N/A (light spires on black HD cabinet)
  - Pinout (pins left to right)
    - 1: TXD2
    - 2: RXD2
    - 3: GND
- `RGB`: Video out to monitor, supports 15khz/31khz based on hardware switch for video freq/mode
  selection
- `LINE OUT1`: Primary audio out to amplifier
- `LINE OUT2`: N.C.
- `LAN`: Network
- `DIP SW`
  - `1`:
  - `2`:
  - `3`:
  - `4`: On = force 15 khz monitor output

## Python IO boards and differences

### P3IO DDR(X)

- Connects to USB and actually enumerates as a USB device and not a virtual COM port
- COM ports 1-4 on breakout of the PCB are being passed through as actual COM ports to the operating
  system

### P3IO GF/DM

- Connects to USB and actually enumerates as a USB device and not a virtual COM port
- COM ports 1-2 on breakout of the PCB are just virtual COM ports
- These do not show up as COM ports on the operating system
- The game drives the COM ports through the main P3IO protocol with additional P3IO commands to
  open, read, write and close these virtual COM ports

### P2IO DDR

- Connects to USB and actually enumerates as a USB device and not a virtual COM port
- COM ports 1-2 on breakout of the PCB are just virtual COM ports
- These do not show up as COM ports on the operating system
- The game drives the COM ports through the main P3IO protocol with additional P3IO commands to
  open, read, write and close these virtual COM ports

## Pinout card reader P1 -> P2 mini din8 male to mini din8 male

Port 1 2 and 3:

- Pin 3: TX
- Pin 4: GND
- Pin 5 : RX

Pin 3 and 5 need to be reversed inbetween readers They are all the same, so your cable needs to
bridge them over. A standard male to male mini din 8 cable does not do that.

### Pinout card reader stock cable s-sub9 to round pin9

```text
dsub-9 female -> mini-din8 male
2 (TXD) -> 3
3 (RXD) -> 5
5 (GND) -> 4
```

## ADE board and com port assignments

Default or incorrectly configured?

- `COM1` -> on mainboard
- `COM2` -> COM2 on P3IO breakout
- `COM3` -> ???
- `COM4` -> COM1 on P3IO breakout

## P3IO command init sequence on DDR 18

From the ddrio-python23 library

````text
.data:1002D5D0 g_init_pakets   db 0AAh, 2, 0, 1, 29h dup(0); field_0.field_0
.data:1002D5D0                                         ; DATA XREF: initialize_and_send_pakets+5↑o
.data:1002D5D0                 db 0AAh, 2, 0, 2Fh, 29h dup(0); field_0.field_0
.data:1002D5D0                 db 0AAh, 3, 1, 27h, 1, 28h dup(0); field_0.field_0
.data:1002D5D0                 db 0AAh, 2 dup(2), 31h, 29h dup(0); field_0.field_0
.data:1002D5D0                 db 0AAh, 2, 3, 1, 29h dup(0); field_0.field_0
.data:1002D5D0                 db 0AAh, 3, 4, 27h, 29h dup(0); field_0.field_0
.data:1002D5D0                 db 0AAh, 2Bh, 5, 25h, 10h, 28h dup(0); field_0.field_0
.data:1002D5D0                 db 0AAh, 2Bh, 6, 25h, 10h, 28h dup(0); field_0.field_0
.data:1002D5D0                 db 0AAh, 2Bh, 7, 25h, 10h, 28h dup(0); field_0.field_0
.data:1002D5D0                 db 0AAh, 2Bh, 8, 25h, 10h, 28h dup(0); field_0.field_0
.data:1002D5D0                 db 0AAh, 2Bh, 9, 25h, 10h, 28h dup(0); field_0.field_0
.data:1002D5D0                 db 0AAh, 2Bh, 0Ah, 25h, 29h dup(0); field_0.field_0
.data:1002D5D0                 db 0AAh, 2Bh, 0Bh, 25h, 29h dup(0); field_0.field_0
.data:1002D5D0                 db 0AAh, 2Bh, 0Ch, 25h, 29h dup(0); field_0.field_0
.data:1002D5D0                 db 0AAh, 2Bh, 0Dh, 25h, 29h dup(0); field_0.field_0
.data:1002D5D0                 db 0AAh, 2Bh, 0Eh, 25h, 29h dup(0); field_0.field_0
.data:1002D5D0                 db 0AAh, 3, 0Fh, 5, 29h dup(0); field_0.field_0
.data:1002D5D0                 db 0AAh, 3, 0, 2Bh, 1, 28h dup(0); field_0.field_0
.data:1002D5D0                 db 0AAh, 3, 1, 29h, 5, 28h dup(0); field_0.field_0
.data:1002D5D0                 db 0AAh, 3, 2, 5, 30h, 28h dup(0); field_0.field_0
.data:1002D5D0                 db 0AAh, 2 dup(3), 27h, 29h dup(0); field_0.field_0
``

```text
// 01: get version <- this is part of a "flush" of old data?
// 2F: set mode
// 27: get cab type or dispsw
// 31: get coinstock
// 1: get version
// 27: get cab type or dipsw
// 25: read plug
// 25: read plug
// 25: read plug
// 25: read plug
// 25: read plug
// 25: read plug
// 25: read plug
// 25: read plug
// 25: read plug
// 25: read plug
// 5: set watchdog
// 2b: unknown
// 29: get video freq
// 5: set watchdog
// 27: get cab type or dipsw
````
