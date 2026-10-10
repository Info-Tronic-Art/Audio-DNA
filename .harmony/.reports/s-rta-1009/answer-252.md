# ANSWER 252 -- "the output comes first": what decides the frame rate of the three pictures, and how the output is protected

Written 2026-10-09 18:55:29 by the Architect seat (s-rta-1009). Main working tree, HEAD 33989a8.
Read-only: nothing built, run, probed or launched; Audio-DNA and Resolume Arena were not touched. NOTHING HERE IS MEASURED.
Labels: VERIFIED = read in the program text or a document, at the line named ("read, not run"). INFERRED = derived from the lines named. ESTIMATE = a judgement, no number behind it. UNKNOWN = settled only by a test.
Paths are under /Users/boriskarpman/projects/RealTimeAudio. CT = .harmony/.reports/s-rta-1005/facts-app-cue-today.md (line numbers of that file). AC = .harmony/.reports/s-rta-1005/area-cue-layers.md.

His words (BF267, block "@@BOX 252" of .harmony/.reports/s-rta-1009/answers-by-item.md): "What do you mean by come first? Playing something smaller or less smooth may create problems. What do you think? You understand how the system needs to be built better than me. Of course the output is more important, but the output preview screen which we have and the preview-cure screen below it should all run at the same fps, no?"

VERDICT IN ONE PARAGRAPH. He is right: all three pictures should run at the same rate, and in this app they do so by construction, because one thread makes every picture in one frame. A preview at its own, lower rate is possible but is the worse design: it does not protect the output and it makes trails in the preview wrong. "The output comes first" should be given one exact meaning: (1) previewing can never change what the output shows; (2) inside each frame the output's work and its share of the video budget are served first; (3) if the computer cannot keep up, the preview's own extra work stops and the preview holds a marked still, before the output loses anything. "Smaller" and "less smooth" are dropped from the assumption as things that happen by themselves; "trails a little differently" is dropped for cued layers (they can be shown from the output's own pixels) and replaced by two narrow, plainly named limits.

## WHAT DECIDES THE FRAME RATE

### 1. Who draws what, and when

- VERIFIED - One thread draws everything. All of JUCE's GL contexts in the app render on ONE shared thread named "OpenGL Renderer" (build/_deps/juce-src/modules/juce_opengl/opengl/juce_OpenGLContext.cpp:890-893, the loop `while (flags.waitForWork (renderAll() ...))`; said again by the app's authors at src/output/SharedFrameSet.h:15 and src/ui/OutputWindow.cpp:12-14). JUCE 8.0.4 (CMakeLists.txt:46).
- VERIFIED - The composition is drawn ONCE per frame, offscreen, at the composition's size, in Renderer::renderOpenGL (src/render/Renderer.cpp:296; canvas at 449-468; the one compositeShow call at 694-696; global effects 714; CLAUDE.md "Render Thread" and Pitfall 37).
- VERIFIED - Picture (2), the output panel in the window, is that canvas drawn letter-boxed into the window by presentCanvas, the last pass of the same frame (Renderer.cpp:823; docs/claude/rendering.md:158).
- VERIFIED - Picture (1), each output screen, is the SAME canvas: publishToOutputs copies it once per frame into shared surfaces (Renderer.cpp:819; rendering.md:160), and each output window copies the newest finished one to its screen once per refresh of its own display, never waiting (src/ui/OutputWindow.cpp:10-27, swap interval 0 at :15; display-link paced at :69). A surface is offered one frame after it was drawn (rendering.md:160 "+1 frame of latency").
- INFERRED - What sets the pace of NEW pictures: the main context repaints continuously (Renderer.cpp:57) with JUCE's default swap interval 1 (juce_OpenGLContext.cpp:660; the only setSwapInterval call in src is the output windows' at OutputWindow.cpp:15), and on macOS each repaint is triggered by the display link of the screen the context's component is on (juce_OpenGLContext.cpp:895-905). So the composition gets a new picture once per refresh of the screen THE APP'S WINDOW is on, provided the frame's work fits in that time. An output screen cannot show new pictures faster than that, whatever its own refresh rate. Not run.
- VERIFIED - Motion is tied to real time, not to the frame count: the frame's real elapsed time (clamped to 0.25 s) drives video playheads and source clocks (Renderer.cpp:651-667). INFERRED: when frames come late, videos keep their speed and lose smoothness; nothing slows down like a tape.
- VERIFIED - Picture (3), the preview monitor, does not exist yet. No second picture of any kind is drawn in the main window today; the panel's "Preview" and "Output" buttons only change colour (CT:15, CT:22, CT:59).

