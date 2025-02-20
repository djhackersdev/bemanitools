# IIDX "Syncbook": A guide to ensuring sync and performant gameplay

This is a guide applicable to all IIDX versions that are native Windows PC-based. The goal is to 
enable proper configuration of those games to ensure the game engine "runs in-sync" with any of its
IO (visual GPU/display output, audio output, user input) resulting in high and stable performance
for smooth game-play.

This document won't go into the implementation details of the game engine. Please refer to the
[IIDX engine details journal](../dev/journal/2025-02-09-iidx-engine.md) for a more in-depth
information.

## Understanding the problems and options to address them

First, let's understand what the key problems are and how they contribute to different issues
that impact game-play.

### Song de-syncing

Symptom: Throughout playing a song, the audio is de-syncing even it was fine when the song started.

Cause: The game engine is fundamentally designed to run on a fixed and consistent frame
rate/time per engine step. A mismatch of expected frame time by the game engine and actually delivered
frame time by the GPU driver. This can come either as a stable frame time that just mismatches in
general or (highly) fluctuating frame times.

* G-sync on modern displays causes known fluctuations because it adjusts dynamically the monitor
  refresh rate
* V-sync not correctly configured/enforced or even vsync-off override in driver is causing
  inconsistent frame times
* Incorrect display timings causing refresh rate fluctuations resulting in inconsistent frame times

Stable frame times but not having the correct ones that the game engine expects is a different
problem that can be addressed by:

* Knowing the refresh rates expected by the game engine of the different game versions
* Configuring your GPU driver's display timings to match the expected frame times
* Or, patch the game's chart data with your custom display timings so the game engine gets uses
  *your* "correct" frame times
  * Note: Offline patching the chart data doesn't work for several game versions as the monitor
    check does override this data, but not as flexible as required
  * Use the chart patch feature of bemanitools instead

### Micro-stuttering and non-smooth note scrolling

This is a trickier problem as it can be caused by a few different things.

* GPU related
  * Fluctuating refresh rates result in fluctuating frame times in the main game loop
  * GPU power configuration in non-performance mode (e.g. "power saving") causes processing latency
    due to sleep states
