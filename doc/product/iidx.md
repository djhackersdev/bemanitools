# Beatmania IIDX products

This document outlines a list of products identified by [Konami product identifiers](#../kpi.md) relevant to the 
Beatmania IIDX game series.

## Cabinets

Cabinets are both the product and hardware as the unit is sold as a whole. The cabinet body identifiers are located on
a sticker at the back of the cabinet, e.g. close to the main power socket.

The following lists identifies differnt cabinet bodies and their stock hardware and software configuration they
initially shipped with, aka dedicated cabinets. According to major hardware/software transitions, they are categorized
in generations which is a non-Konami terminology.

### Generation 1 cabinet

| [KPI](../kpi.md) | Game      | Main PCB                  | IO                        | Monitor                                             |
|------------------|-----------|---------------------------|---------------------------|-----------------------------------------------------|
| `GQ863-JA`       | 1st Style | [Twinkle V1](#twinkle-v1) | [Twinkle IO](#twinkle-io) | [Toshiba rear projection](#toshiba-rear-projection) |
| `GQ983-JA`       | Substream | [Twinkle V1](#twinkle-v1) | [Twinkle IO](#twinkle-io) | [Toshiba rear projection](#toshiba-rear-projection) |
| `GQ985-JA`       | 2nd Style | [Twinkle V1](#twinkle-v1) | [Twinkle IO](#twinkle-io) | [Toshiba rear projection](#toshiba-rear-projection) |
| `GQ992-JA`       | 3rd Style | [Twinkle V2](#twinkle-v2) | [Twinkle IO](#twinkle-io) | [Toshiba rear projection](#toshiba-rear-projection) |

* Type of cabinet easily recognizable by grey colored front/coin door

### Generation 2 cabinet

| [KPI](../kpi.md) | Game        | Main PCB                              | IO          | Monitor               |
|------------------|-------------|---------------------------------------|-------------|-----------------------|
| `GQD01-JA`       | 10th Style  | [Bemani PC Type 1](#bemani-pc-type-1) | [D01](#d01) | [CRT](#CRT)           |
| `GQE11-JA`       | RED         | [Bemani PC Type 1](#bemani-pc-type-1) | [D01](#d01) | [CRT](#CRT)           |
| `GQECO-JA`       | HAPPY SKY   | [Bemani PC Type 1](#bemani-pc-type-1) | [D01](#d01) | [CRT](#CRT)           |
| `GQFDD-JA`       | DistorteD   | [Bemani PC Type 1](#bemani-pc-type-1) | [D01](#d01) | [CRT](#CRT)           |
| `GQGLD-JA`       | GOLD        | [Bemani PC Type 2](#bemani-pc-type-2) | [IO2](#io2) | [CRT](#CRT)           |
| `GQHDD-JA`       | DJ TROOPERS | [Bemani PC Type 2](#bemani-pc-type-2) | [IO2](#io2) | [GUHDD-JB](#GUHDD-JB) |
| `GQI00-JA`       | EMPRESS     | [Bemani PC Type 2](#bemani-pc-type-2) | [IO2](#io2) | [GUHDD-JB](#GUHDD-JB) |
| `GQJDZ-JA`       | SIRIUS      | [Bemani PC Type 2](#bemani-pc-type-2) | [IO2](#io2) | [GUHDD-JB](#GUHDD-JB) |

* Type of cabinet recognizable by black colored front/coin door

### Generation 3 cabinet

#### `GQLDJ-JA`

* Game: Tricoro
* New Tricoro LCD

#### `GQLDJ-JB`

* Pendual cab?
* New pendual LCD

#### `GQLDJ-JC`

* Game: COPULA
* Introduced new Copula LCD monitor
* Introduced LED spotlights and neons?

### Generation 4 cabinet

#### `GQTDJ-JA`

* "Lightning model"
* Game: HEROIC VERSE

## Upgrades

Officially called "conversation kits" but usually referred to as upgrade kits or simply updates. Depending on the
product, this can be pure software updates but also include hardware upgrades.

### 8th Style upgrade

#### `GCC44-JA`

* (Software) version requirements
  * `GC983`
  * `GU985`
  * `GC985`
  * `GC992`
  * `GCA03`
  * `GCA17`
  * `GCB4U`
  * `GCB44`
* Contents
  * Program CD
  * Video DVD
  * Security chip
  * Game data HDD
  * Turntable refurbishing parts
  * Artwork
    * Game instructions
    * Speaker side-pops
    * Marquee
* Supported cabinets
  * [Generation 1 cabinet](#generation-1-cabinet)
* Supported Main PCB
  * [Twinkle V1](#twinkle-v1)
  * [Twinkle V2](#twinkle-v2)
* Supported IO
  * [Twinkle IO](#twinkle-io)

### 9th Style upgrade

9th Style "machines" were essentially just upgrades from any [generation 1 cabinet](#generation-1-cabient) cabinet using
either [Twinkle V1](#twinkle-v1) or [Twinkle V2](#twinkle-v2) hardware.

* (Software) version requirements
  * `GQ863`
  * `GC983`
  * `GC985`
  * `GC992`
  * `GCA03`
  * `GCA17`
  * `GCB4U`
  * `GCB44`
  * `GCC44`
* Contents: Differ based on package
* Supported cabinets
  * [Generation 1 cabinet](#generation-1-cabinet)
* Supported Main PCB
  * [Bemani PC Type 1](#bemani-pc-type-1)
* Supported IO
  * [C02](#c02)

#### `GEC02-JA`

* Eamuse compatible
* [Bemani PC Type 1](#bemani-pc-type-1)
* [C02](#c02)
* Magnetic card readers
* Cabinet artwork

#### `GEC02-JB`

* Eamuse compatible
* [Bemani PC Type 1](#bemani-pc-type-1)
* [C02](#c02)
* Magnetic card readers

#### `GEC02-JC`

* Eamuse compatible
* [Bemani PC Type 1](#bemani-pc-type-1)
* [C02](#c02)

### RED upgrade

`GEE11-JA`
`GEE11-JB`

### Spada upgrade

#### `GULDJ-JF`

### Pendual upgrade

#### `GULDJ-JH`

* Update DVD: `LDJ JC A01`
* Artwork

### Cannon Ballers upgrade

#### `GELDJ-JM`

* Compatible with `GQLDJ-JA`, `GQLDJ-JA`, `GQLDJ-JA` [generation 3 cabinets](#generation-3-cabinet)
* Contents
  * Main PCB: [Bemani PC ADE-6291](../parts/iidx.md#bemani-pc-ade-6291)
  * IO: [BIO2](../parts/iidx.md#bio2) and [BIO2 sub IO](../parts/iidx.md#bio2-sub-io)
  * Camera
  * Replacement bass platform artwork
  * Cabinet artwork
  * Wiring for compatible cabinet types

#### `GELDJ-JN`

Identical to `GELDJ-JO` but also includes a VGA to HDMI converter cable for old style monitors, e.g.
[Toshiba rear projection](../parts/iidx.md#toshiba-rear-projection) and [CRT](../parts/iidx.md#crt).

#### `GELDJ-JO`

* Compatible with [generation 1 cabinets](#generation-1-cabinet) and [generation 2 cabinets](#generation-2-cabinet)
  * Assumes other required hardware upgrades, e.g. card readers, available
* Contents
  * [Bemani PC ADE-6291](../parts/iidx.md#bemani-pc-ade-6291)
  * IO: [BIO2](../parts/iidx.md#bio2) and [BIO2 sub IO](../parts/iidx.md#bio2-sub-io)
  * Camera
  * Replacement bass platform artwork
  * Cabinet artwork
  * Wiring for compatible cabinet types
