# IIDX engine overview and how game-play is impacted by hardware and software

Date: 2025-02-09 Author: icex2

This document provides information and explains the following:

* A sort-of taxonomy of the different major IIDX game engine versions
* How the core game-play part of the engine works regarding
  * Threading model
  * Main render loop
  * Timing and synchronization

The goal is to capture key knowledge about how the game's engine works to enable a better
understanding for developing bemanitools and how certain features can impact performance and
synchronization during game-play.

This document is not claiming to be complete or 100% accurate. It is based on my own personal
research and understanding of the IIDX game engine. It is very likely that some of the information
is wrong or misunderstood. Feel free to raise any questions or concerns in an issue as I want
this document to be as accurate as possible.

## Taxonomy of IIDX game engine versions

With every version, there have been numerous changes and improvements to the game engine. The
following is a rough taxonomy that is focused on cabinet and hardware changes. As these are likely
the main driver for key changes to the software and game engine, they create fairly distinct
stages throughout the different versions of the game. The key attributes to be considered here are:

* Main PCB
* IO board
* Monitor

All of these had significant impact on key software features appearing or changing.

### Twinkle hardware era: 1st Style to 8th Style

* [Twinkle PCB](https://github.com/shizmob/arcade-docs/blob/main/konami/boards.md#twinkle)
* [Original rear projection monitor](https://github.com/shizmob/arcade-docs/blob/main/konami/products.md#iidx-rear-projection-monitor)
* Homogenous hardware and software

### 1st gen PC-based upgrade only: 9th Style

* 9th Style came as an upgrade kit only to existing twinkle-based cabinet hardware
* [KNM-845G3-A02 PCB](https://github.com/shizmob/arcade-docs/blob/main/konami/boards.md#knm-845g3-a02)
* [C02 EZUSB FX USB IO board](https://github.com/shizmob/arcade-docs/blob/main/konami/io.md#gec02-pwbaa)
* Magnetic card readers connected to ezusb IO board

### 1st gen PC-based with dedicated cabinet: 10th Style to DistorteD (13)

* From here on, old cabinets that received the "Twinkle hardware" upgrade kit could always receive software only
  upgrades
* [CRT monitor](https://github.com/shizmob/arcade-docs/blob/main/konami/products.md#iidx-crt-monitor) with new dedicated
  cabinets
* [D01 EZUSB FX USB IO board](https://github.com/shizmob/arcade-docs/blob/main/konami/io.md#d01-io)
* Slotted card readers starting version 13

### 2nd gen PC: GOLD (14) to Lincle (19)

* [FAB-e945-KN205](https://github.com/shizmob/arcade-docs/blob/main/konami/boards.md#fab-e945-kn205)
* All prior cabinet configurations can be upgraded with a PCB and software upgrade
* New LCD monitor starting version 15
  * Further variants of the LCD monitor appear starting version 18
* [IO2 EZUSB FX2 USB IO board](https://github.com/shizmob/arcade-docs/blob/main/konami/io.md#usbio2) introduced with
  version 14
* Switch to wave pass readers on version 19

### 3rd gen PC: Tricoro (20) to Sinobuz (24)

* All prior cabinet configurations can be upgraded with a PCB and software upgrade
* [ADE-HM65](https://github.com/shizmob/arcade-docs/blob/main/konami/boards.md#hm65)
* Game engine supports HD mode (720p)
* Many new LCD monitor variants appear with every new version

### 4th gen PC: CANNON BALLERS (25) to RESIDENT (30)

* All prior cabinet configurations can be upgraded with a PCB and software upgrade
* [ADE-6291](https://github.com/shizmob/arcade-docs/blob/main/konami/boards.md#ade-6291)
* [BIO2 IO board](https://github.com/shizmob/arcade-docs/blob/main/konami/boards.md#hm65)
  * Upgrade cabinets required the [BIO2 sub-IO board](https://github.com/shizmob/arcade-docs/blob/main/konami/io.md#bio2-ldj-sub-io)
* Drop of SD mode support starting version 27
* Supporting 120 hz monitors starting version 27
* Many new LCD monitor variants appear with every new version
  * 42" 120 hz for lightning cabinets starting version 27
  * 42" 60 hz for non-lightning cabinets starting version 25

### 5th gen PC: RESIDENT (30) to EPOLIS (31)

* All prior cabinet configurations can be upgraded with a PCB and software upgrade
* [C300-xonarae](https://github.com/shizmob/arcade-docs/blob/main/konami/boards.md#c300-xonarae)
* Game engine supports FHD mode (1080p)

## Threading model and core responsibilities in PC-based games

Since the early days of 9th Style to today (as of time of writing this: EPOLIS), the threading model
hasn't changed significantly:

* Main and rendering thread
  * Runs your typical D3D9 render loop
    * Using D3D9ex starting version 27
  * Starting and managing other threads
  * Any file and game asset loading
  * Handles all the core game logic of the different screens starting from the initial boot screen
    to the core game-play screen
* IO thread
  * Polls the main IO hardware and synchronizes the in-memory input and output state with the
    hardware
  * Executes commands issued by the main thread to the IO board such as dongle or card reader
    commands (ezusb generation of IO only)
* Card reader thread (not applicable to versions 9 to 12)
  * Runs the protocol to read/write the hardware of the readers
  * Async execution of higher level commands issued by the main thread to the card reader hardware,
    e.g. card read, card eject etc.
* Audio thread
  * Streaming of audio data to the sound API
    * DirectSound for versions 9 to 26
    * WASAPI for versions 27 to 30 for non-lighting cabinets with ADE-6291 hardware
    * ASIO for versions 27 and newer for lighting cabinets
    * ASIO for version 30/31 and newer for non-lighting cabinets with C300-xonarae hardware
  * Async execution of higher level commands issued by the main thread to the audio backend, e.g.
    play audio, stop audio, etc.
* Network thread
  * Network plumbing with the xrpc protocol
  * Async execution of request-response commands issued by the main thread

## Main and rendering thread

With the game having transitioning to different screens throughout it's lifecycle, the main render
loop might contain different logic such as asset loading. The following focuses on the main
game-play screen, only. Common rendering logic still applies to other screens.

This follows your typical D3D9 single threaded rendering loop which boils down to the following:

* Begin the scene (`BeginScene`)
* Set the render target (`SetRenderTarget`)
* Clear the back buffer (`Clear`)
* Run engine step. This includes and is mixed with
  * IO input evaluation
  * Game engine state and (re-) drawing the scene
  * Commands to the audio playback, e.g. play key sounds
  * Setting IO outputs
* End the scene (`EndScene`)
* Target a minimum frame time
  * Introduced with version 11
  * Sleep (`Sleep` or `SleepEx`) to fill up the frame up to either 13 ms or 14 ms (depending on the
    version)
  * This was likely introduced to not have the game run at hundreds of FPS if v-sync was disabled
    (probably when running in window mode for development/testing purposes)
* Swapping the back buffers (`Present`)
  * D3D9 is configured with v-sync enabled by default and targets 60 hz
  * With this configuration, the call to `Present` fills up the remaining frame time to target the
    configured 60 hz refresh rate

### Rendering resolutions

The game engine went through a couple of different rendering resolutions:

* 640x240: version 1 to 8 (all twinkle hardware)
* 640x480: version 9 to 19 (1st and 2nd gen PC-based)
* 1280x720 (HD mode) and 640x480 (SD mode): version 20 to 27 (3rd and 4th gen PC-based)
* 1280x720 (HD mode) only for version 28 to 29 (4th gen PC-based)
* 1280x720 (HD mode) and 1920x1080 (FHD mode) for version 30 (4th gen PC-based)
* 1920x1080 (FHD mode) only for version 30+ (5th gen PC-based)

## Core engine timing and synchronization

The following outlines and explains the core engine concepts related to timing and synchronization.

### The origins, twinkle hardware

* It appears that the game has been on the same codebase since it's inception.
* This can be reasoned by the following observations
  * 9th style as the first PC-based game very buggy on initial release 
  * 9th style threading model very clunky with various thread settings showing they tried to
    prioritize different processing on a single core CPU 
  * Coming from twinkle hardware that is a realtime system vs. a Windows XP system
    * Fixed playstation 1 based hardware with a fixed refresh rate of 60.925 hz
    * No need to assume getting anything else if the resource budget per frame is not exceeded
    * All sub-systems were driven synchronously to that fixed time step
    * No need for the engine to synchronize audio during the song. When the song is started
      correctly, it stayed sync throughout the song
* It appears the developers struggled a lot with the significant platform shift to PC as indicated
  by various core engine improvements throughout the years still
* Switches of hardware generations further complicated the situation as these forced them to further
  drift away from how the game engine was initially designed on the twinkle system

### Frame time/timing and engine step

* The game's engine step and timing is frame based
* The main render loop defines the minimum valid timing window for the game engine to be able to
  evaluate
  * 60 hz/fps = 16.667 ms (rounded to three decimal places)
  * 120 hz/fps = 8.333 ms (rounded to three decimal places), only relevant for lightning cabinets with their 120 hz
    screen/mode
* Depending on the GPU hardware and configuration, the GPU might drive the screen not exactly at
  perfectly 60 hz/fps = 16.667 ms
* As outlined in the [main render loop](#main-and-rendering-thread), the `Present` call is the
  key function call to determine the frame time for the game engine
* Just having v-sync enabled and telling D3D9 to **target** 60 hz, it still depends on how the GPU
  driver is implemented and how the GPU driver's implementation of D3D9 executes on the `Present`
  call
* This is primarily determined by how the GPU driver is operating and driving connected display
  hardware
* The following factors can contribute to that either directly or indirectly
  * Different monitor types analog vs. digital (CRT vs. LCD)
  * Different monitor connections VGA vs. DVI/HDMI/DP
  * Different GPU drivers
  * Different GPU hardware and vendors
  * Different GPU settings

### Monitor check screen

The monitor check screen was introduced with version 14. This was a solution to the problem of not
guaranteeing a single fixed refresh rate throughout different GPU and monitor combinations. See
a detailed explanation in the previous sub-subsection about
[what influences the frame time](#frame-timetiming-and-engine-step) of the game.

The goal of the monitor check is to measure the frame time of the main render loop and use the
results for [driving the core engine step](#engine-step-and-synchronization).

The monitor check evolved slightly as different GPU and monitor hardware, and GPU drivers yielded
varying refresh rates that determined the frame times of the main game loop.

Actual values are outlined further down in the [engine step time](#engine-step-time) section.

There are two main versions of the monitor check, all of them have always been part of the boot
process:

* 14
  * Measures a total of 3000 frames
  * If total time this took < 50004 ms -> so average of 16.668 ms per frame =  59.9952 hz
    * S-Video mode -> 59.95 hz (also shows S-VIDEO when the monitor check completes)
    * Otherwise VGA mode -> 60.05 hz
  * If VGA is determined, the game patches the chart event data when loading
    * `event_timestamp * 0.99817199 + 0.40000001` 
  * When S-Video no charts are patched and the timing data in the chart is used as is
* 15 to 18
  * Identical to 14 except that chart patching now happens in milliseconds instead of frames
  * VGA mode: `16.65279`
  * S-Video mode: `16.680567`
  * `event_timestamp * (1.0 / frame_time) + 0.40000001)`
* 19
  * Identical to previous version
  * Monitor check screen shows current FPS on the last revision of that version. However, it doesn't
    use the value shown there by any means. Assuming this was added before the introduction of the
    new monitor check on 20.
* 20 to 26
  * Runs on boot or when switching monitor modes SD vs. HD 
  * Measures only 1200 frames
  * Takes the average refresh rate of 1200 frames
  * Applies the measured avg. frame time directly to the charts before song
* 27 to 30 
  * Frame rate measuring identical to prior versions
  * Added additional logic after the frame time measurements for handling of 120 hz displays 
  * Includes checking the monitor and if the framerate matches as expected
* 30/31 LDJ-010 upgrade
  * Added NvDisplayConfigLDJ tool which runs on system start and configures a custom timing in the
    NVIDIA GPU driver to ensure a fixed monitor refresh rate
* 31
  * Same code as before
  * Consider the first 460 frames of the measurement as warm-up frames
  * Discard these for the actual measurement to get more stable frame times
  * Use a base avg. value of 119.982 for the 120 hz monitors and 59.9 for 60 hz monitors. Further
    frames measured are counted towards the average with the given base value

### Engine core game-play with visual and audio synchronization

#### Chart data

* Chart data encodes note timing and timing window information
* Chart data consists of *note events* encoding a timestamp and note information such as type
  and key-sound information
* Timing windows are defined per chart
* Most infamous example: GAMBOL (7Key/Another)
* Game engine timestamp evaluation and note event timestamp encoding
  * 9 to 14: Refresh rate in hz
  * 15 to 31: Milliseconds (equivalent to assuming 1000 fps)
* Fundamentally, that doesn't change game play experience as it's just a different way of
  evaluating the timestamps since refresh rate in hz/frame rate in fps can be converted lossless to
  frame time in ms

#### Engine step time

The engine uses the following source for frame rate/time to drive internal logic on the following
versions

* 9 to 13: Hardcoded 59.95 hz
* 14: Either 59.95 hz or 60.05 hz determined by which value is determined to be closer to the
  measured one during the monitor check screen on startup
* 15 to 19: 59.95 hz or 60.05 hz
  * Monitor check on startup measures the frame time and picks the value closest to what's been
    measured
  * Monitor check result can be overridden with the *Output Type* setting in the operator menu
    * *S-Video* to force to 59.95 hz
    * *VGA* to force to 60.05 hz
* 20 to 31: Monitor check on boot determines the frame time and uses the result throughout the
  entire game session

#### Engine step and synchronization

Per frame engine step advancing and synchronization during game-play

* Game-play needs synchronization of the following aspects
  * Drawn content on the screen
  * Background audio playback track
  * Key sound effects
* The game engine synchronizes these only at the start of a song
  * It takes known/determined refresh rate/frame time as the fixed time for every frame cycle
    throughout the entire song
  * It starts playback of the background audio track together with playback of the chart data
  * With every engine step/frame it advances internal calculations by that fixed amount of time.
    Thus, the game engine expects this to be constant/stable throughout an entire song
  * The chart data is advanced every frame by the engine
  * Key sound events are turned into audio playback on command as the chart advances
  * No re-synchronization or check-pointing between main rendering thread and audio is happening
    throughout the song
* Remark: The above does not consider any kind of offsetting and timing adjustments for the sake
  of keeping this simple and focus on the absolute core concepts 