* Display related
  * Display firmware buggy and/or internal scalers that expect to run at very specific refresh rates
    and cannot adapt to any custom display timings enforced by the GPU
  * A known example is the official
    [PENDUAL LCD monitor](https://github.com/shizmob/arcade-docs/blob/6ac99975cdb2bf668362f65fa9fa3ffb2127308b/konami/product/GULDJ-JI.md#notes-and-known-issues) that does not v-sync correctly to any other refresh-rate than 60.000 hz
* CPU related
  * CPU power configuration on Windows in non-performance mode (e.g. "power saving") causes
    processing latency due to sleep states
  * Correct CPU C-states configuration in BIOS/UEFI
    * These cannot be configured in Windows and are separate from Windows's power configuration
* Other stuff running in the background of the system causing fluctuations
  * (Synchronous) Disk I/O heavy applications stealing CPU time
  * Or any other process creating a considerable amount of CPU or GPU load

## Configuration guide for different game versions

### Measuring monitor refresh rate

Let's start with configuring the system to meet the game engine's expectation regarding the
target monitor refresh rate and requiring vsync.

First, use the [d3d9-monitor-check tool](../tools/d3d9-monitor-check.md) to measure the currently
configured refresh rate of your monitor. Note that your monitor might yield different timings on
different resolutions and different game versions run on
[different rendering resolutions](../dev/journal/2025-02-09-iidx-engine.md#rendering-resolutions).

For example, the game version is 31 with the rendering resolution of 1920x1080 in FHD mode that you
want to target:

```bat
d3d9-monitor-check.exe cmdline refresh-rate-test 1920 1080 60
```

The test shows the current monitor refresh rate and the average refresh rate over the course of
the test. Check that the avg. refresh rate is stable during the measuring phase (during warm-up,
it's ok if it fluctuates).

This test allows you to determine/check:

* Refresh rate/frame time fluctuations: if you see (high) fluctuations on the first or second
  decimal place
* What's your current target refresh rate the GPU is driving your monitor at
* If vsync isn't overridden by some other driver setting as you want to see a refresh rate around
  the 60 hz mark

### Measuring vsync issues

Some displays or the GPU configuration might lead to vsync issues which can be detected with the
`d3d9-monitor-check.exe` tool as well.

```bat
d3d9-monitor-check.exe cmdline vsync-test 1920 1080 60
```

This runs a test with a VSYNC text that alternates red and cyan colors. If the text appears grey
all the time, everything's fine. If you spot brief red or cyan text either occasionally or
periodically, your current configuration has vsync issues resulting in skipped frames or old
frames displaying longer.

### Configuration

The following are recommended configurations for the different game versions. If any of these
don't work as expected, you have to troubleshoot and tweak these potentially. Please refer to the
[guidelines above](#configuration-guide-for-different-game-versions) to understand how to use
the tooling and test your changes.

#### Recommended configuration with NVIDIA GPU and driver

We want to achieve the following configuration on the GPU driver:

* Turn-off g-sync if you have a monitor that supports it
* Disable power saving mode on GPU driver
* V-sync is not forced off
* Run the display on a target refresh rate of `59.950 hz` which has been determined to be
  [the most compatible across all PC-based versions](../dev/journal/2025-02-09-iidx-engine.md#engine-step-time).

##### Turn-off g-sync

Create a new GPU profile for IIDX, if it doesn't exist yet:

```bat
nvgpu.exe profile create iidx
```

Add the application names when the profile needs to be applied:

```bat
:: For iidx versions 9 to 17
nvgpu.exe profile application-add iidx bm2dx.exe
:: For iidx versions 18+
nvgpu.exe profile application-add iidx launcher.exe
```

Disable G-SYNC for the profile:

```bat
nvgpu.exe profile gsync-disable iidx
```

##### Disable power saving mode

Set GPU power state to maximum for the profile (assumes you have a 
[GPU profile called `iidx` already created](#turn-off-g-sync)):

```bat
nvgpu.exe profile gpu-power-state-max iidx
```

##### Configure and verify target refresh rate of 59.950 hz and vsync

Get display ID of your primary display:

```bat
nvgpu.exe display primary-display-id
```

Copy the hex-value of the display ID and use it in the following commands.

Test the custom display timing first, e.g. for versions that run in 1920x1080
(Replace `<DISPLAY_ID> ` with the primary display ID you got):

```bat
nvgpu.exe display custom-resolution-test <DISPLAY_ID> 1920 1080 59.95 10
```

This will test it for 10 seconds and then revert. If the result looked good, i.e. the screen didn't
go blank or the display output was garbage, apply it:

```bat
nvgpu.exe display custom-resolution-set <DISPLAY_ID> 1920 1080 59.95
```

Test and verify the configuration is applied and works correctly:

```bat
d3d9-monitor-check.exe cmdline refresh-rate-test 1920 1080 60
```

Test if the enforced refresh rate is compatible with your display and vsync is working correctly:

```bat
d3d9-monitor-check.exe cmdline vsync-test 1920 1080 60
```

##### Alternative refresh rate configuration

If for some reason your display doesn't support 59.950 hz properly, i.e. display artifacts or vsync
issues, you can use any other 60 hz compatible refresh rate, e.g. anything between 59.900 hz and
60.100 hz that your display supports properly.

Follow the same steps [as above](#configure-and-verify-target-refresh-rate-of-59950-hz-and-vsync) 
but use a different refresh rate, e.g. 60.000 hz.

Verify using the `d3d9-monitor-check.exe` with the outlined steps if you need to play around to find
a refresh rate that works for your setup.

For the game versions 9 to 19, you have to use the chart patch feature of bemanitools to patch
the charts to your custom refresh rate. Otherwise, the game engine will run on your configured
refresh rate, but it will take the non-matching refresh rate from the chart data which causes
song de-syncing.

For example, to configure a refresh rate of 60.000 hz in the iidxhook config file:

```text
gfx.monitor_check=60.000
```

##### ATI/AMD

Currently, there is no tooling provided by bemanitools to configure the AMD GPU driver. The basic
concepts of what needs to be configured still applies as outlined in the NVIDIA sections above
though, e.g. through AMD's GPU control panel.

Further steps and instructions TBD