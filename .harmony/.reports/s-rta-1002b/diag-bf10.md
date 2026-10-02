## BUILDER REPORT — diag-bf10 (lane diag10): MilkDrop output vs the composition canvas

STATUS: DONE
RESULT: BF10 root cause found and reproduced in the real app (main HEAD build, --test-mode). The MilkDrop source draws into the WINDOW's framebuffer (framebuffer 0, the Preview panel, sized in physical pixels), not into an offscreen buffer the size of the composition canvas, and then copies a canvas-sized rectangle out of it. Only the part of the canvas that fits inside the Preview panel gets a picture, anchored bottom-left. The rest of the canvas is transparent black. projectM's own drawing is left behind in the panel's letter-box bars, which is where the "garbage strips" come from. Diagnosis only, no fix.
FACTS: `src/sources/ProjectMSource.cpp:167-179` (glBindFramebuffer(0), glViewport(fboWidth_, fboHeight_), projectm_opengl_render_frame, glBlitFramebuffer from READ framebuffer 0, rect 0..fboWidth_ x 0..fboHeight_, into outputFBO_); `src/sources/ProjectMSource.cpp:89-105` (resize() follows the canvas: re-creates outputFBO_ and calls projectm_set_window_size(canvas), so resizing already works and is not the bug); `src/render/Renderer.cpp:821-823` (legacy source path renders the source at renderW/renderH = canvas size) and `src/render/CompositorEngine.cpp:1103` (clip path, same sourceRenderFn_ at composite size); `src/render/Renderer.cpp:1085-1112` (presentCanvas draws only inside the present rect, so whatever projectM drew into framebuffer 0 stays visible in the letter-box bars); introducing commit `c9c27c8` (2026-09-27, "the composition canvas drives the picture's shape"). Before it, `git show c9c27c8^:src/render/Renderer.cpp` line 456 has renderW = compW, the panel's physical width, so the source was rendered at exactly framebuffer 0's size and the copy was exact. libprojectM in use is 4.1.1 (`~/.local/include/projectM-4/version.h`, linked dylib "current version 4.1.1"). It exports only `projectm_opengl_render_frame`, with no `_fbo` variant (nm). Live captures are under /private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/73d4f54c-e9d5-409c-8b5d-694bfd57c171/scratchpad/diag10/run1 and run2.
METHOD: (1) Read the source. (2) Ran 2 live batches on the main app, read-only (`open -g ... --args --test-mode`, lock helper, quit_app our pid only, no Output window opened): load_source projectm_visualizer, set the canvas through set_composition_params outputWidth/outputHeight to 640x360, 1280x720, 1920x1080, 2560x1440, 3840x2160 and 1080x1920 (plus render_frame width/height locks), captured canvas frames with render_frame, and decoded the pixels with numpy (lit bounding box, alpha, frame-to-frame diff). Control: the plasma source at 1920x1080 and 2560x1440. Window-only captures (`screencapture -l <windowid>`, never full screen) of the app window show the Preview panel. (3) A standalone scratch probe linked against libprojectM 4.1.1 (offscreen CGL 4.1 core context; bind our own framebuffer object, then call projectm_opengl_render_frame) shows which framebuffer projectM leaves bound.
CONFIDENCE+VERIFY: HIGH on the root cause. Re-prove: run `scratchpad/diag10/run2.sh` then `decode.py run2/c_*.png`. Expected: at every canvas size larger than the panel, the lit box is (0, H-840) to (756, H), i.e. exactly 756x840 px from the bottom-left. 640x360 is 100 % lit because it fits inside 756x840. Outside the box every pixel is RGBA (0,0,0,0). Plasma is 100 % lit at all sizes. Look at `view_panels.png`: the bars above and below the panel's picture show raw MilkDrop imagery at all 4 sizes.
UNKNOWNS/NOT-DONE: (a) That libprojectM >= 4.2 adds `projectm_opengl_render_frame_fbo(instance, fbo)` is RECALLED, NOT VERIFIED here. No 4.2 headers exist on this machine. Check the 4.2 release / header before choosing the fix path. (The comment in `src/sources/ProjectMSource.h` already says "Calls projectm_opengl_render_frame_fbo()", but the code calls the plain function because 4.1.1 lacks it.) (b) Boris's exact panel size is not measured: his "~55 % x ~50 %" block is his own Preview panel's pixel size divided by his canvas size. On this machine it is 756x840 / 1920x1080 = 39 % x 78 %. (c) I did not drive a deck clip with MilkDrop through the UI. The clip path goes through the same `ProjectMSource::render` at the composite size (`src/render/CompositorEngine.cpp:1103`), so I infer it is identical. (d) The probe's final-composite target is inferred from "binding reset to 0 + our framebuffer object stayed empty". A core-profile context has no pbuffer, so framebuffer 0 could not be read back in the probe.
NUANCE: The "black" outside the block is TRANSPARENT (alpha 0), not opaque. So a MilkDrop clip on a layer probably lets the layers beneath show through outside the block (inferred, not captured). This is separate from the opaque black first frame after any canvas resize: render_frame with a width/height override on MilkDrop returns a fully opaque black frame (lock_new_640x360: alpha 255), because projectM's first frame after projectm_set_window_size is black. That is a minor second symptom (a one-frame black flash per canvas resize) and also a test-harness trap: capture MilkDrop with set_composition_params plus a wait, never with a render_frame size override. Outputs, recorder, Syphon and snapshots all read the canvas (`src/render/Renderer.cpp:1117-1122` publishToOutputs), so they carry the same cropped block (inferred from code). Harness note: the relayed user request (decks are boxes of clips, BF9) is a different topic from this computed task (BF10). They do not conflict. BF9 is tracked as bf9b in `.harmony/boris-feedback-backlog.md`.
HANDOFF-NEEDS: none

