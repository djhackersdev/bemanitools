# IIDX engine overview and how game-play is impacted by hardware and software

Date: 2025-02-09 Author: icex2

This document provides information and explains the following:

* A taxonomy of the different major IIDX game engine versions
* How the iidx game engine works from a high level perspective
  * Different threads running and tasks they are doing
* How the main IIDX render loop works (roughly)
* How song synchronization of render loop with audio on IIDX works
* How the monitor check works
* Explaining known symptoms impacting game play experience and linking to root causes and known solutions
  * Song slowly de-syncing
  * Song off-sync from the start
  * Micro-stuttering
  * Non-smooth note scrolling

This document is not claiming to be complete or 100% accurate. It is based on my own personal
research and understanding of the IIDX game engine. It is very likely that some of the information
is wrong or misunderstood. Feel free to raise any questions or concerns in an issue as I want
this document to be as accurate as possible.

## Taxonomy of IIDX game engine versions

With every version, there have been numerous changes and improvements to the game engine. The
following is a rough taxonomy that is based on cabinet and hardware changes. As these are likely
the main driver for key changes to the software and game engine, they create fairly distinct
eras throughout the different versions of the game. The key attributes to be considered here are:

* Main PCB
* IO board
* Monitor

All of these had significant impact on key software features appearing or changing.

### Twinkle hardware era: 1st Style to 8th Style

### 



assumed engine generations (TODO verify):
* 1-8 (SD era with twinkle hardware): Talk about this to set the ground and where everything was
  coming from. explain how the hardware operated, the realtime requirements and how the hardware met
  them. This also likely set the understanding of the devs and the expectations for the game
  engine and how everything behaves and works software-wise, but coupled very tightly to the hardware
  requirements (which is one of the key root causes for all the mess that came after that with the
  PC platform)
* 9: first game to use a PC and had serious issues with the major hardware shift. having this separate because of a few
  differences: fully unlocked frame rate possible, they didn't have a sleep part of code to slow it down if vsync wasn't
  enabled for some reason
* 10-13 (SD era with ezusb1), 59.95 hz expected, two officially supported monitors: old rearprojection or CRT (TODO link arcade docs)
* 14: first game with monitor check but no options in operator menu, two officially supported monitors: old rearprojection or CRT (TODO link arcade docs). this versions seems to have been a preparation already for introduction of a new monitor type LCD
* 15 - 18: (SD era with ezusb1/2, monitor check added with options in the OP menu to set svideo or vga, new LCD monitor type, also introducing monitor type A and B which compensates in software for the LCDs lag (A is CRT, B is LCD))
* 19: Still the same as 15 to 18 but the last revision of 19 had some test code added for the "modern" monitor check. however, that code was purely for testing purpose and the values shown on boot are never used
* 14 - 19 (SD era with ezusb1/2, monitor check added)
* 20 - 24 (HD era with ezusb1/2, auto monitor check added, supports SD and HD mode, various different LCD monitors have been released,
  the monitor type A and B option doesn't exist anymore (?) TODO check that
* 25 - 26 (non lightning but bio2)
* 27 - 29 (two cabint types, bia2x added, 120 fps mode, two different hardware platforms for each cabinet type)
* 30-31 (FHD mode)


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

## Core engine timing and synchronization

The following outlines and explains the core engine concepts related to timing and synchronization.

### The origins, twinkle hardware

* It appears that the game has been on the same codebase since it's inception.
* This can be reasoned by the following things
  * 9th style as the first PC-based game very buggy on initial release 
  * 9th style threading model very clunky with various thread settings showing they tried to
    prioritize different processing on a single core CPU 
  * Coming from twinkle hardware that is a realtime system vs. a Windows XP system
    * Fixed playstation 1 based hardware
    * Render loop fixed at refresh rate of XXX TODO lookup
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

* 14:
  * Measures a total of 3000 frames
  * If total time this took < 50004 ms -> so average of 16.668 ms per frame =  59.9952 hz
    * S-Video mode -> 59.95 hz (also shows S-VIDEO when the monitor check completes)
    * otherwise VGA mode -> 60.05 hz
  * If VGA is determined, the game patches the chart when loading
    * `timestamp * 0.99817199 + 0.40000001` 
  * When S-Video no charts are patched and the timing data in the chart is used as is

* 15:
  * Identical to 14 except that chart patching now happens in milliseconds instead of frames
  * VGA mode: `16.65279`
  * S-Video mode: `16.680567`
  * `timestamp * (1.0 / flt_29C53F4) + 0.40000001)`

* 19: Identical to previous version. Monitor check screen shows current FPS on the last revision
    of that version. However, it doesn't use the value shown there by any means. Assuming this was
    added before the introduction of the new monitor check on 20.

* 20+
  * Runs on boot or when switching monitor modes SD vs. HD 
  * Measures only 1200 frames
  * Takes the average refresh rate of 1200 frames
  * Applies the frame time directly to the charts before song

* 27 to 30 
  * Frame rate measuring identical to prior versions
  * added additional logic for handling the 120 hz mode (after the measurements) that includes monitor info and errors if not matching

* 30/31 LDJ-010 upgrade
  * LDJMonitorConfiguration configured the frame rate and fixed it properly which improved accuracy

* 31
  * Same code as before
  * Added warm-up frames to stabilize the frame rate measurement
  * The first 460 frames are discarded and a base avg. value of 119.982 for the 120 hz monitors and 59.9 for 60 hz monitors
    is set. Further frames measured are counted towards the average.

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