# Notes outlining some aspects of the DirectX calls which were required to know to implement a up-/downscaling feature
As the title says, this outlines some aspects I needed to figure out in order to implement a up-/downscaling feature
within the d3d9 hook module that works on all currently available IIDX versions (9 to 26).

To analyze the rendering loops, I have used a tool called apitrace which traces the calls of many graphic APIs:
https://github.com/apitrace/apitrace

Defintely recommended to quickly figure out what is going on regarding rendering. It also allows you to let you render
parts of a scene after the application exited because it records all API calls and data passed to them.

Anyway, considering the various iterations in (GPU) hardware the game had to undergo combined with weird quirks and
"fixes" Bemanitools is undoing, I wouldn't have guessed that their rendering engine was nearly the same until IIDX 20.
That's when they introduced SD/HD mode.

In this case, that's great news because I had to craft a solution that allows up-/downscaling the final frame to
different resolutions (see the iidxhook-util/d3d9 module for more details about the feature).

But first, we need a breakdown of the render loop's most relevant parts for this:

## IIDX pre 20
Using apitrace, we can see the following outline of a frame (not counting the first one that does a lot of setup in
the beginning):
```
BeginScene
Clear
...
// Here are the main draw calls for the scene rendering to the back buffer
EndScene
Present
```

No render target switching, simply render everything to the back buffer...plain and simple.

Note: The viewport size is determined by the size returned by GetClientRect, wtf.
Welp, no official Konmai seal of approval without that. ¯\_(ツ)_/¯

## IIDX 20+
Using apitrace, we can see the following outline of a frame (not counting the first one that does a lot of setup in
the beginning):
```
BeginScene
// tex1 is a render target texture with size 1280x720 (also in SD mode)
// The game will render to this intermediate target and not directly to the fram ebuffer
SetRenderTarget(0, tex1)
...
Clear
BeginScene -> ErrInvalidCall return code
Clear
...
// Here are the main draw calls for the scene rendering to tex1
...
// Sets the render target to the original frame buffer. The size of the framebuffer is set to the output mode
// resolution, e.g. 1280x720 for HD mode and 640x480 for SD mode
SetRenderTarget(0, frame_buffer)
...
Clear
EndScene
...
// Render, texture and sampler state updates as well as a draw call that draws tex1 to a quad which fills the
// screen space.
...
EndScene -> ErrInvalidCall return code
Present
```

This is quite a different flow to implement HD and SD mode but the solution to solve that particular problem is straight
forward and easy to understand. This means that the game will always render in HD mode and only downscale the final
frame to SD resolution for 640x480 output.

Also, why the fuck do they call BeginScene and EndScene twice? Looks like they wanted to do this in two separate scenes
for some reason. Checking the return values would have revealed to them that something's not right...lucky them that
this code works nevertheless.
Another Konmai seal of approval, a job well done. ¯\_(ツ)_/¯

## iidxhook's up-/downscaling solution
The initial solution simply hooked into BeginScene and EndScene and let the game render to an intermediate render
target texture. The texture was scaled according to the actual target frame buffer size before getting presented. This
solution worked fine for pre IIDX 20 games but created a black screen on IIDX 20+.

In order to avoid two different scaling flows, the final solution that works for both does the following:
* Create a render target texture with native resolution and let the game render to it
* Set the render target to that intermediate render target texture on BeginScene
* Before Present
    * Scale the intermediate render target texture to the back buffer
    * Set the back buffer as the render target
* Present frame

Just an outline which follows the actual implementation that you can find in the iidxhook-util/d3d9.