### SUMMARY
`ProjectMSource::render` renders projectM into framebuffer 0, which is the Preview panel's framebuffer (756x840 px on this machine). It then copies a canvas-sized rectangle out of it. Since c9c27c8 (2026-09-27) the canvas is the composition size instead of the panel size, so only min(panel, canvas) pixels hold a picture, anchored bottom-left. The rest is transparent black. projectM's raw drawing also stays in the panel's letter-box bars.

### ROOT CAUSE (with the proof)
1. **Where it draws** (`src/sources/ProjectMSource.cpp:167-179`). The code does `glBindFramebuffer(GL_FRAMEBUFFER, 0)` and sets `glViewport(0,0,canvasW,canvasH)`. It calls `projectm_opengl_render_frame`, then blits from READ framebuffer 0 over (0,0)-(canvasW,canvasH) into `outputFBO_`. Framebuffer 0 is the JUCE window drawable, which is the Preview panel at its physical size (`glContext_.attachTo(previewPanel)`, `src/render/Renderer.cpp:54-60`). It is never the canvas.
2. **Why it broke "when output became composition size."** Before `c9c27c8`, sources were rendered at renderW = compW, the panel's physical size (c9c27c8^ Renderer.cpp:456). The source size equalled framebuffer 0's size, so the blit was exact. After it, sources render at the canvas size (`src/render/Renderer.cpp:821-823`, CompositorEngine:1103), which is larger than framebuffer 0. Pixels outside framebuffer 0 do not exist, so the blit leaves them undefined. Here they read as RGBA 0,0,0,0.
3. **Live proof (main app, test mode), decoded canvas captures:**

| canvas | lit bounding box (PNG coords) | lit % | outside the box |
|---|---|---|---|
| 640x360 | (0,0)-(640,360) | 100 % | n/a, it fits inside 756x840 |
| 1280x720 | (0,0)-(756,720) | 46.8 % | RGBA 0 |
| 1920x1080 | (0,240)-(756,1080) | 30.3 % | RGBA 0 (max value 0) |
| 2560x1440 | (0,600)-(756,1440) | 12.4 % | RGBA 0 |
| 3840x2160 | (0,1320)-(756,2160) | 1.4-3.8 % | RGBA 0 |
| 1080x1920 | (0,1080)-(756,1920) | 30.5 % | RGBA 0 |
| control plasma 1920x1080 / 2560x1440 | full | 100 % | n/a |

   The covered box is always 756 x 840 = min(panel, canvas), bottom-left. That is the Preview panel's physical size at 2x scaling (the main window is 1728x1079 pt on a 2.0 backing screen). Inside the box the picture is live: 99 % of pixels change between two captures 0.5 s apart.
