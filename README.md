# Bemanitools 6

**THIS IS A HIGHLY WORK/DEVELOPMENT IN PROGRESS VERSION**

**THINGS ARE BROKEN AND EVERYTHING IS SUBJECT TO CHANGE**

**ALL DOCUMENTATION IS CONSIDERED OUTDATED AND IS NOT NECESSARILY REVISED UNTIL FURTHER NOTICE**

Version: `6.0.0-alpha.3`

[Changelog](CHANGELOG.md)

A collection of tools to run [various Bemani arcade games](#supported-games).

Bemanitools 5 (BT5) is the successor to Bemanitools 4 which introduces a big code cleanup and
support for newer games. BT5 uses a cleaner approach than BT4 did; specifically, all input and
lighting is handled by emulating the protocols spoken by the real IO PCBs, instead of replacing
chunks of game code like BT4. The benefits of this approach are a more authentic gameplay
experience, and easier support for a broader range of releases from each game series.

## Documentation

Browse our [documentation](doc/README.md) as it might already cover various questions and concerns
you are looking for or about to ask.

## Contributions and bug reporting

[Read the dedicated CONTRIBUTING.md documentation](CONTRIBUTING.md).

The tl;dr version and golden rules of the sections in the document:

- **EVERYONE** can contribute, this is **NOT** limited to people coding
- [Open an issue on gitlab for discussions, feature requests and bug reports](CONTRIBUTING.md#reporting-and-discussions-issues-section-on-github)
- [ALWAYS report bugs as issues and ALWAYS use the available bug template](CONTRIBUTING.md#bug-reports)
- [Everyone is allowed to submit changes which are not just limited to code by opening merge requests](CONTRIBUTING.md#pull-requests-bugfixes-new-features-or-other-code-contributions)
- [Documentation improvements can and even should be contributed by non developers](CONTRIBUTING.md#pull-requests-bugfixes-new-features-or-other-code-contributions)

## Supported games

The following games are supported with their corresponding hook-libraries.

- BeatStream
  - BeatStream (`bst.zip`): bsthook
  - BeatStream アニムトライヴ (`bst.zip`): bsthook
- [Dance Dance Revolution](doc/ddrhook/README.md)
  - Dance Dance Revolution X (`ddr-11.zip`): [ddrhook1](doc/ddrhook/ddrhook1.md)
  - Dance Dance Revolution X2 (US/EU regions) (`ddr-12-us.zip`): [ddrhook1](doc/ddrhook/ddrhook1.md)
  - Dance Dance Revolution X2 (JP region) (`ddr-12.zip`): [ddrhook2](doc/ddrhook/ddrhook2.md)
  - Dance Dance Revolution X3 vs. 2ndMIX (`ddr-13.zip`): [ddrhook2](doc/ddrhook/ddrhook2.md)
  - Dance Dance Revolution 2013 (`ddr-14-to-18.zip`): [ddrhook2](doc/ddrhook/ddrhook2.md)
  - Dance Dance Revolution 2014 (`ddr-14-to-18.zip`): [ddrhook2](doc/ddrhook/ddrhook2.md)
  - Dance Dance Revolution A (`ddr-14-to-18.zip`): [ddrhook2](doc/ddrhook/ddrhook2.md)
  - Dance Dance Revolution A20 (`ddr-14-to-18.zip`): [ddrhook2](doc/ddrhook/ddrhook2.md)
  - Dance Dance Revolution A20+ (`ddr-14-to-18.zip`): [ddrhook2](doc/ddrhook/ddrhook2.md)
- [Beatmania IIDX](doc/iidxhook/README.md)
  - Beatmania IIDX 9th Style (`iidx-09-to-12.zip`): [iidxhook1](doc/iidxhook/iidxhook1.md)
  - Beatmania IIDX 10th Style (`iidx-09-to-12.zip`): [iidxhook1](doc/iidxhook/iidxhook1.md)
  - Beatmania IIDX 11 IIDX RED (`iidx-09-to-12.zip`): [iidxhook1](doc/iidxhook/iidxhook1.md)
  - Beatmania IIDX 12 HAPPY SKY (`iidx-09-to-12.zip`): [iidxhook1](doc/iidxhook/iidxhook1.md)
  - Beatmania IIDX 13 DistorteD (`iidx-13.zip`): [iidxhook2](doc/iidxhook/iidxhook2.md)
  - Beatmania IIDX 14 GOLD (`iidx-14-to-17.zip`): [iidxhook3](doc/iidxhook/iidxhook3.md)
  - Beatmania IIDX 15 DJ TROOPERS (`iidx-14-to-17.zip`): [iidxhook3](doc/iidxhook/iidxhook3.md)
  - Beatmania IIDX 16 EMPRESS (`iidx-14-to-17.zip`): [iidxhook3](doc/iidxhook/iidxhook3.md)
  - Beatmania IIDX 17 SIRIUS (`iidx-14-to-17.zip`): [iidxhook3](doc/iidxhook/iidxhook3.md)
  - Beatmania IIDX 18 Resort Anthem (`iidx-18.zip`): [iidxhook4](doc/iidxhook/iidxhook4.md)
  - Beatmania IIDX 19 Lincle (`iidx-19.zip`): [iidxhook5](doc/iidxhook/iidxhook5.md)
  - Beatmania IIDX tricoro CN (狂热节拍 IIDX 2) (`iidx-20-cn.zip`):
    [iidxhook5-cn](doc/iidxhook/iidxhook5-cn.md)
  - Beatmania IIDX 20 Tricoro (`iidx-20.zip`): [iidxhook6](doc/iidxhook/iidxhook6.md)
  - Beatmania IIDX 21 SPADA (`iidx-21-to-24.zip`): [iidxhook7](doc/iidxhook/iidxhook7.md)
  - Beatmania IIDX 22 PENDUAL (`iidx-21-to-24.zip`): [iidxhook7](doc/iidxhook/iidxhook7.md)
  - Beatmania IIDX 23 copula (`iidx-21-to-24.zip`): [iidxhook7](doc/iidxhook/iidxhook7.md)
  - Beatmania IIDX 24 SINOBUZ (`iidx-21-to-24.zip`): [iidxhook7](doc/iidxhook/iidxhook7.md)
  - Beatmania IIDX 25 CANNON BALLERS (`iidx-25-to-26.zip`): [iidxhook8](doc/iidxhook/iidxhook8.md)
  - Beatmania IIDX 26 Rootage (`iidx-25-to-26.zip`): [iidxhook8](doc/iidxhook/iidxhook8.md)
  - Beatmania IIDX 27 Heroic Verse (`iidx-27-to-30.zip`): [iidxhook9](doc/iidxhook/iidxhook9.md)
  - Beatmania IIDX 28 BISTROVER (`iidx-27-to-30.zip`): [iidxhook9](doc/iidxhook/iidxhook9.md)
  - Beatmania IIDX 29 CASTHOUR (`iidx-27-to-30.zip`): [iidxhook9](doc/iidxhook/iidxhook9.md)
  - Beatmania IIDX 30 RESIDENT (`iidx-27-to-30.zip`): [iidxhook9](doc/iidxhook/iidxhook9.md)
- [jubeat](doc/jbhook/README.md)
  - jubeat (`jb-01.zip`): [jbhook1](doc/jbhook/jbhook1.md)
  - jubeat ripples (`jb-02.zip`): [jbhook1](doc/jbhook/jbhook1.md)
  - jubeat knit (`jb-03.zip`): [jbhook2](doc/jbhook/jbhook2.md)
  - jubeat copious (`jb-04.zip`): [jbhook2](doc/jbhook/jbhook2.md)
  - jubeat saucer (fulfill) (`jb-05-to-07.zip`): [jbhook3](doc/jbhook/jbhook3.md)
  - jubeat prop (`jb-05-to-07.zip`): [jbhook3](doc/jbhook/jbhook3.md)
  - jubeat qubell (`jb-05-to-07.zip`): [jbhook3](doc/jbhook/jbhook3.md)
  - jubeat clan (`jb-08.zip`): [jbhook3](doc/jbhook/jbhook3.md)
  - jubeat festo (`jb-08.zip`): [jbhook3](doc/jbhook/jbhook3.md)
- [pop'n music](doc/popnhook/README.md)
  - pop'n music 15 ADVENTURE (`popn-15-to-18.zip`) using [popnhook1](doc/popnhook/popnhook1.md)
  - pop'n music 16 PARTY♪ (`popn-15-to-18.zip`) using [popnhook1](doc/popnhook/popnhook1.md)
  - pop'n music 17 THE MOVIE (`popn-15-to-18.zip`) using [popnhook1](doc/popnhook/popnhook1.md)
  - pop'n music 18 せんごく列伝 (`popn-15-to-18.zip`) using [popnhook1](doc/popnhook/popnhook1.md)
- SOUND VOLTEX
  - SOUND VOLTEX BOOTH (`sdvx-01-to-04.zip`): sdvxhook
  - SOUND VOLTEX II -infinite infection- (`sdvx-01-to-04.zip`): sdvxhook
  - SOUND VOLTEX III GRAVITY WARS (`sdvx-01-to-04.zip`): sdvxhook
  - SOUND VOLTEX IV HEAVENLY HAVEN (`sdvx-01-to-04.zip`): sdvxhook
  - SOUND VOLTEX Vivid Wave (`sdvx-05-to-06`): sdvxhook2
  - SOUND VOLTEX EXCEED GEAR (`sdvx-05-to-06`): sdvxhook2

## Auxiliary tooling

- Bootstrapping
  - [inject](doc/inject.md): Inject arbitrary hooking libraries into a target application process.
  - [launcher](doc/launcher.md): Bootstrap Konami's AVS environment and launch a target application
    with arbitrary injected hooking libraries.
- Beatmnia IIDX Ezusb IO board
  - [ezusb-iidx-fpga-flash](doc/tools/ezusb-iidx-fpga-flash.md): Flash a binary blob with FPGA
    firmware to a target ezusb FX IO board
  - [ezusb-iidx-sram-flash](doc/tools/ezusb-iidx-sram-flash.md): Flash a binary blob with SRAM
    contents to a target ezusb FX2 IO board
- Exit hooks: Exit the game with a button combination using native cabinet inputs
  - [iidx-ezusb-exit-hook](doc/tools/iidx-ezusb-exit-hook.md): For IIDX with ezusb IO
  - [iidx-bio2-exit-hook](doc/tools/iidx-bio2-exit-hook.md): For IIDX with BIO2 IO
  - [iidx-ezusb2-exit-hook](doc/tools/iidx-ezusb-exit-hook.md): For IIDX with ezusb FX2 IO
- Bemanitools API testing: Tools for testing bemanitools API implementations
  - [ddriotest](doc/tools/ddriotest.md): For [ddrio API](doc/api.md#io-boards)
  - [eamiotest](doc/tools/eamiotest.md): For [eamio API](doc/api.md#eamuse-readers)
  - [iidxiotest](doc/tools/iidxiotest.md): For [iidxio API](doc/api.md#io-boards)
  - [jbiotest](doc/tools/jbiotest.md): For [jbio API](doc/api.md#io-boards)
- DDR IO testing: Tools for testing hardware of a real DDR cabinet
  - [p3io-ddr-tool](doc/tools/p3io-ddr-tool.md)
  - [extiotest](doc/tools/extiotest.md)
- [aciotest](doc/tools/aciotest.md): Command line tool to quickly test ACIO devices
- config: UI input/output configuration tool when using the default bemanitools API (geninput)
- ir-beat-patch-9/10: Patch the IR beat phase on IIDX 9 and 10
- [mempatch-hook](doc/tools/mempatch-hook.md): Patch raw memory locations in the target process
  based on the provided configuration
- [pcbidgen](doc/tools/pcbidgen.md): Konami PCBID generator tool
- [ViGEm clients](doc/vigem/README.md): Expose BT5 APIs as XBOX game controllers to play any games
  with real cabinet hardware.

## Pre-requisites

### Supported platforms

Our main platforms are currently Windows XP and Windows 7 which are also the target platforms on the
original hardware of those games. However, as it gets more difficult to get and maintain hardware
comptible with Windows XP, this might change in the future. Many games also run on very recent
Windows 10 builds but bear with us that it's hard to keep up with Windows updates breaking legacy
software.

### Distribution contents

Check the [list of supported games](#supported-games) to grab the right files for your game. BT5
also includes a *tools* subpackage (tools.zip) as well as the full source code (src.zip).

You will find \*.md files in various sub-packages that give you further instructions for setup,
usage, error information or FAQ. We advice you to read them as your questions and concerns might
already be answered by them. If not, let us know if there is any information that you consider
helpful or important to know and should be added.

### Setup and dependencies

The games bemanitools support span several generations of (embedded) Windows versions: XP, 7 and 10
(IoT). With these come different generations of dependencies as requirements.

Simplified, this boils down to the following list of C++ Redistributable (vcredist) packages that
are recommended to be installed without going into detail which game needs exactly which version.
Keep in mind the "bitness" (32-bit vs. 64-bit) of each specific game and download the right ones
accordingly.

You can use a tool like [dependencywalker](https://www.dependencywalker.com/) to figure that out if
it is relevant to you.

Links/files are also provided by
[bemanitools-supplement](https://github.com/djhackersdev/bemanitools-supplement/tree/master/misc/win-runtime#windows-runtimes-and-libraries)

#### Visual C++ Redistributable Packages

- 2010
  - [32-bit (x86)](http://download.microsoft.com/download/5/B/C/5BC5DBB3-652D-4DCE-B14A-475AB85EEF6E/vcredist_x86.exe)
  - [64-bit (x64)](http://download.microsoft.com/download/d/2/4/d242c3fb-da5a-4542-ad66-f9661d0a8d19/vcredist_x64.exe)
- 2013
  - [32-bit (x86)](http://download.microsoft.com/download/2/E/6/2E61CFA4-993B-4DD4-91DA-3737CD5CD6E3/vcredist_x86.exe)
  - [64-bit (x64)](http://download.microsoft.com/download/2/E/6/2E61CFA4-993B-4DD4-91DA-3737CD5CD6E3/vcredist_x64.exe)
- 2015
  - [32-bit (x86)](https://download.microsoft.com/download/9/3/F/93FCF1E7-E6A4-478B-96E7-D4B285925B00/vc_redist.x86.exe)
  - [64-bit (x64)](https://download.microsoft.com/download/9/3/F/93FCF1E7-E6A4-478B-96E7-D4B285925B00/vc_redist.x64.exe)

#### DirectX 9

Most/nearly all games support to this day use the DirectX 9(ex) API. Make sure to install the
[DirectX 9 End-User Runtimes (June 2010)](https://www.microsoft.com/en-us/download/details.aspx?id=8109)

## Development

### Building

See the [development document](doc/development.md).

### Architecture

A dedicate [architecture document](doc/architecture.md) outlines the architecture of Bemanitools and
points out the most important aspects you should know before you get started with development.

### API

Please refer to the [API documentation](doc/api.md).

## Release process

Please refer to the [dedicated documentation](doc/release-process.md).

## License

Source code license is the Unlicense; you are permitted to do with this as thou wilt. For details,
please refer to the [LICENSE file](LICENSE) included with the source code.