### 2. The three pictures under the plan: same frame, same thread, same rate

- INFERRED - Any preview monitor built in this app is drawn by the same thread. If it is drawn inside Renderer::renderOpenGL into a second target and presented next to the canvas in the same window surface (one GL host covering both monitors, two rectangles), then the output panel and the preview monitor are finished by the same frame and appear with the same swap: the same rate BY CONSTRUCTION. Nothing has to be built to make them equal; something would have to be built to make them differ.
- INFERRED - A second GL context for the preview (the way output windows are made) is the harder road: no GL object may be shared between contexts (Pitfall 40; SharedFrameSet.h:6-10), so the preview would travel through shared surfaces and arrive one frame late, and it would still be drawn by the same thread. It buys nothing here.
- Can the preview run at a different rate than the output? Technically yes: redraw its target every second frame and present the kept picture in between. It is HARDER to build correctly than one rate, and it does not do the job:
  - INFERRED - It does not protect the output. The frames that carry the preview are as heavy as before. A frame that misses its refresh delays everything on the one thread, the output windows included (OutputWindow.cpp:12-14). The output would stutter in a regular rhythm instead of evenly.
  - INFERRED - It makes the preview lie about trails. Trail length is set per drawn frame (Echo decay is described as a per-frame factor: docs/claude/pitfalls.md:45, Pitfall 18; the shader itself was not opened), so a chain that runs on half of the frames holds its trails about twice as long as the same effect will on the output.
  - INFERRED - Every per-frame bookkeeping line would need a second cadence: video uploads are budgeted per render frame (Renderer.cpp:322-329; rendering.md:77), a simulation steps once per draw (CT:66).
- So: one rate for all three is both what he asks for and the simplest thing to build.

### 3. What a cued mix costs on top of the output's frame

The fact sheet's warning stands: the compositor cannot simply be run twice. A second run in the same frame would advance every fade twice, advance video twice, step simulations twice, and read and overwrite the same trail memory (CT:59-67; AC T1 at AC:122). The cost question therefore depends on HOW the preview is made. There is a cheap and exact way for cued layers, read from the code:

