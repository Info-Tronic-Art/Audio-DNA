# CHECK of answer-252.md (item 252, BF267): blind re-check

Written by an adversarial checker seat, stamp: see last line. Main working tree, HEAD 33989a8. Read-only: nothing built, run, probed or launched; Audio-DNA and Resolume Arena were not touched. Everything about the app below is "read, not run". Paths are under /Users/boriskarpman/projects/RealTimeAudio. CT = .harmony/.reports/s-rta-1005/facts-app-cue-today.md.

VERDICT: SOUND_WITH_CORRECTIONS. The core design reading (one thread, one canvas, shared layer pictures, the preview must not run the compositor twice) holds against the code. Two statements that Harmony would rely on are wrong or unproven (F1, F2), and the "ANSWER FOR BORIS" says more than the paper proves (F1, F3, F5).

## MY OWN VIEW (formed before opening the paper)

Same frame, same thread? Output panel and any preview built inside Renderer::renderOpenGL: YES, by construction.
- Panel picture: Renderer::attachTo makes the panel's GL host a continuously repainting context (src/render/Renderer.cpp:53-60, setContinuousRepainting(true) at :57). Each frame composites the show ONCE offscreen into canvasFBO_ (:449-468, compositeShow call ~:692-695, global effects :714), then publishToOutputs (:819) and presentCanvas (:823). The panel only presents (CLAUDE.md Pitfall 37; docs/claude/rendering.md:158).
- Output screens: separate OutputWindow contexts (src/ui/OutputWindow.cpp:67-71), swap interval 0 (:15), continuous repaint "display-link paced" (:69). Each presents the NEWEST shared frame once per refresh of its OWN display (:10-27; rendering.md:160), one frame after it was drawn.
- Threads: JUCE renders all contexts on ONE shared thread "OpenGL Renderer": `SharedResourcePointer<RenderThread>` (build/_deps/juce-src/modules/juce_opengl/opengl/juce_OpenGLContext.cpp:966, class :746; thread body :889-892; renderAll loops over all contexts :791-812). On macOS a frame is only started when the display link of the screen the context's window is on sets pending (:322-380 `noAutomaticRepaint = true` on JUCE_MAC; :895-910 link; :1023 lastDisplay).
- Can one run at another rate? The panel and a preview drawn in the same function: no, not unless made to. An output screen: YES. It makes new pictures at most as fast as the main window's display link, but it presents at its own display's refresh. If its screen refreshes faster, it repeats frames; slower, it skips. It is also one frame behind the panel. The shared thread also means one slow main frame delays the output presents (OutputWindow.cpp:12-14 says the same). Not run.
- What does a second mix of the same layers cost? Done by calling compositeShow again: wrong, not just slow. It advances every fade (CompositorEngine.cpp:~1089-1091), video playheads by dt (Renderer.cpp:1614-1676 via ClipTransportSync.h), steps simulations, and reads/writes the same per-layer trail memory (CT:61-67). Done from the pictures the compositor already keeps (saveLayerOutput, CompositorEngine.cpp:1154, fn :236-262; taken after clip effects, transition, feedback, layer effects, transform and BEFORE keying/opacity/blend: keying :1160, blend :1162) it is two fixed-function passes per layer (blend modes are plain glBlendFunc/min/max, :1343-1400) plus one extra canvas-size target: cheap, an ESTIMATE. A clip that is on no layer has no draw path at all (CT:62) and its model fields carry its play state (ClipTransportSync.h:30-38 reads `clip.playing` as intent): that is where the real cost and the real risk sit.
- Hidden fact for "the output comes first": the output screens are fed by the panel's context. When the preview panel is hidden (signal bar expanded, src/MainComponent.cpp:2723) the main GL context detaches and the outputs freeze on the last frame (src/output/SharedFrameSet.h:7-11; Renderer.cpp:496-503 comment). Today the output depends on the preview panel being alive.

## CITATIONS CHECKED

