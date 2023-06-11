# Tools
Documentation about various additional tooling that is fundamentally not required to run any of
BT5's supported games, but provides additional features for users and developers alike.

## BT5 API
Various command line tools for quick and easy testing of BT5 API implementations witohut having
to run any target games.

* [ddriotest](ddriotest.md): `ddrio` API
* [eamiotest](eamiotest.md): `eamio` API
* [iidxiotest](iidxiotest.md): `iidxio` API
* [jbiotest](jbiotest.md): `jbio` API

## IO related
### ACIO
* [aciotest](aciotest.md): Command line tool for quick and easy testing of ACIO devices without
having to run a game.

### P3IO DDR testing tool
* [p3io-ddr-tool](p3io-ddr-tool.md): Extensive command line tool to test and debug a real P3IO DDR
  (Dragon PCB) IO board + EXTIO

### Ezusb
* [ezusb-iidx-fpga-flash](ezusb-iidx-fpga-flash.md): Tool for flashing the FPGA on ezusb 1 boards.
Required if you want to run games without native ezusb support using BT5's iidxio API.
* [ezusb-iidx-sram-flash](ezusb-iidx-sram-flash.md): Tool for flashing data to the SRAM of ezusb
FX 2 boards. Required if you want to run games without native ezusb FX 2 support using BT5's iidxio
API. 
* [ezusb-tool](ezusb-tool.md): Fundamental tool for flashing the base firmware to ezusb 1/2 boards.
Required if you want to run games without native ezusb 1/2 support using BT5's iidxio API. 

### IIDX exit hooks
Exit hooks allow you to exit the game using a combination of inputs on native IO hardware.

Supported IO hardware:
* [ezusb](iidx-ezusb-exit-hook.md)
* [ezusb 2](iidx-ezusb2-exit-hook.md)
* [bio2](iidx-bio2-exit-hook.md)

## Misc
* [mempatch-hook]: Hook library to dynamically apply memory address patches during runtime before
your application starts execution. Should be prefered over hardcoded hex-edits if applicable.
* [pcbidgen]: Command line tool to generate random PCBIDs