- VERIFIED - Every frame, for every Opaque or Transparent layer it draws, the compositor already keeps that layer's finished picture: after the clip's effects, the transition, the layer's feedback, the layer's effects and the layer transform, and BEFORE keying, the layer's transparency and the blend (src/render/CompositorEngine.cpp:1150-1154; the copy itself 236-262). Its only reader today is the Layer Router source (CT:60).
- INFERRED - A cued mix can be built from those kept pictures: for each cued layer one keying pass and one blend pass (the same two steps the output uses, CompositorEngine.cpp:1158-1163, with the cue slider in place of the layer's own transparency at :1331) into a second mixing target. The layer is not drawn again: no second decode, no second run of its effects, no second step of anything.

| Case | Extra work in the frame | Label |
|---|---|---|
| A cued layer that the output is drawing anyway (any transparency, even 0: a layer at 0 is still fully drawn, CompositorEngine.cpp:1078-1080, CT:55) | two light full-picture passes per cued layer (key, blend) | INFERRED; "close to free" is an ESTIMATE |
| A cued layer the output skips today: bypassed, or silenced by another layer's solo (CompositorEngine.cpp:1078-1080) | that whole layer: decode, clip effects, layer effects, once. It is the price of the rule "a bypassed layer plays on out of sight" (R149 c), paid once, never twice | INFERRED |
| A cued layer that holds only effects (it acts on the cued layers under it, R149 e) | its effects run a second time, on the preview's mix, with a trail memory of their own | INFERRED |
| Master cue on (old item 177) | the whole global effects stack a second time, on the preview's mix, with a trail memory of its own (today one key, LayerStateKey.h:38; CompositorEngine.cpp:1283-1286) | INFERRED; the heaviest cue-mode case |
| Preview mode: a clip shown alone by its name, or a file | one whole extra clip: its decode thread, its uploads from the shared per-frame video budget (Pitfall 60), its effects with a trail memory of their own | INFERRED; the heaviest case overall |
| Presenting the second picture in the window | one more small pass like presentCanvas (Renderer.cpp:823) | INFERRED |

- Free, or close to it: cue mode with ordinary layers that are playing, master cue off. This is the DJ case he described (layers' own sliders down, cue on, look, bring them up).
- Could push the frame over: master cue on with a heavy global stack; a previewed clip with heavy effects or a large video on top of a full show; a cued bypassed layer with heavy effects; a MilkDrop preset loaded for the preview (docs/claude/milkdrop.md:224 lists the first-load shader-compile hitch on a preset change as not measured). Over what: the project's own figures are 16.67 ms per frame at 60 pictures a second with "<8ms for full chain" (CLAUDE.md "Render Thread"), and a built-in alarm at 12 ms (Renderer.h:555).
- VERIFIED, and it matters here - the app already has one automatic reaction to sustained slow frames, and it acts on the OUTPUT: after 30 frames in a row over 12 ms it switches off the last enabled effect of the legacy effect chain and logs it (Renderer.cpp:839-858; Renderer.h:553-556). If the preview's work is timed inside that window, a heavy preview could trip a change of the output's picture. The build must keep the preview's time out of that trigger (or that reaction is ruled on separately). It is the opposite of "the output comes first".

## WAYS TO PROTECT THE OUTPUT

"Lies" = the preview shows something the layer will not look like on the output.

| Way | What he would SEE in the preview | Cost to build | Does it lie? |
|---|---|---|---|
| A. Nothing is protected | Normal times: perfect. Overload: output and preview stutter together | none | No. But the audience pays for the preview |
| B. Preview's own passes at a smaller size (fixed, chosen once) | A slightly softer picture; motion identical | small: one scale number on the preview's targets; but the preview can then no longer borrow the compositor's working targets, which are rebuilt when the size changes (CompositorEngine.cpp:1067, 107-139; CT:61), so it needs its own set | A little: effects that work in pixels look coarser at a smaller size (a grep of EmbeddedShaders.h finds about 256 lines that read the picture's size; which effects change visibly is UNKNOWN). Cued layers shown from the output's own pixels are not affected |
| C. Preview skips frames, output keeps its rate | Jerky preview | medium: a second cadence for every per-frame rule | Yes for trails (about twice as long), and it does not lower the heaviest frame, so it does not protect (section 2) |
| D. Preview leaves out effects that hold picture history | No trails in the preview | small | Yes, plainly: the thing he previews is not the thing he will get. This is the "problem" he names. Rejected |
| E. Shared pixels: cued layers are shown from the pictures the output already made | Exactly the output's layer, trails included, at the same moment | medium: the cue mixing target, the layer loop drawing a layer when it is "on the output OR cued" and blending it only where it belongs | No. It removes most of the cost instead of hiding it |
| F. The preview gives way: when frames run late for a sustained time, the preview's OWN extra work stops; the monitor holds its last picture under a plain mark ("preview paused") until the load is gone | A still with a mark, briefly; never a wrong moving picture | small: the frame timer and a sustained-count already exist (Renderer.cpp:826-858); it needs a rule for coming back that does not flicker on and off | No: a marked still claims nothing |
| G. Output served first inside the frame | nothing visible normally; under video pressure the previewed video may hold a frame | small: order of work, and the previewed clip takes video uploads only after the output's clips (budget at Renderer.cpp:322-329) | No (a held frame is the same hold the output's own rule uses, Pitfall 56) |

## TRAILS AND MILKDROP

Why item 252 said "a little differently". It was written from old D9 and assumption B-11, under the fact sheet's finding that trail memory is ONE per layer and chain (CT:64; Pitfalls 13, 14, 35), that a simulation steps once per draw and that there is ONE MilkDrop engine (CT:66; Pitfall 66; docs/claude/milkdrop.md:145, 195). If the preview re-draws a layer, it either shares the output's trail memory (and corrupts it) or keeps a second one (and its trail differs). Hence "differently".

Whether it still has to be so. Mostly not:

1. A cued layer's own Echo, Freeze, Feedback: NO difference, if the cued mix is made from the layer pictures the output already produced (way E). The trail is in those pixels (the kept picture is taken after the layer's feedback and effects, CompositorEngine.cpp:1150-1154). INFERRED; not run.
2. A cued MilkDrop layer, a cued simulation layer: NO difference, for the same reason. It is the layer's own picture.
3. Trails made by something the preview has to run for itself (the global effects under master cue; an effects-only cued layer; a clip previewed alone): these get a trail memory of their own. Their trails are then the true trails of what the preview is showing; they are not the output's trails because the picture going in is not the output's. Two honest small prints: such a trail starts empty at the moment the preview starts, and a second memory costs graphics memory (a Screen Split or Frame Stutter history is up to 237 MiB per chain: Pitfall 20, docs/claude/pitfalls.md:49). How many such preview chains may exist at once should be capped (ESTIMATE: one previewed clip plus the cue path).
4. What does remain, and it is not "a little":
   - MilkDrop in PREVIEW MODE. There is one engine and one picture. While a MilkDrop clip plays on the output, a different MilkDrop clip clicked by its name cannot be shown: loading its preset would change the audience's picture (milkdrop.md:88, 195). The preview can only show the playing engine's picture, or say that it cannot. MilkDrop is not changed in this build (old D9, citing his line L126). With no MilkDrop on the output, the one engine can serve the preview; the preset-load hitch is unmeasured.
   - A SIMULATION source previewed by name while a clip of the same kind plays: one instance per kind (CT:66); the preview shows the playing one's state unless a second instance for the preview is built. Ordinary (stateless) generated sources are not affected: their settings are written before every draw (Renderer.cpp:1306-1318, as cited at CT:66), INFERRED.
   These two are the only places where the preview cannot be true in the first build, and the next page should name them as such instead of "a little differently".

## RECOMMENDATION

ONE DESIGN: one frame, one rate, shared pixels, and the preview gives way.

1. SAME FRAME, SAME RATE. The preview monitor's picture is drawn inside Renderer::renderOpenGL, after the canvas is final and handed to the outputs (after Renderer.cpp:819), into its own target, and presented in the same window surface as the output monitor (one GL host, two rectangles; presentCanvas at :823 gains a second rectangle). No second GL context, no second cadence.
2. PREVIEWING NEVER TOUCHES THE OUTPUT'S STATE. compositeShow is never called twice. The layer loop (CompositorEngine.cpp:1076-1200) draws a layer's stages once when it is "on the output OR cued" and blends it into the output only when it is on the output. Fade clocks, video playheads, simulations and trail memories advance once per frame, as today. This is the build's first acceptance test: the output's pictures with the preview busy must equal the same output with the preview off, picture for picture (the capture path gives frames at a fixed time; Pitfall 52).
3. CUE MODE FROM SHARED PIXELS. The cued mix keys and blends the kept layer pictures into a cue target. Layers' trails are identical to the output's.
4. PREVIEW-ONLY CHAINS GET THEIR OWN MEMORY. Master cue's global stack, an effects-only cued layer and a clip previewed alone run under new history keys beside LayerStateKey::kGlobalEffects (LayerStateKey.h:29-40). Never left out, never shared.
5. FULL SIZE FIRST. The preview's targets are made at the composition's size in the first build: every effect looks as it will on the output, and the preview can reuse the compositor's working targets of that size. The size is one number; it is lowered (fixed, once) only if the test below says the heavy cases do not fit.
6. THE PREVIEW GIVES WAY. The previewed clip's video uploads come after the output's. The preview's time is kept out of the existing 12 ms reaction that switches an output effect off (Renderer.cpp:839-858). When frames run late for a sustained time and preview-only work is running, that work stops and the monitor holds a marked still; the cheap shared-pixel cue mix may go on. It comes back when the load has stayed low for a set time or when he previews something else.
7. THE TWO NAMED LIMITS (MilkDrop by name while MilkDrop plays; a simulation by name while its kind plays) are shown as limits, not as slightly wrong pictures.