(CONFIRMED unless stated; "~" = within 2-3 lines.)
1. juce_OpenGLContext.cpp:890-893, one thread: CONFIRMED (~; thread name :890, loop :891; shared via SharedResourcePointer :966).
2. juce_OpenGLContext.cpp:660 swap interval 1: CONFIRMED.
3. juce_OpenGLContext.cpp:895-905 display link: CONFIRMED (~; function runs :895-910, per-screen via lastDisplay).
4. CMakeLists.txt:46 JUCE 8.0.4: CONFIRMED.
5. src/output/SharedFrameSet.h:15 "ONE shared thread": CONFIRMED. SharedFrameSet.h:6-10 no GL object shared: CONFIRMED.
6. src/ui/OutputWindow.cpp:12-14, :10-27, :15, :69: CONFIRMED.
7. Renderer.cpp:57 continuous repaint: CONFIRMED. :296 renderOpenGL: CONFIRMED.
8. Renderer.cpp:449-468 canvas; :819 publishToOutputs; :823 presentCanvas; :714 global effects: CONFIRMED.
9. Renderer.cpp:694-696 compositeShow: WRONG by two lines: the call is at :692-695 (cosmetic).
10. Renderer.cpp:651-667 realDt clamp 0.25 s: CONFIRMED (~; code :645-669).
11. Renderer.cpp:322-329 video upload budget per frame: CONFIRMED (:316-330).
12. Renderer.cpp:839-858 adaptive quality (30 frames over 12 ms disables last enabled effect of effectChain_): CONFIRMED. Renderer.h:553-556 and :555 (12 ms): CONFIRMED (:555 kFrameTimeBudgetMs = 12.0f, :556 threshold 30).
13. Renderer.cpp:826-858 "frame timer": CONFIRMED but it is CPU submit time only (see F2): comment at :826-828, renderEnd :829, no glFinish.
14. Renderer.cpp:1306-1318 source params written each draw: CONFIRMED (~, :1307-1320).
15. CompositorEngine.cpp:1150-1154 saveLayerOutput after renderLayerStages: CONFIRMED (:1150 stages, :1154 save); :236-262 function: CONFIRMED.
16. CompositorEngine.cpp:1158-1163 keying/blend: CONFIRMED (:1160, :1162). :1331 u_opacity = layer.eff(Opacity): CONFIRMED.
17. CompositorEngine.cpp:1078-1080 gate (visible/bypassed/solo only; fader 0 still drawn): CONFIRMED (:1079-1080).
18. CompositorEngine.cpp:1067 and :107-139 resize rebuild: CONFIRMED.
19. CompositorEngine.cpp:1283-1286 global effects key: CONFIRMED. LayerStateKey.h:29-40, :38: CONFIRMED.
20. docs/claude/rendering.md:158, :160: CONFIRMED. rendering.md:77 (upload budget): CONFIRMED.
21. docs/claude/pitfalls.md:45 "Echo decay is described as a per-frame factor": WRONG as worded. Line 45 (Pitfall 18) is about remapping the decay range and says nothing about per frame. The claim itself is TRUE from the shader: src/render/EmbeddedShaders.h:~8445-8485 Echo does `trail = prev * decay` with decay = mix(0.82, 0.995, slider), once per drawn frame (so trails are frame-rate dependent; the paper said "the shader itself was not opened").
22. docs/claude/pitfalls.md:49 (Pitfall 20, 237 MiB): CONFIRMED.
23. docs/claude/milkdrop.md:88, :145, :195, :224: CONFIRMED (one engine; first-load hitch UNKNOWN).
24. src/api/ApiServer.cpp:1442-1490 (frame_time_ms, peak_frame_time_ms, peak_callback_ms, gpu_time_ms, peak_gpu_time_ms): CONFIRMED.
25. CT:15, 22, 55, 59, 60, 61, 62, 64, 66, 67: CONFIRMED against CT (CT:104-111 corrections do not touch them). AC:122 (T1): CONFIRMED.
26. src/MainComponent.cpp / Pitfall 40 "no GL object shared": not cited by the paper for hide; see F3.
Count opened: 33 distinct cited locations (plus the 5 extra I opened for my own view). WRONG: 9 (cosmetic), 21 (wording, claim true).

