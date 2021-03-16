# BIO2 sub IO turntable bypass cable
If you upgraded a C02 wired cabinet, the turntables are connected to the
sub IO via a serial line that goes to another sub IO behind the front panel
next to the 16seg display. This introduces additional latency as the input
data needs to pass through multiple systems/PCBs until it reaches the game.

This can be reduced by using a so-called "bypass" cable, that connects the
turntables directly to the BIO2 sub IO board by emulating the wiring of a
D01 cabinet, reducing latency significantly.

This cable could be ordered with the IIDX Rootage upgrade kit but can also
be easily created from scratch.

## Parts
* Some wires
* [JST HL](https://jst.de/file/download/219/pitch-3-96-mm-wtw-hl2-pdf) 2x4 pin female connector
* [JST HVQ](https://jst.de/file/download/637/hvq) 8 pin female connector
* Matching terminals for the connectors

## Pinout
```text
JST HL 2x4 pin front view connector

notch at top
    ___
|---------|
| 1 2 3 4 |
| 5 6 7 8 |
|---------|
```

```
JST HVQ 8 pin front view connector
Numbering on connector matches

   notch at top
___________________
-------------------
|-----------------|
| 1 2 3 4 5 6 7 8 |
|-----------------|

```

Pins of the two connectors need to be connected like this:
| JST HL 2x4 pin | JST 8 pin |
|----------------|-----------|
| 1              | 1         |
| 2              | 1         |
| 3              | 3         |
| 4              | 2         |
| 5              | 5         |
| 6              | 4         |
| 7              | 8         |
| 8              | 8         |
