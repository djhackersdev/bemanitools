# Bemanitools 5
[![pipeline status](https://dev.s-ul.eu/djhackers/bemanitools/badges/master/pipeline.svg)](https://dev.s-ul.eu/djhackers/bemanitools/commits/master)

Version: 5.28</br>
[Release history](CHANGELOG.md)

A collection of tools to run [various Bemani arcade games](#list-of-supported-games).

Bemanitools 5 (BT5) is the successor to Bemanitools 4 which introduces a big code cleanup and support for newer games.
BT5 uses a cleaner approach than BT4 did; specifically, all input and lighting is handled by emulating the protocols
spoken by the real IO PCBs, instead of replacing chunks of game code like BT4. The benefits of this approach are a more
authentic gameplay experience, and easier support for a broader range of releases from each game series.

# List of supported games
* BeatStream
    * BeatStream (bst.zip) using bsthook
    * BeatStream アニムトライヴ (bst.zip) using bsthook
* Dance Dance Revolution
    * Dance Dance Revolution X2 (ddr-12-to-16.zip) using ddrhook
    * Dance Dance Revolution X3 vs. 2ndMIX (ddr-12-to-16.zip) using ddrhook
    * Dance Dance Revolution 2013 (ddr-12-to-16.zip) using ddrhook
    * Dance Dance Revolution 2014 (ddr-12-to-16.zip) using ddrhook
    * Dance Dance Revolution A (ddr-12-to-16.zip) using ddrhook
* Beatmania IIDX
    * Beatmania IIDX 9th Style (iidx-09-to-12.zip) using [iidxhook1](doc/iidxhook/iidxhook1.md)
    * Beatmania IIDX 10th Style (iidx-09-to-12.zip) using [iidxhook1](doc/iidxhook/iidxhook1.md)
    * Beatmania IIDX 11 IIDX RED (iidx-09-to-12.zip) using [iidxhook1](doc/iidxhook/iidxhook1.md)
    * Beatmania IIDX 12 HAPPY SKY (iidx-09-to-12.zip) using [iidxhook1](doc/iidxhook/iidxhook1.md)
    * Beatmania IIDX 13 DistorteD (iidx-13.zip) using [iidxhook2](doc/iidxhook/iidxhook2.md)
    * Beatmania IIDX 14 GOLD (iidx-14-to-17.zip) using [iidxhook3](doc/iidxhook/iidxhook3.md)
    * Beatmania IIDX 15 DJ TROOPERS (iidx-14-to-17.zip) using [iidxhook3](doc/iidxhook/iidxhook3.md)
    * Beatmania IIDX 16 EMPRESS (iidx-14-to-17.zip) using [iidxhook3](doc/iidxhook/iidxhook3.md)
    * Beatmania IIDX 17 SIRIUS (iidx-14-to-17.zip) using [iidxhook3](doc/iidxhook/iidxhook3.md)
    * Beatmania IIDX 18 Resort Anthem (iidx-18.zip) using [iidxhook4](doc/iidxhook/iidxhook4.md)
    * Beatmania IIDX 19 Lincle (iidx-19.zip) using [iidxhook5](doc/iidxhook/iidxhook5.md)
    * Beatmania IIDX 20 Tricoro (iidx-20.zip) using [iidxhook6](doc/iidxhook/iidxhook6.md)
    * Beatmania IIDX 21 SPADA (iidx-21-to-24.zip) using [iidxhook7](doc/iidxhook/iidxhook7.md)
    * Beatmania IIDX 22 PENDUAL (iidx-21-to-24.zip) using [iidxhook7](doc/iidxhook/iidxhook7.md)
    * Beatmania IIDX 23 copula (iidx-21-to-24.zip) using [iidxhook7](doc/iidxhook/iidxhook7.md)
    * Beatmania IIDX 24 SINOBUZ (iidx-21-to-24.zip) using [iidxhook7](doc/iidxhook/iidxhook7.md)
    * Beatmania IIDX 25 CANNON BALLERS (iidx-25-to-26.zip) using [iidxhook8](doc/iidxhook/iidxhook8.md)
    * Beatmania IIDX 26 Rootage (iidx-25-to-26.zip) using [iidxhook8](doc/iidxhook/iidxhook8.md)
* jubeat
    * jubeat (experimental/buggy) (jb-01.zip) using [jbhook1](doc/jbhook1/jbhook1.md)
    * jubeat saucer (fulfill) (jb-05-to-07.zip) using jbhook
    * jubeat prop (jb-05-to-07.zip) using jbhook
    * jubeat Qubell (jb-05-to-07.zip) using jbhook
    * jubeat clan (jb-08.zip) using jbhook
* SOUND VOLTEX
    * SOUND VOLTEX BOOTH (sdvx.zip) using sdvxhook
    * SOUND VOLTEX II -infinite infection- (sdvx.zip) using sdvxhook
    * SOUND VOLTEX III GRAVITY WARS (sdvx.zip) using sdvxhook
    * SOUND VOLTEX IV HEAVENLY HAVEN (sdvx.zip) using sdvxhook

# Supported platforms
Our main platforms are currently Windows XP and Windows 7 which are also the target platforms on the original hardware
of those games. However, as it gets more difficult to get and maintain hardware comptible with Windows XP, this might
change in the future. Many games also run on very recent Windows 10 builds but bear with us that it's hard to keep up
with Windows updates breaking legacy software.

# Distribution contents
Check the [list of supported games](#list-of-supported-games) to grab the right files for your game. BT5 also includes
a *tools* subpackage (tools.zip) as well as the full source code (src.zip).

You will find *.md files in various sub-packages that give you further instructions for setup, usage, error information
or FAQ. We advice you to read them as your questions and concerns might already be answered by them. If not, let us
know if there is any information that you consider helpful or important to know and should be added.

# Contributions
Please refer to the [dedicated documentation](CONTRIBUTING.md).

# Development
## Architecture
A dedicate [architecture document](doc/architecture.md) outlines the architecture of Bemanitools and points out the most
important aspects you should know before you get started with development.

## API
Please refer to the [API documentation](doc/api.md).

## Source Code
The source code is included with this distribution package (src.zip). Please refer to the
[development document](doc/development.md) for further details.

# License
Source code license is the Unlicense; you are permitted to do with this as thou wilt. For details, please refer to the
[LICENSE file](LICENSE) included with the source code.