## FINDINGS

F1. MUST | "The output loses nothing first" / "one hard guarantee" is false to the paper's own way F. The give-way rule fires after frames have already run late for a sustained time (way F; the existing counter needs 30 frames over 12 ms, about half a second: Renderer.h:555-556). During that window the output stutters too, on the shared thread (OutputWindow.cpp:12-14), and the output's trails (per-frame decay, EmbeddedShaders.h ~8464) change with it. What the design really guarantees: the preview cannot alter the output's STATE, and the preview stops its own extra work after a delay. It does not guarantee the output never loses a frame first. | Fix: ANSWER FOR BORIS and RECOMMENDATION say "the preview stops its own extra work as soon as it sees trouble, so the audience may see a short hitch (about half a second, ESTIMATE from the existing 30-frame counter) before the preview pauses" or drop the word "first". Do not call it a guarantee until the test has run.

F2. MUST | Clip previewed alone: "previewing never touches the output's state" (recommendation point 2) cannot hold as written, and the Answer's "does not lie" does not cover it. Boris asked for exactly this ("when a user clicks on the bottom of a clip ... it is displayed in the preview monitor", CT:10). The paper's shared-pixels trick (way E) covers only layers the compositor draws anyway; a clip on no layer has no draw path (CT:62; AC T3). Three read-from-code hazards the paper does not name: (a) a video's play intent and playhead live in the shared Clip model: Renderer::syncMedia pushes `clip.playing` (a clip that is not fired is `false`, so the player is told to STOP) and writes the playhead and play state back into the model (ClipTransportSync.h:30-38, 46-56; Renderer.cpp:1646-1675), one player per clip id; so a preview either shows a frozen frame or writes the model the output reads, and if the clip is also on a layer it is advanced twice (CT:67). (b) A video's player is parked until drawn and holds while it catches up (CT:67), so the preview starts late, not live. (c) The clip is previewed raw or in the layer's context (layer effects, blend)? Not stated; the paper's "does not lie" is silent on this. | Fix: say that clip-alone preview needs its own player/clip state (a preview copy, not the model's clip), or is limited (still frame / not for a clip that is playing), and move it out of the "true all the time" claim. Keep acceptance test 2, but add this case to it.

F3. SHOULD | The Answer's "one part of the app draws the output and the preview together, each moment once, so they cannot drift apart" is true for the panel and the preview monitor, not for the output screens. Output screens present the newest finished frame once per refresh of their OWN display, one frame later, swap interval 0 (OutputWindow.cpp:15, 69; rendering.md:160). On a different-rate display (or a projector at another rate) the number of new pictures per second differs, and the main swap on the shared thread could delay them (juce :791-812, :1052 comment; UNKNOWN, not run). The paper's section 1 says part of this; the Answer drops it. | Fix: "the output panel and the preview are the same frame; each output screen shows the same picture, about one frame later, at its own screen's speed". Unmeasured; his display setup is UNKNOWN (paper test 5 covers only the window's screen).

F4. SHOULD | The give-way trigger is described as cheap ("the frame timer and a sustained-count already exist"). That timer measures CPU submission time only, with no glFinish (Renderer.cpp:826-829), taken before the swap and before the GPU finishes. A GPU-bound overload (heavy shaders, the likely one here) can pass it. gpu_time_ms exists but is an async EMA read one to two frames late and skipped when its query is in flight (Renderer.cpp:~975-1008). | Fix: the build needs a trigger on GPU time or on a missed refresh (display-link lateness), not frameMs; mark the effort "medium, unknown" not "small".

F5. SHOULD | "Output comes first" is today the other way round, and the paper never says it. The output screens are fed from the panel's context: hide the panel and the main context detaches, the show stops advancing and the outputs freeze (SharedFrameSet.h:7-11; MainComponent.cpp:2723; Renderer.cpp:496-503). The recommended "one GL host, two rectangles" puts the preview monitor in the same fate and means the host must span both monitors with the layer picker/gear row between or below them, while the GL host paints no JUCE children (setComponentPaintingEnabled(false), Renderer.cpp:58; PreviewPanel.cpp "tab bar stays above the GL surface"). | Fix: add a line "the output is fed by the panel's drawing, so the panel must stay on screen while the output is live"; say what the layout needs (controls outside the host, or two hosts) before Boris is told the monitors are one surface.

F6. SHOULD | "A layer you cue is shown from the picture the output already made, trails included" is labelled INFERRED in the paper but stated flat in the Answer. It also holds only for media layers: effects-only layers, Mask layers, master cue and a bypassed cued layer need their own run (paper's table), and the saved picture is wiped on any canvas-size change (CompositorEngine.cpp:107-139) and held when an image is still decoding (:1118-1130). Not run; the design is not built. | Fix: "I expect"; name the exceptions in one clause.

F7. SHOULD | Pitfall citation (citation 21) and compositeShow line (citation 9): fix the cites. Also "reuse the compositor's working targets" (recommendation 5) is true for effect targets A/B/C only; the accumulator and scratch are busy with the output's mix in the same loop, so the cue mix needs its own accumulator and scratch (two extra canvas-size targets; small, but they are rebuilt with the canvas).

F8. SHOULD | The last-resort paragraph: "starts true, lowered on evidence, reversible" is a fair argument, but the Answer says "I would build it no other way" while the paper's own UNKNOWN list (tests 1-4) is not run and the cost of the hardest case (a clip alone) is not estimated at all. | Fix: soften to "this is what I would build first, and I would measure before ruling the size".

## THE STRONGEST OTHER DESIGN

H. Shared pixels for what the output draws, a deliberately limited second path for what it does not, rather than "one design for everything":
1. Cue of layers that play = way E (as the paper recommends).
2. Clip-by-name preview in the first build = a still (frame 0 / thumbnail path, no live playhead), or live only when no layer shows that clip and with a preview-owned player; sources by type only when the type is not live; no MilkDrop by name while MilkDrop plays.
Reason this beats the single recommended design: the paper's cheap, exact, state-free part (E) is separable from its expensive, state-coupled part (clip alone, F2); bundling them under "one design, true all the time" hides that the second part has no draw path (CT:62), shares model fields with the output, and cannot meet acceptance test 2 without new player state. Does the paper's reason against it hold? The paper rejects B (always smaller) and C (preview at half rate) well: C is real (trail memory in the preview-only chains runs per drawn frame: Echo `prev * decay` EmbeddedShaders.h ~8464, so a half-rate chain holds trails about twice as long in real time; reasoning sound), and B does lie for pixel-reading effects (paper counted 256 lines that read the size; I did not recount; UNKNOWN which look different). The paper does not weigh H. Its reason against a small preview ("a permanent small untruth") does not apply to H1, only to H2, where H gives a plainly marked still.

Could the recommended design make the preview LIE about a layer on the output (his fear "playing something smaller or less smooth may create problems")?
- Cued playing layers (E): no, by the code read: the picture is the output's own. Two caveats: it is the layer BEFORE the layers below it and global effects, so it never equals the output's final look with blend and master, which is inherent to a cue; and the preview-only chains (master cue, effects-only layer) start with empty trail memory (paper says so).
- Clip alone: it can lie in time (late start, parked player), in state (F2) and in context (raw vs in the layer's effects/blend). Not named in the paper.
- Smaller or less smooth: the paper is right that these are not needed at first; "less smooth" cannot happen by itself, because all three share one frame rate (but see F3 for the output screens).
- Overload: the preview shows a marked still (not a wrong moving picture); but only after F1's delay.

## A BETTER ANSWER FOR BORIS

Yes, the output panel and the preview run at the same speed, because the app draws them in one go. The output screens show that same picture, about one frame later, at their own screen's speed. "Comes first" means: looking at a preview must never change what the audience sees, and if the computer struggles, the preview pauses (marked) while the output carries on, after a short hitch. A layer you cue should look like it does on the output; a clip you click by name is harder and may be a still at first. None of this is measured; a short test on your show comes before anything is built.

(111 words)

Stamp: 2026-10-09 19:06:19
