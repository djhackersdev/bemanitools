this document outlines the important parts of the rendering pipeline to provide SD/HD modes in iidx20+ (iidx24 used as a basis for this analysis)

- a texture is created and marked as render target -> 1280x720
this is the texture the game renders the final frame to. all rendering is always done in 1280x720 by the engine.
- the actual framebuffer is set to the target resolution: HD 1280x720 and SD 640x480
- the final frame is then re-drawn to the framebuffer with the target resolution