4. **The garbage strips** (window captures `run2/win_*.png`, cropped side by side into `view_panels.png`). `presentCanvas` draws the canvas only inside the letter-box rect (`src/render/Renderer.cpp:1085-1112`). projectM's own drawing into framebuffer 0 (a viewport the size of the canvas, clipped to the panel) stays in the bars above and below. This happens at every canvas size, even 640x360 where the canvas itself is correct. The 1920x1080 panel image reproduces Boris's screenshot: a picture block at the bottom-left, black to its right and above it, and MilkDrop imagery in the top and bottom bars.
5. **Why "just bind our framebuffer object" is not enough on 4.1.1** (scratch probe `pmprobe/probe.cpp`). With our own framebuffer object (FBO 1, 320x180, complete) bound and cleared to black before each call, after 90 `projectm_opengl_render_frame` calls the DRAW binding is 0 and our framebuffer object has 0 / 57600 lit pixels. libprojectM 4.1.1 rebinds framebuffer 0 itself and never draws into the caller's framebuffer object. Its only render export is `_projectm_opengl_render_frame` (nm).

### WHAT THE FIX MUST CHANGE
- Make projectM render INTO a framebuffer the size of the canvas (`outputFBO_`), never framebuffer 0. That also removes the bar garbage, because nothing is drawn into the window framebuffer any more. It is not a viewport, texture-coordinate or resize fix: `projectm_set_window_size` already follows the canvas (`src/sources/ProjectMSource.cpp:89-105`).
- With libprojectM 4.1.1 that is impossible through the public API. Options: (a) upgrade libprojectM to the release that provides `projectm_opengl_render_frame_fbo(pm, outputFBO_)` (believed to be 4.2, RECALLED, verify first) and drop the window-framebuffer blit; or (b) patch the locally built projectM (`~/.local`) so it renders to the caller's framebuffer. Either way, re-run run2.sh: the expected result is 100 % lit at every size and no MilkDrop imagery in the panel bars.
- Optional follow-on: the opaque-black first frame after a canvas resize (projectM's). If the fix wants no flash, keep the old frame until projectM's first real frame at the new size.
- Same-pattern scan: `ProjectMSource.cpp:175` is the only `GL_READ_FRAMEBUFFER, 0` read in `src/`. Every other source renders into its own framebuffer object (the plasma control is full at every size). No other source or effect has this bug.

### FILES CHANGED
- none in the repo. This report is the only repo file written. Scratch only: scratchpad/diag10/{run1.sh, run2.sh, decode.py, pmprobe/probe.cpp, pmprobe/probe2.cpp, run1/, run2/, view_*.png}.

### TESTS
- n/a (diagnosis). Live batches: run1 (14:47) and run2 (14:49), both clean. App quit, lock released, 0 Output-named windows, 0 UserNotificationCenter windows 16 s after each quit.

### SLIM CHECK
nothing to cut — no code was shipped.

### PACKET QUALITY
- Clarity: CLEAR
- Missing context: the projectM library version and the fact that 4.1.1 has no render-to-framebuffer-object call. Found via nm and version.h.
- Unused context: the scratch-worktree instrumented build (not needed; a 40-line standalone probe answered the library question).
- Self-brief files: CLAUDE.md (Pitfalls 37/47/52) useful; boris-feedback-backlog.md BF10 useful.

INBOX-RECHECK: none

### STATUS
DONE

### NEXT ACTION
Harmony: put BF10 on the fix list with fix path (a) after verifying the 4.2 API. Suggested notebook line: "ProjectMSource renders via framebuffer 0 (the panel): sizes above the panel are cropped. Capture MilkDrop with set_composition_params + wait, never a render_frame size override (first frame after resize is black)."
