# D3D9 Monitor Check

A separate application to run the infamous IIDX “monitor check” without having to run the actual
game. The tool can be used to test measure the current avg. monitor refresh rate or debug/check if
that value is fluctuating for some reason.

The final avg. value that is provided at the end of the test can be used as input for other tooling
or settings (e.g. patching charts to a different refresh rate on older games with bemanitools).

Simply run the tool without any arguments to get a full synopsis with usage instructions.

## "Accuracy" remarks

The tool has been tested on an actual cabinet with `nvgpu` setting different custom timings. The
accuracy seems to be even higher than what IIDX’s monitor check is actually showing. For example,
with a custom timing of 59.900 hz, this tool yields fairly accurate and stable avg. 59.902 hz.

The monitor check of IIDX 29 shows results of 59.8981 hz to 59.8997 hz on screen. As these are the
only visible values to the user, determining a specific (avg.) value that can be used as input for
other tooling or settings (e.g. patching charts to a different refresh rate on older games with
bemanitools) is difficult. This doesn't mean that the game's monitor checks are actually
inaccurate or wrong. Modern games with a built-in monitor check (starting IIDX 20) are syncing up
fine and don't need any further patching or modifications.

For older games, picking a value that is not as close as possible to an accurate avg. value can
easily lead to issues with sync. So it's recommended to use the d3d9-monitor-check tool to get the
most accurate value.