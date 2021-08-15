# Konami Product Identifier (KPI)

Konami labels their products, i.e. full cabinet, a hardware + software upgrade kit, software update, for identification
using a propriatary format. These product codes are often seen denoting specific versions/iterations of software.

## Sources

* pigstall
* [bemaniwiki](http://bemaniwiki.com/)
* Game/cabinet manuals

## Product identifier format

Format string: `ptggg:d:s:r:e`

* `p`: product type
  * `G`: game (?)
* `t`: license type/eamuse participation
  * `C`: serial, roundplug, ikey / software shipped with cabinet
  * `E`: roundplug, ikey, etoken / upgrade kit or conversion kit
  * `K`: offline capability (?) / offline upgrade kit
  * `Q`: serial, ikey, etoken / rental unit (?)
  * `U`: etoken
* `ggg`: 3 digit gamecode, see [gamecodes and revisions section](#gamecodes-and-revisions)
* `d`: dest (locale)
  * `J`: Japan
  * `U`: US
  * `E`: Europe
  * `K`: Korea
  * `A`: Asia
  * `Y`: Indonesia
  * `C`: China
* `s`: spec (specification), describes a specific cabinet type and combination of hardware, e.g. PCB and IO boards
  * `A`, `B`, `C`, ...
* `r` rev (revision), software revision
  * `A`, `B`, `C`, ...
* `e`: ext (build date)
  * Format: `yyyymmddnn`
    * `yyyy` = year, e.g. `2020`
    * `mm` = month, e.g. `11` for November
    * `dd` = day of month, e.g. `03`
    * `nn` = build number on the day, e.g. `00`, `01`, ...

Note that older games are lacking the ext part/build date.

Example: `GQLDJ:J:A:A:2011050900`

* product type: `G`
* license type: `Q`
* gamecode: `LDJ` (Beatmania IIDX)
  * project start year: `2011`
  * remainder: `DJ`
* locale: `J` (Japan)
* cab specification: `A`
* revision: `A`
* build date: `2011/05/09`, first build of the day

## Gamecodes and revisions

* 3 digit game code
* Format `yxx`
  * `y`: `A` + (project start year - 2000) (`L`: 2011, `T`: 2019, etc.)
  * `xx`: Remainder, incremental version identifier?

### Beatmania IIDX

* Beatmania IIDX `863`
* Beatmania IIDX with DDR Club Kit `896`
* Substream `983`
* Club Version 2 `984`
* 2nd Style `985`
* 3rd Style `992`
* 4th Style `A03`
  * dest: `J`
  * spec: `A`
* 5th Style `A17`
  * dest: `J`
  * spec: `A`
* 6th Style `B4U`
  * dest: `J`
  * spec: `A`
* 7th Style `B44`
* 8th Style `C44`
  * dest: `J`
  * spec: `A`
* 9th Style `C02`
  * dest: `J`
  * spec
    * `A` (eamusement compatible): Bemani PC, C02 IO board, magnetic card readers, artwork, stage decoration unit (?)
    * `B` (eamusement compatible): Bemani PC, C02 IO board, magnetic card readers, artwork
    * `C` (offline): Bemani PC, C02 IO board, artwork
  * rev: `A`, `B`, `C`, `D`, `E`, `F`, `G`
* 10th Style `D01`
  * dest: `J`
  * spec: `A`
  * rev: `A`, `B`, `C`, `D`, `E`
* 11 RED `E11`
  * dest: `J`
  * spec
    * `A` (eamusement compatible): Bemani PC, C02 IO board, magnetic card readers, artwork
    * `B` (offline): Bemani PC, C02 IO board, artwork
  * rev: `A`, `B`
* 12 HAPPY SKY `ECO`
  * dest: `J`
  * spec
    * `A` (eamusement compatible): Bemani PC, C02 IO board, magnetic card readers, artwork
    * `B` (offline): Bemani PC, C02 IO board, artwork
  * rev: `A`, `B`, `C`, `D`
* 13 DistorteD `FDD`
  * dest: `J`
  * spec:
    * `A` (eamusement compatible): Bemani PC, C02 IO board, Slotted readers, artwork
    * `B` (offline): Bemani PC, C02 IO board, artwork
  * rev: `A`, `B`, `C`, `D`, `E`, `F`, `G`
* 14 GOLD `GLD`
  * dest: `J`
  * spec: `A`
  * rev: `A`
  * ext: 
* 15 DJ TROOPERS `HDD`
  * dest: `J`
  * spec: `A`
  * rev: `A`
  * ext
    * `2007112800`
    * `2007122500`
    * `2008020500`
    * `2008031100`
* 16 EMPRESS `I00`
  * dest: `J`
  * spec: `A`
  * rev: `A`
  * ext
    * `2008111900`
    * `2008112600`
    * `2008120800`
    * `2009010600`
    * `2009072200`
* 17 SIRIUS `JDJ`
  * dest: `J`
  * spec: `A`
  * rev: `A`
  * ext
    * `2009102100`
    * `2009102300`
    * `2009111700`
    * `2009122100`
    * `2010071200`
* 18 Resort Anthem `JDZ`
* 19 Lincle `KDZ`
* 20 Tricoro `LDJ`
  * ext
    * First software revision: `2012091904`
    * Last software revision: `2013090900`
* 21 SPADA `LDJ`
  * ext
    * First software revision: `2013100200`
    * Last software revision: `2014071600`
* 22 PENDUAL `LDJ`
  * ext
    * First software revision: `2014091700`
    * Last software revision: `2015080500`
* 23 copula `LDJ`
  * spec
    * `C`
  * ext
    * First software revision: `2015111100`
    * Last software revision: `2016102000`
* 24 SINOBUZ `LDJ`
  * ext
    * First software revision: `2016102400`
    * Last software revision: `2017071800`
* 25 CANNON BALLERS `LDJ`
  * ext
    * First software revision: `2017122100`
    * Last software revision: `2018103100`
* 26 Rootage `LDJ`
  * ext
    * First software revision: `2018110700`
    * Last software revision: `2019100700`
* 27 Heroic Verse `LDJ`
  * ext
    * First software revision: `2019101600`
    * Last software revision: `2020102200`