WHY IT BEATS THE NEXT BEST. The next best is the old item 252: a smaller preview, always (way B), with trails approximated. That is cheaper to reason about, but it makes the preview a little untrue all the time in order to guard against an overload that may never happen, and it still guarantees nothing when the overload does come. The recommended design is true all the time, costs little in the common case because it reuses work already done, and has one hard guarantee for the rare case. It also matches his instinct: the same rate everywhere.

STRONGEST ARGUMENT AGAINST IT, AND WHY IT LOSES. "A monitor that is a few hundred pixels wide does not need a full-size picture; full size wastes the machine." True for the eye, and if the test shows the heavy cases do not fit, point 5 lowers the size with one number. But at this stage nobody knows the cost, and starting small bakes a permanent small untruth (pixel-based effects) into every preview to save an amount nobody has measured. Starting true and lowering on evidence is reversible; the other order is not noticed until he is misled on stage. Second argument: "the give-way rule can flicker". It can, if it comes back too eagerly; that is why its return is slow or by his own action, and why it is a test item.

WHAT THIS CHANGES IN THE SPEC (for Harmony): old D8 "smaller and less smooth" is replaced; old D9's first sentence stands and its trail sentence is narrowed to the two limits; assumption B-11's alternative b (second copies of every simulation and of MilkDrop) stays not built. Two points are new and are Harmony's to rule or to put to him: the "preview paused" mark (a look), and what the preview shows for a MilkDrop clip clicked while another MilkDrop plays.

## UNKNOWN UNTIL MEASURED

Nothing below is known. Each line: what, and the smallest test.

1. HEADROOM TODAY. How long his heaviest real show takes per frame at his real canvas size with his outputs on. Smallest test, no code: his show running, read frame_time_ms, peak_frame_time_ms, gpu_time_ms, peak_gpu_time_ms, peak_callback_ms from /api/state (src/api/ApiServer.cpp:1442-1490) for a minute. Against 16.67 ms.
2. COST OF MASTER CUE (upper bound). Smallest test, no code: put his usual global effects on the global stack a second time and read the same numbers; the rise is about what a second run at full size costs.
3. COST OF A PREVIEWED CLIP (upper bound). Smallest test, no code: one more layer at transparency 0 playing his heaviest clip with its effects (a layer at 0 is fully drawn, CompositorEngine.cpp:1078-1080); read the rise.
4. THE MILKDROP PRESET-LOAD HITCH. peak_frame_time_ms across a preset change (milkdrop.md:224 names the probe .harmony/probe-milkdrop.sh; not run).
5. THE REFRESH RATE THAT PACES THE SHOW. Which screen his app window sits on and its rate; a 120-pictures-a-second laptop screen would halve the time per frame. Whether JUCE then really renders at that rate is INFERRED from juce_OpenGLContext.cpp:895-905, not run. Smallest test: the same /api/state reading with the window on each screen.
6. WHICH EFFECTS LOOK DIFFERENT AT A SMALLER SIZE. Only needed if test 2 or 3 fails. Smallest test: captures of the pixel-based effects at full and at half size, compared.
7. Whether drawing "on the output OR cued" in one loop really leaves the output untouched: INFERRED from the code; it is the first acceptance test of the build (recommendation point 2), not a pre-test.
8. Graphics memory for the preview's own trail memories on his machine (the Pitfall 20 figure is per chain).

Tests 1 to 4 need a running app and nothing new built. They are not run here (this seat never launches the app) and they are Harmony's to schedule with him.

## ANSWER FOR BORIS

Yes: all three run at the same number of pictures a second, and I would build it no other way. One part of the app draws the output and the preview together, each moment once, so they cannot drift apart. "The output comes first" means three things. Previewing can never change what the audience sees. A layer you cue is shown from the picture the output already made, trails included, so it does not lie. And if the computer cannot keep up, the preview stops on a still picture marked "paused" until it can; the output loses nothing first. How close your shows come to that limit is not measured; a short test tells us before anything is built.

## THE ASSUMPTION FOR HIS NEXT PAGE

I assume the output and the preview always run at the same pictures a second and a cued layer looks exactly as on the output; if the computer cannot keep up, the preview pauses, marked, and the output keeps going.

b) The preview is always drawn smaller, so it rarely has to pause; price: a few effects look slightly different in it than on the output.

STATUS: DONE
