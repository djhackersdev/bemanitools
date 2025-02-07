# nvgpu - Command line tool to configure and tweak the NVIDIA GPU driver

An open source re-implementation of the “NvDisplayConfigLDJ" tool with additional enhancements.

The tool is based on NVIDIA's [nvapi](https://github.com/NVIDIA/nvapi) which is an interface to
various driver settings that can also be tweaked from the NVIDIA Control Panel. The goal of this
tool is to provide a streamlined command line interface to configure various settings that can
improve gameplay experience significantly.

This can be used to tweak your nvidia GPU driver settings to create custom display timings to address
IIDX’s requirement if expecting proper display timings. This can also be used for any legacy IIDX
versions that even expect very specific display timings, e.g. 59.95 or 60.05 hz.

Furthermore, creating application profiles allows further tweaks to important GPU settings such as
the current performance mode setting. This is crucial to ensure the GPU is not going into any kind of
power saving states which results in non-smooth scrolling during gameplay and micro stuttering that
cannot be measured on application level.

Simply run the tool without any arguments to get a full synopsis of available commands.

## Example usage for modern IIDX

### Custom GPU profile

The following creates a custom profile to address potential performance concerns such as not 100% smooth scrolling and
micro stuttering.

* Create a custom profile: `nvgpu profile create launcher`
* Add the launcher application to the profile: `nvgpu profile application-add launcher launcher.exe`
  * This will apply to any (IIDX) game running with `launcher.exe`
* Disable G-SYNC for the profile: `nvgpu profile gsync-disable launcher`
* Set GPU power state to maximum for the profile: `nvgpu profile gpu-power-state-max launcher`

### Custom timing

* Run the game and observe the monitor check screen (requires IIDX 20+)
* Take note of the refresh rate of the monitor that is determined by the game
* Exit the game
* Run `nvgpu display list` to get the display ID of the monitor you want to change use that ID in the following commands
* Run `nvgpu display custom-resolution-test` with your display ID and monitor settings to test the custom configuration
  first
  * For example, for IIDX 31 which runs in native 1920x1080 with a monitor also having that as its native resolution,
    having the monitor id `0x12345678` and the monitor check yielding a value of ~`59.9345`, run 
    `nvgpu display custom-resolution-test 1920 1080 59.9345 10`
  * Observe if the test is successful and the display doesn't turn blank or displays a glitched image for ~10 seconds
* Run `nvgpu display custom-resolution-set` with the previously tested settings to apply the custom display mode
  * For example, `nvgpu display custom-resolution-set 0x12345678 1920 1080 59.9345`

## Example usage for legacy IIDX

Legacy IIDX concerns any game prior to IIDX 20 that introduced the monitor check screen. The game engine was expecting
a the display/GPU/driver to perform at specific refresh rate timings in order to provide correct timing and audio
playback for the game. To this date (i.e. IIDX 31), the game engine never re-syncs audio playback during gameplay.
Therefore any flaky and incorrect timing will result in audio desynchronization either early on or throughout a song.

### Custom GPU profile

The following creates a custom profile to address potential performance concerns such as not 100% smooth scrolling and
micro stuttering.

* Create a custom profile: `nvgpu profile create bm2dx`
* Add the launcher application to the profile: `nvgpu profile application-add bm2dx bm2dx.exe`
  * This will apply to any (IIDX) game running with an executable named `bm2fx.exe`
* Disable G-SYNC for the profile: `nvgpu profile gsync-disable bm2dx`
* Set GPU power state to maximum for the profile: `nvgpu profile gpu-power-state-max bm2dx`

### Custom timing

* Use a modern game and it to observe the monitor check screen (requires IIDX 20+)
* Take note of the refresh rate of the monitor that is determined by the game
* Exit the game
* Run `nvgpu display list` to get the display ID of the monitor you want to change use that ID in the following commands
* Run `nvgpu display custom-resolution-test` with your display ID and monitor settings to test the custom configuration
  first
  * For example, for IIDX 31 which runs in native 1920x1080 with a monitor also having that as its native resolution,
    having the monitor id `0x12345678` and the monitor check yielding a value of ~`59.9345`, run 
    `nvgpu display custom-resolution-test 1920 1080 59.9345 10`
  * Observe if the test is successful and the display doesn't turn blank or displays a glitched image for ~10 seconds
* Run `nvgpu display custom-resolution-set` with the previously tested settings to apply the custom display mode
  * For example, `nvgpu display custom-resolution-set 0x12345678 1920 1080 59.9345`