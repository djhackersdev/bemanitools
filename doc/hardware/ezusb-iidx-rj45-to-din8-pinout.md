# Ezusb 1/2 RJ45 to DIN8 connector pinout
This was created from using the BIO2 sub IO board to connect pre IIDX 25 cabinet hardware to the
BIO2 IO board. The two connector types from different generation IO boards, RJ45 on the ezusb FX
(C02) board and a round DIN8 connector on the ezusb FX2 (IO2) board are just different connectors to
the same hardware on the PCB.

Therefore, any C02 wired cabinet can be easily upgraded to IO2 wiring and vice versa by building
a simple RJ45 -> DIN8 or DIN8 -> RJ45 adapter.

The following connection information was traced on a original Konami BIO2 sub IO board.

RJ45 (female) connector, top view:
```text
plug insert
     |
     |
     v
|-----------|
|  Top view |
|           |
|   Pins    |    
|   7 5 3 1 |
|  8 6 4 2  |
|-----------|
```

DIN8 (female) connector, top view:
```text
plug insert
     |
     |
     v
|-----------|
|  Top view |
|   Pins    |    
|  3  1 2 6 |
| 7 4 5 8 9 |
|-----------|
```

The two connectors are interconnected on the PCB like this:
| RJ45 pins | DIN8 pins |
|-----------|-----------|
| 1         | 1         |
| 2         | 7         |
| 3         | 4         |
| 4         | 6         |
| 5         | 5         |
| 6         | 3         |
| 7         | 2         |
| 8         | 1         |