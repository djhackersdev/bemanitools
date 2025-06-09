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

## Example usage for IIDX

Refer to the dedicated [iidx-syncbook](../iidxhook/iidx-syncbook.md) documentation for details on
how to use `nvgpu` to configure your system to run IIDX versions with proper display timings, smooth
frame rates and sync game-play.