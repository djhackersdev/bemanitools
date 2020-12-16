# ACIO BIO2 IIDX package dump
Package dump excerpt of the init sequence of a original Konami IIDX BIO2 with sub IO connected.
This was used to identify a missing piece of information that needs to be communicated to the BIO2
for IIDX to initialize the sub IO correctly.

The dump was cut off after two polls as the sequence just keeps on repeating from that point on.

## Findings for problem to solve
With the previous implemention of the BIO2 driver, which was created off references of SDVX KFCA,
a BIO2 used with IIDX and the sub IO (to upgrade older C02, IO2 cabinets) connected didn't
initialize properly. This resulted in no inputs/outputs other than 14 keys working.

The problem identified was a different byte, exact meaning not known, that is sent in
[exchange 4](#exchange-4-ac-io-cmd-clear). Instead of `0x3B` from the SDVX KFCA based
implementation, it needs to be set to `0x2D`.

## Exchange 1: AC_IO_CMD_ASSIGN_ADDRS
### Write
```text
AA 00 00 01 00 01 00 02

AA: SOF
00: addr
0001: AC_IO_CMD_ASSIGN_ADDRS
00: seq_no
01: nbytes
data: 00
02: checksum
```

### Read
```
AA AA 00 00 01 00 01 01 03

AA: SOF
AA: SOF
00: addr
0001: addr
00: seq_no
01: nbytes
01: data
03: checksum
```

## Exchange 2: AC_IO_CMD_GET_VERSION
### Write
```
AA 01 00 02 00 00 03

AA: SOF
01: addr, node 1
0002: AC_IO_CMD_GET_VERSION
00: seq_no
00: nbytes
03: checksum
```

### Read
```
AA AA 81 00 02 00 2C 0D 06 00 00 ...

AA: SOF
AA: SOF
81: Response flag + node 1
0002: AC_IO_CMD_GET_VERSION
00: seq_no
2C: nbytes
0D 06 00 00 ...: data
XX: checksum
```

## Exchange 3: AC_IO_CMD_START_UP
### Write
```
AA 01 00 03 00 00 04

AA: SOF
01: addr
0003: AC_IO_CMD_START_UP
00: seq_no
00: nbytes
04: checksum
```

### Read
```
AA AA 81 00 03 00 01 00 85

AA: SOF
AA: SOF
81: Response flag + node 1
0003: AC_IO_CMD_START_UP
00: seq_no
01: nbytes
00: data
85: checksum
```

## Exchange 4: AC_IO_CMD_CLEAR
### Write
```
AA 01 01 00 00 01 2D 30

AA: SOF
01: addr
0100: AC_IO_CMD_CLEAR
00: seq_no
01: nbytes
2D: data
30: checksum
```

### Read
```
AA AA 81 01 00 00 01 00 83

AA: SOF
AA: SOF
81: Response flag + node 1
0100: AC_IO_CMD_CLEAR
00: seq_no
01: nbytes
00: data
83: checksum
```

## Exchange 5: BIO2_BI2A_CMD_WATCHDOG
### Write
```
AA 01 01 20 00 02 00 00 24

AA: SOF
01: addr
0120: BIO2_BI2A_CMD_WATCHDOG
00: seq_no
02: nbytes
00 00: data
24: checksum
```

### Read
```
AA AA 81 01 20 00 01 00 A3

AA: SOF
AA: SOF
81: Response flag + node 1
0120: BIO2_BI2A_CMD_WATCHDOG
00: seq_no
01: nbytes
00: data
A3: checksum
```

## Exchange 6: BIO2_BI2A_CMD_POLL
### Write
```
AA 01 01 52 00 30 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 84

AA: SOF
01: addr
0152: BIO2_BI2A_CMD_POLL
00: seq_no
30: nbtes
...: data
84: checksum 
```

### Read
```
AA AA 81 01 52 00 2E 00 00 B0 00 F0 00 F0 F0 00 00 00 00 00 02 00 5F 11 FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 F3

AA: SOF
AA: SOF
81: Response flag + node 1
0152: BIO2_BI2A_CMD_POLL
00: seq_no
2E: nbytes
...: data
F3: checksum
```

## Exchange 7: BIO2_BI2A_CMD_POLL
### Write
```
AA 01 01 52 00 30 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 84
```

### Read
```
AA AA 81 01 52 00 2E 00 00 B0 00 F0 00 F0 F0 00 00 00 00 00 02 00 64 11 FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 F8
```