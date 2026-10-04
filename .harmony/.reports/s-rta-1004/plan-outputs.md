# PLAN -- lane "outputs": a settings window for every output screen (Device, Delay, Opacity, Brightness, Contrast, Red, Green, Blue) for display outputs and for Syphon

Author: architect (s-rta-1004). Read-only pass. Pin checks run by command: main `git rev-parse --short HEAD` = 185147b and
`git status --short -- src tests docs CMakeLists.txt` printed nothing (plain files read); lane/bf2 = 740b6d6 and lane/bf2-keys =
9eab9bd, both worktrees clean. Nothing was built, run or launched. Paths are relative to /Users/boriskarpman/projects/RealTimeAudio.
Labels: VERIFIED (read at 185147b, file:line, or a fact-sheet row its VERIFICATION section did not overturn), INFERRED, ASSUMED.
Sheets: FO = .harmony/.reports/s-rta-1004/facts-outputs.md, FR = facts-resolume-screen-delay.md, FP = facts-syncdial-parts.md,
FS = facts-saves.md (same folder).

STATUS: DONE (what could not be established by reading is listed in section 8 "NOT VERIFIED" and returned as not_verified).

---------------------------------------------------------------------------------------------------
## 1 GOAL
---------------------------------------------------------------------------------------------------
Boris (binding-decisions.md, "2026-10-04 (s-rta-1004)"): "I think we should copy what resolume does for delay. each output screen
can be delayed and that is set on output display properties. Makes it simpler. These are the resolume screen output adjustment
window" / "replace our sync with this" / "so the video matches the audio at the soundboard or wherever the vj is stationed in
middle of room preferably" / "yes add all of those output settings" / "syphon is an output and treated with same output settings
as a screen".

What is built: every output (each connected display, and Syphon) gets eight settings -- Device, Delay, Opacity, Brightness,
Contrast, Red, Green, Blue -- set in ONE new window, remembered per machine with the screen (question 38's default stands,
boris-clarify-38-41.md ANSWERS line), never in a show file.

Harmony constraint (lane outputs): the delay never blocks, waits or allocates per frame on the shared GL thread; its memory is
bounded, stated per output and per canvas size, and clamped by a stated rule; an output at Delay 0 with every colour setting at
its default shows exactly what it shows today (the same newest frame, no added copy, no added frame of latency) and a gate proves
it; the in-app preview, recordings, snapshots and the Eyes capture are never delayed or coloured by an output's settings; a
screen's settings are per machine, never in the show file.

The plan in one paragraph: the EXISTING shared IOSurface pipeline is deepened only while some live output asks for a Delay (4
slots stay 4 when nobody does), every offered frame gets a time stamp in a small lock-free log, and each output's present step
picks "the frame that was newest N ms ago" from that one shared ring -- the writer still makes exactly one copy per frame, so the
memory bill does not grow with the number of outputs. The colour stage is a small shader in each output's own present step, used
only when a setting is off its default (all-default keeps today's plain blit). Syphon becomes one more reader of the same ring.
Settings are keyed by the macOS display UUID in a new settings.json key. Everything is proven offscreen (private GL contexts, the
existing output probe) -- no gate opens an Output window.

---------------------------------------------------------------------------------------------------
## 2 ESTABLISHED FACTS (verified lines only)
---------------------------------------------------------------------------------------------------
F1  One per-output record exists today and holds no setting: `Live{window, target}` (src/output/OutputManager.h:98-103); members
    :120-138 hold nothing per output but the fingerprint lists. (FO Q1, VERIFICATION A1.)
F2  A display is known only by `DisplayInfo{x,y,w,h,scale,isMain}` and all six are in `operator==`
    (src/output/OutputTargets.h:17-27). JUCE's Display has no id and no name (FO VERIFICATION A2).
F3  JUCE builds its display list from `[NSScreen screens]`, in that order
    (build/_deps/juce-src/modules/juce_gui_basics/native/juce_Windowing_mac.mm:470-480, read as a plain file).
F4  The canvas is copied once per frame into ONE shared set of 4 IOSurface slots: `SurfacePool::kSlots = 4`
    (src/output/SharedFrameSet.h:65), strict round robin (SharedFrameSet.cpp:64), one blit + fence + flush (:65-71), the PREVIOUS
    copy offered as `front_` only when its fence completed, polled with timeout 0 (:46-58), a still-busy fence skips the frame
    (:49-50). +1 frame of latency by design (SharedFrameSet.h:117-119).
F5  A reader presents `frames.front()` only: src/output/OutputPresenter.cpp:87 (front), :97 (`readFBO[f.slot]`), :99
    (`glBlitFramebuffer`, GL_LINEAR, into `RenderGeometry::fitCanvas`). No shader, no per-output parameter. One code path for the
    window, the offscreen ctest and the in-app probe (OutputPresenter.h:3-6).
F6  The ring holds at most front, front-1, front-2 intact; a slot is rewritten 2 publishes after it stops being front
    (SharedFrameSet.h:64; FO VERIFICATION B, conclusion 2). It is a pipeline, not a history.
F7  All JUCE GL contexts render on ONE thread; an output context has swap interval 0 (src/ui/OutputWindow.cpp:10-16) and renders
    once per refresh of ITS display (FO Q4 steps 8-9, VERIFICATION A13). The main context is also display-link paced, swap
    interval 1 (FO VERIFICATION A13).
F8  The tap runs only while an output is live or the TEST-ONLY flag is set: `Renderer::publishToOutputs`
    (src/render/Renderer.cpp:966-972); four call sites :426, :484, :746, :819.
F9  A canvas size change makes a NEW generation of surfaces; the old one is retired and released `kRetireFrames = 120` publishes
    later (SharedFrameSet.h:66-68, SharedFrameSet.cpp:19-44); a reader's own CFRetain keeps a retired generation alive
    (OutputPresenter.cpp:39, OutputPresenter.h:8-9). `SurfacePool::tick()` runs only inside `publish` (SharedFrameSet.cpp:20).
F10 Frame bytes (arithmetic, 4 B/pixel): 1920x1080 = 8,294,400 B = 7.91 MiB; 3840x2160 = 33,177,600 B = 31.64 MiB (FO Q4,
    VERIFICATION A12).
F11 Other readers of the canvas, all on the main context, all reading `canvasFBO_` / `canvasTex_` directly: preview
    `presentCanvas` (Renderer.cpp:822, def. :938-962), recorder `submitFrame` (:884-888), Syphon `publishSyphonFrame` (:893-894,
    def. :2576-2599), capture / snapshot `processPendingCapture` (:897). No record-into-a-cell reader exists (FO Q5,
    VERIFICATION A10).
F12 Syphon today: `publishSyphonFrame` blits `canvasFBO_` into its own GL_TEXTURE_2D `syphonTexture_` (Renderer.cpp:2585-2595)
    and calls `SyphonOutput::publishTexture` (src/output/SyphonOutput.mm:72-92); only on the full render path, only when enabled
    and initialised (Renderer.cpp:893); toggled from the Output menu (`kOutputSyphon`, src/MainComponent.cpp:7261-7266, "Default
    OFF each boot"); build option `AUDIODNA_BUILD_SYPHON` (CMakeLists.txt:94). Its enabled flag is not saved anywhere (FS
    VERIFICATION: AppSettings has exactly 3 users).
F13 settings.json: `AppSettings` (src/model/AppSettings.h:18-19 two keys; AppSettings.cpp:19-41): `update` is read-modify-write
    and keeps unknown keys; a file that does not parse to an object reads as EMPTY, so the next `update` rewrites the file without
    its other keys (FS section 4 item 8).
F14 The `outputs` key is replaced whole from the fingerprint set, only when the SET changes (`persistWanted`,
    src/output/OutputManager.cpp:288-305); quitting writes nothing; `lastWanted_` advances before the write so a failed write is
    not retried (FS row C1).
F15 The one item list: `output::buildOutputMenu` (src/output/OutputMenuModel.h:43-58); command ids
    `kOutputDisabled = 1600 ... kOutputRestoreLast` (src/ui/MenuBarModel.h:93-102, "APPENDED, so no existing id moves").
F16 No per-output properties window exists (FO Q2, VERIFICATION A1). Existing secondary surfaces: `PreferencesDialog`
    (juce::DialogWindow, src/ui/PreferencesDialog.h:8), `TimingWindow` (a Component inside the main window,
    src/MainComponent.cpp:1745-1746). The Composition inspector's "Output Settings" section is the CANVAS size, saved in the
    show (FO VERIFICATION C4) -- a name clash to avoid.
F17 Offscreen proof exists today: tests/test_shared_frame_gl.cpp builds private CGL contexts with a 4.1 core pixel format
    (:22-27) and eight cases (:184-362, `REQUIRE(kSlots == 4)` at :366); `/api/output_probe` (8080) presents through a private
    CGL context with the same `presentSharedFrame` (src/test/TestServer.cpp:1850-1935); `set_output_tap` forces the tap
    (:243, Renderer.h:429).
F18 `ResettableSlider` with `setDefaultValue` (src/ui/UniversalParamControl.h:28-53).
F19 Resolume (FR, VERIFICATION marks CONFIRMED): Delay "between 0 and 100 ms", per screen, to absorb delay "after the outputs
    leave Resolume"; Opacity / Brightness / Contrast / Red / Green / Blue "to adjust mismatched outputs"; the Device menu shows
    "the output it's sending to"; "Every output can only have a single screen associated with it"; the output setup "is not tied
    to a certain composition". NOT DOCUMENTED by Resolume: the Brightness / Contrast / channel arithmetic, the displayed number
    scale, the Delay step, whether preview or a recording is delayed. THIRD-PARTY only: ranges 0..1 (Opacity) and -1..1 (the five
    centred rows); NDI / Syphon / virtual screens showed no Delay row (2019-2020).
F20 The stopped lane has no picture delay: it delays PCM before analysis, globally (FP Q3, VERIFICATION V3 conclusion A).
F21 macOS exposes a display UUID and name without a new dependency: `CGDisplayCreateUUIDFromDisplayID`
    (ColorSync/ColorSyncDevice.h:233 in the installed SDK, read as a header) and `NSScreen.localizedName` (AppKit NSScreen.h:58,
    macOS 10.15+). The app already has Objective-C++ sources (src/ui/NativeLayerHost.mm, src/output/SyphonOutput.mm) and links
    Cocoa. What the UUID survives is NOT verified (section 8).
F22 Docs still stale on main: docs/claude/testing-eyes.md:72 ("pkill -f ..."), docs/claude/architecture.md:107, :231, :334
    ("Triple-buffer"), .harmony/APP-INVENTORY.md:31 ("1249 unit tests") (FP Q6 and VERIFICATION V2; lines re-read today).

---------------------------------------------------------------------------------------------------
## 3 ITEMS
---------------------------------------------------------------------------------------------------

### OA1 THE DELAY MECHANISM

VERIFIED: F4-F10. A frame older than front-2 does not exist anywhere at canvas size (F6; the 480-cell effect ring is downscaled,
per clip chain and before the canvas -- FO Q6).

FORKS
 (a) A ring per output: each output context copies every frame into its own N surfaces.
 (b) A second, separate history ring beside the 4-slot pipeline (one extra full-canvas copy per frame on the main context).
 (c) THE SAME shared pipeline, deepened: the one copy the writer already makes per frame IS the history write; a time-stamped
     log says which slot holds which frame; each output keeps only a read rule (its Delay).

CHOICE: (c). Runner-up (b) loses: same memory, one more full-canvas blit per frame on the shared GL thread for nothing. (a)
loses twice: memory multiplies by the number of outputs, and each output would copy a full canvas per refresh on the shared
thread.

MEMORY (MiB; frames at 60 fps: 100 ms = 6, 250 ms = 15, 500 ms = 30; exact-frame figures, before slack)
 mechanism              canvas   delay   1 output   2 outputs   3 outputs
 (a) ring per output    1080p    100       47.5       94.9       142.4
                        1080p    250      118.7      237.3       356.0
                        1080p    500      237.3      474.6       711.9
                        4K       100      189.8      379.7       569.5
                        4K       250      474.6      949.2      1423.8
                        4K       500      949.2     1898.4      2847.7
 (b) and (c) shared     1080p    100 / 250 / 500 =  47.5 / 118.7 / 237.3   -- the same for 1, 2 or 3 outputs
                        4K       100 / 250 / 500 = 189.8 / 474.6 / 949.2   -- the same for 1, 2 or 3 outputs
 (the shared ring is sized by the LARGEST Delay among live outputs; "per output" cost is zero beyond the first.)
 All figures are on top of today's 4 slots (31.6 MiB at 1080p, 126.6 MiB at 4K, F10).

AS RULED (tiers with 2 slots of slack; extra slots beyond the base 4, at the 60 Hz publish class):
 tier        extra slots   1080p      4K           total slots
 off (0 ms)      0          0          0            4   (today, byte for byte)
 <= 100 ms       8          63.3      253.1         12
 <= 250 ms      17         134.5      537.9         21
 <= 500 ms      32         253.1     1012.5         36
 At the 120 Hz publish class (the preview's display refreshes at 120 Hz and the app keeps up) the extra slots double
 (14 / 32 / 62; `kMaxSlots = 66`).

THE RANGE AND ITS BILL (ruled)
 R-1 Range 0 to 500 ms, whole milliseconds, default 0 (what Boris was told as reading R11, boris-clarify-38-41.md; Resolume's
     own maximum is 100 ms, F19). The picture moves in steps of one canvas frame (16.7 ms at 60 fps): the number is a time, the
     effect is the nearest older frame.
 R-2 Budget: `kHistoryBudgetBytes = 640 MiB` for the EXTRA slots (the smallest round figure that admits 250 ms at 4K -- 537.9 MiB
     -- and bars 500 ms at 4K -- 1012.5 MiB). `maxDelayMs(canvasW, canvasH, publishClass)` = the largest tier whose extra slots
     fit the budget: 1080p -> 500; 2560x1440 -> 500 (450 MiB); 4K -> 250; 5120x2880 -> 100; 7680x4320 -> 0. At the 120 Hz class:
     1080p -> 500 (490 MiB); 4K -> 100.
 R-3 Clamp, never refuse: the stored Delay is kept as set; the APPLIED Delay = min(stored, maxDelayMs now). The Delay row's
     slider range is 0..maxDelayMs now (a state display; no text announces it). When the canvas shrinks again the applied value
     returns to the stored one by itself.
 R-4 Transient: while a tier or canvas size changes, the old and new generation coexist for up to `kRetireFrames = 120`
     publishes (F9, unchanged). Worst steady + transient at 4K (60 Hz class): 21 + 12 slots = 1044 MiB for about 2 s; at 1080p
     36 + 21 slots = 451 MiB. Stated, measured by gate row oa_memory, not budgeted away.
 R-5 Release: when no live consumer asks for a Delay any more the ring returns to 4 slots after 600 publishes of hysteresis
     (about 10 s; stops a drag across a tier edge from re-allocating), and when the tap stops altogether (last output closed,
     Syphon off) a deep generation is dropped at the next render frame by an idle trim in `publishToOutputs`' early-return branch
     (F9: `tick()` only runs inside `publish`, so without the trim a deep ring would stay allocated with no output live).

WHAT IT COSTS THE SHARED GL THREAD PER FRAME
 Writer (main context): unchanged -- the same single 1:1 blit, fence, flush (F4) -- plus one log push (two atomic 64-bit
 stores) and one steady-clock read (no syscall on macOS; the Renderer already reads a clock per frame, Renderer.cpp renderEnd).
 Reader (each output): one pick (a scan of at most 128 log entries, no allocation) and the same single blit as today (or one
 textured quad when a colour setting is off default, OA2). No wait, no lock, no allocation. One-time costs at a tier / canvas
 change: the surfaces of the new generation are created in `pool_.ensure` on the GL thread, exactly where a canvas size change
 creates them today (SharedFrameSet.cpp:19), and each context binds the new slots once. Bar: gate row oa_tier_spike. Named
 fallback if that bar is not met (reported to Harmony, never loosened): create the generation on the message thread
 (`SurfacePool::prepare`) and bind slots lazily, one per frame.

THE RULES OF THE READ (all pure, in a new GL-free header src/output/FrameHistory.h; unit-tested headless)
 H-1 Stamp: every frame gets the steady-clock time at which it was OFFERED as front (SharedFrameSet.cpp:58 is the place). An
     output with Delay d shows what a Delay-0 output showed d ms ago: pick(now, d) = the newest entry with stamp <= now - d.
     Property (unit, with a mutant): pick(t, d) == pick(t - d, 0). So the cadence of a delayed output is the cadence of today's
     output, shifted.
 H-2 Still filling (output just opened, Delay just raised, ring just deepened): no entry is old enough -> the OLDEST valid entry.
     The screen holds one still picture until the ring reaches the Delay. Never black beyond today's "nothing published yet".
 H-3 Never back in time: a reader remembers the serial it last showed and never shows a lower one. Raising the Delay holds the
     current picture until the ring catches up (a hold of exactly the added time); lowering it jumps forward at once. No slew.
     (Runner-up: glide the applied Delay at a bounded rate, as the stopped dial's `SyncSlew` did. Loses: a second clock, the
     shown number would differ from the applied one, and a drag in 1 ms steps already holds one frame at a time. Question 53
     lets Boris overrule.)
 H-4 Never a slot the writer is about to touch: the writer's next two target slots are not pickable (the same 2-publish grace as
     today, F6). With K slots, K - 3 completed frames are pickable; that is where "tier frames + 2 slack + base 4" comes from.
 H-5 Generations: an entry names (generation, slot, serial, stamp). The presenter binds whatever generation its pick names --
     the rule that already exists (OutputPresenter.cpp:90). So at a canvas size change or a tier change a delayed output keeps
     showing the OLD generation's frames (it holds CFRetains, F9; nobody writes them any more) until its pick moves into the new
     one: no black, no jump, the picture changes size d ms late, exactly as a Delay should. If a generation it needs is already
     released (`retainSurface` returns nullptr, SharedFrameSet.h:89-91): fall forward to the oldest pickable entry of a
     generation that can be retained.
 H-6 Epoch: a gap of more than 1 s between publishes (the tap was off, or the preview's GL context was gone -- F9, SharedFrameSet.h
     :7-10) starts a new epoch; entries of an older epoch are not pickable; with no pickable entry the reader shows `front()`
     exactly as today. So a re-opened output never shows a picture from minutes ago for d ms.
 H-7 Log safety: `FrameLog` is a fixed array of 128 entries, each two `std::atomic<uint64_t>` (the existing `packFront` word
     + the stamp), written stamp first; a reader re-reads the packed word after the stamp and skips a torn entry. Single writer
     (the main context). Readers: output contexts (same thread) and the probe's private context on an HTTP thread
     (TestServer.cpp:1870) -- the reason it must be lock-free and why it gets a [tsan] case. No new mutex.

STATES (each has a test in section 5)
 - Ring filling: H-2 (still picture, never black).
 - Delay dragged: H-3 (hold on raise, jump forward on lower; never backwards; no black). Stutter: only the hold itself.
 - Canvas size change: H-5.
 - Open: the window opens as today; its reader starts with no "last shown"; H-2 applies. If it is the first Delay consumer the
   ring deepens at the next publish (seamless: `front_` is not touched until the new generation's first frame is offered,
   SharedFrameSet.cpp:39-43).
 - Close: the reader's bindings and retains go (`PresenterGLState::release`, today's path); the manager recomputes the largest
   Delay; R-5 shrinks.
 - Hot-plug: a vanished display closes its output (target kept as interrupted, OutputManager.cpp:257-267); on return it opens
   again = "Open", with that screen's settings looked up again by its UUID (OA4).
 - 120 Hz display fed by a 60 fps canvas: the reader runs 120 times a second and picks by TIME, so each canvas frame is shown
   twice, delayed or not -- today's behaviour, shifted. A 50 Hz projector drops frames as today. If the CANVAS itself runs at
   120 fps (preview on a 120 Hz display, F7) the publish class is 120 and the ring is sized for it (R-2).
 - Preview hidden / app minimised: the writer stops; delayed outputs run out of newer frames and hold the last one -- the frozen
   picture of today (SharedFrameSet.h:7-10), reached d ms later.

CHANGE BY FILE / FUNCTION (no code)
 - NEW src/output/FrameHistory.h (GL-free): `HistEntry`, `FrameLog` (push, pick, noteWrite, newEpoch), `DelayTier` /
   `extraSlotsFor(tierMs, publishClass)`, `maxDelayMs(w, h, publishClass)`, `HistoryDepthPolicy` (wanted ms + canvas + class ->
   slot count, with the 600-publish shrink hysteresis), `PublishClass` from an interval average (60 or 120, with hysteresis).
 - src/output/SharedFrameSet.h / SurfacePool.cpp: `kSlots` stays 4 and now means the BASE depth; new `kMaxSlots = 66`;
   `SurfacePool::ensure(int w, int h, int slots = kSlots)` -- a different slot count is a new generation, like a new size;
   `Generation` holds `slots` and `s[kMaxSlots]`; `slotCount()`, `allocBytes()` (sum of `IOSurfaceGetAllocSize`), `retainSurface`
   bounds-checks against the generation's own count.
 - src/output/SharedFrameSet.h / .cpp: `setWantedDelayMs(int)` (message thread, one relaxed atomic); `publish` asks the policy
   for the slot count, stamps and logs each offered frame, measures the publish interval, starts an epoch after a gap; new
   `trimIdle()` (context current, tap off: back to the base depth, releases the deep generation); `historyStats()` (slots, alloc
   bytes, class, max delay now) as atomics for /api/state; `tex_` / `fbo_` sized `kMaxSlots`; a clock function pointer
   replaceable by tests.
 - src/output/OutputPresenter.h / .cpp: arrays sized `kMaxSlots`; `PresenterGLState` gains `lastShownSerial`; new
   `bool presentOutputFrame(SharedFrameSet&, PresenterGLState&, uint64_t packedLook, int64_t nowUs, unsigned targetFBO, int targetW, int targetH, ShownFrame* shown = nullptr)`.
   `presentSharedFrame` is KEPT with its signature and body; `presentOutputFrame` with a default look calls it and nothing else
   (the identity, section 5 row U-ID).
 - src/render/Renderer.cpp `publishToOutputs`: the early-return branch calls `sharedFrames_.trimIdle()` when a deep generation
   is held (one relaxed load otherwise).

RED-FIRST TESTS + MUTANTS: section 5, U-H1..U-H10, U-P1..U-P3, U-G1..U-G8. GATE ROWS: oa_identity_default, oa_delay_age,
oa_delay_monotonic, oa_memory, oa_tier_spike, oa_tap_cost.

### OA2 THE COLOUR STAGE

VERIFIED: F5 (no shader in the present step today), F19 (what Resolume documents and what it does not).

Resolume, as documented: Opacity, Brightness, Contrast, Red, Green, Blue per screen "to adjust mismatched outputs" (dim an LED
panel, remove some red from a projector). THIRD-PARTY (a posted Arena 5 xml): Opacity 0..1 default 1; the other five -1..1
default 0. NOT DOCUMENTED: the arithmetic of any row, whether Brightness adds or multiplies, the scale of the number shown, any
order of operations. So the arithmetic below is OURS (INFERRED as the conventional one), not a copy; section 6 has Boris compare
it with Arena on a real screen.

RANGES AND MEANING (ruled)
 row         range         step   default   "0" / default means        extremes
 Opacity     0 .. 100 %    1      100       100 % = untouched          0 % = black
 Brightness  -1.00 .. 1.00 0.01   0         nothing added              -1 = black, +1 = white
 Contrast    -1.00 .. 1.00 0.01   0         slope 1 (untouched)        -1 = flat mid grey, +1 = slope 2 about mid grey
 Red/Green/Blue -1.00 .. 1.00 0.01 0        gain 1 (untouched)         -1 = that channel removed, +1 = doubled (clipped)

THE ARITHMETIC, per pixel, on the frame's RGB (display-referred 0..1, as the canvas holds it), in this order:
 1. contrast:   c = (c - 0.5) * (1 + Contrast) + 0.5
 2. brightness: c = c + Brightness
 3. channels:   c.r *= (1 + Red); c.g *= (1 + Green); c.b *= (1 + Blue)
 4. clamp to 0..1
 5. opacity:    c = c * Opacity/100
 Alpha is written as it was read (the plain blit copies it today too). Opacity fades toward BLACK on a physical screen: the
 output window is opaque and cleared black (OutputPresenter.cpp:83-84); for Syphon the RGB fades toward black and alpha is left
 alone (question 52 lets Boris choose transparency for Syphon instead).
 Channel rows are GAINS, not offsets: an offset would tint black, which on a projector is the one thing a white-balance trim
 must not do. Brightness is an OFFSET (the conventional meaning); "dim a LED panel" is what Opacity does without lifting or
 crushing anything.

WHERE IT RUNS: in each output's own present step, in that output's own GL context, as the last thing before its framebuffer:
`presentOutputFrame` draws one quad through a program `output_present` (source string `EmbeddedShaders::outputPresent`, the
house place for shader source, CLAUDE.md shader rule 5 note; compiled per context by the presenter itself because the main
context's ShaderManager cannot be used from another context, Pitfall 40). Uniforms: `u_output_frame` (sampler2DRect),
`u_output_size`, `u_output_contrast`, `u_output_brightness`, `u_output_gain` (vec3), `u_output_opacity`. The program is compiled
when the context is created (`Presenter::newOpenGLContextCreated`; the Renderer's context creation for the Syphon reader; lazily
in the probe's private context), never on a slider's first touch. The viewport is the same `fitCanvas` rectangle the blit uses.

THE SETTINGS WORD: one output's seven numbers are integers (Delay ms 0..500; Opacity 0..100; the five centred rows in
hundredths, -100..100) packed into ONE 64-bit word (`output::OutputLook`, pure header, `pack` / `unpack` / `isDefault` /
`applyReference` = the CPU mirror of the shader). The message thread stores the word in one `std::atomic<uint64_t>` per output;
the GL thread loads it once per present. No torn read between rows, no lock (the pattern of Pitfall 63's one-word tuple).

ALL-DEFAULT IS A NO-OP -- the proof, three layers:
 1. Structural: `presentOutputFrame` with `isDefault(look)` (Delay 0, Opacity 100, five zeros) calls today's `presentSharedFrame`
    and returns. No shader, no pick, no log read. With no consumer asking for a Delay the ring stays at 4 slots (R-5). So the
    path, the frame, the copy count and the latency are today's. (Unit U-ID + source-law row L-3.)
 2. Arithmetic: `applyReference(c, defaultLook) == c` for all 256 values of each channel (unit U-L2).
 3. Pixels: the shader path FORCED with a default look (test hook) against the blit path: byte-identical at a 1:1 target;
    at a scaled target the largest difference is reported against a bar of 2/255 (U-G7). This matters for the moment a colour
    row first leaves its default: the picture must not visibly shift when the path switches from blit to quad.

CHANGE BY FILE: NEW src/output/OutputLook.h (pure); src/render/EmbeddedShaders.h (+ `outputPresent`);
src/output/OutputPresenter.cpp (the program, the quad, the non-default path). Tests: U-L1..U-L5, U-G5..U-G7. Gate row:
oa_colour_pixels.

### OA3 SYPHON AS AN OUTPUT

VERIFIED: F12. Resolume's own Syphon / NDI screens showed no Delay row (F19, third-party); Boris's ruling outranks that:
"syphon is an output and treated with same output settings as a screen".

What it takes:
 - Its own read point: Syphon becomes a third kind of READER of the shared frames, running in the main context where it runs
   today. `Renderer` owns one `output::PresenterGLState syphonReader_` and one look atom `syphonLook_`. In `publishSyphonFrame`:
   look all-default -> today's body, unchanged (the direct blit of `canvasFBO_`, Renderer.cpp:2585-2595: newest frame, no +1);
   otherwise -> `presentOutputFrame(sharedFrames_, syphonReader_, look, now, syphonFBO_, w, h)` (target = the Syphon texture at
   canvas size, so `fitCanvas` is 1:1), then `publishTexture` as today. After it, `canvasFBO_` is re-bound exactly as today's
   last line does (:2595) -- the capture that follows reads the canvas (OA6).
 - The tap: `publishToOutputs` also runs when Syphon is enabled, initialised and its look is not default (one more relaxed load
   in the guard at Renderer.cpp:968). The largest-Delay computation (OA1) includes Syphon only while it is enabled.
 - Stated consequence: with every setting at default Syphon is byte-for-byte today's. The moment any Syphon setting leaves its
   default, Syphon reads `front()` like a window does, i.e. one canvas frame later than today's direct blit (F4). One frame,
   once, at the moment of the first change; reported in /api/state, not hidden.
 - Still only on the full render path (not the three early paths), as today (Renderer.cpp:408-413 comment).
 - Identity: it has no display. Its settings live under the fixed key "syphon" (OA4). One server, name "Audio-DNA"
   (SyphonOutput.h:62).
 - In the window: one entry "Syphon" after the displays, listed when `isSyphonAvailable()` (Renderer.cpp:42-45; a build without
   the framework lists nothing -- Pitfall 26's compile-time detection is not touched). Device row reads "Syphon (Audio-DNA)".
   State text "on" / "off" from `isSyphonEnabled()`; its rows are editable while it is off (as for a screen whose output is
   off). Turning Syphon on or off stays where it is (the Output menu's "Syphon Output" item); it is still OFF at every launch.
 - `openGLContextClosing`: `syphonReader_.release()` beside the existing Syphon teardown (Renderer.cpp:1087-1092).

Runner-up: colour Syphon straight from `canvasTex_` with a second (GL_TEXTURE_2D) shader variant when Delay is 0, and use the
ring only for a Delay. Loses: two shader variants and two paths for one output kind; the only gain is one frame in a state
(colour-trimmed Syphon) where one frame is not what the user is tuning.

Tests: U-G8 (the reader path into an FBO target at canvas size, decoded), L-4; gate rows oa_syphon_look, oa_canvas_untouched.

### OA4 IDENTITY AND MEMORY OF SETTINGS

VERIFIED: F2, F3, F13, F14, F21. The dead field: `Composition::outputDisplay` (default -1, no reader) and a deck file's
`outputDisplay` (written 1, ignored); tests/test_output_law.cpp:221-229 forbids OutputManager and MainComponent from naming it
(FO Q8, table row "Composition.h").

IDENTITY (ruled): a screen's settings are keyed by the macOS display UUID.
 - NEW src/output/DisplayIdentity.h (pure struct + the join) and DisplayIdentity.mm (macOS; an empty list elsewhere):
   `std::vector<ScreenIdentity> currentScreenIdentities()` walks `[NSScreen screens]` -- the list and order JUCE itself uses
   (F3) -- and returns for each {uuid string from `CGDisplayCreateUUIDFromDisplayID` of its `NSScreenNumber`, name from
   `localizedName` under an availability check (else empty)}. Message thread only. No new dependency (F21).
 - The join to `DisplayInfo` is by index (F3), cross-checked by the logical bounds; a mismatch falls back to "no identity".
 - Fallback id when the UUID is empty or duplicated in one list: "geom:WxH@scale" (settings then follow the size, the best
   the fingerprint can do). Unit-tested as a pure function.
 - `DisplayInfo` and its `operator==` are NOT touched (F2: a new field there would silently become part of "the same screen"
   and of the saved `outputs` set). The wanted-set machinery, the match ladder and "Restore Last Outputs" keep using the
   geometry fingerprint unchanged. Moving THAT to UUIDs is not in this lane (section 9).
 - What it survives -- NOT VERIFIED by reading (FO U3): re-plug to the same port, a resolution change, a reboot are ASSUMED to
   keep the UUID (that is what the API is for); two identical projectors are ASSUMED to get two UUIDs that may follow the PORT,
   not the device. Cheapest refuting test: section 6 check B-4 (Boris, real hardware, read from `/api/state`), because no gate
   may unplug his screens. The design does not depend on the answer: a wrong match shows another screen's Delay, which he sees
   in the window's Device row and fixes with one drag.

THE SCHEMA: a NEW settings.json key, `AppSettings::kOutputSettings = "outputSettings"`:
 {"version": 1, "screens": [{"id", "name", "w", "h", "lastSeen", "delayMs", "opacity", "brightness", "contrast", "red",
 "green", "blue"}], "syphon": {the seven numbers}}
 (integers as in the settings word; "name", "w", "h" only so a screen that is not connected can be shown by name; "lastSeen" a
 day stamp). At most 32 screens: the one seen longest ago is dropped when a 33rd is added. Reader: unknown keys ignored, a
 missing number reads as its default, an out-of-range number is clamped, a record without "id" is skipped, anything that is not
 an object reads as "no settings". A record equal to all defaults is still written once it exists (it carries the name).
 Why a new key and not fields inside "outputs": that key is replaced whole from the fingerprint set, only when the SET changes
 (F14), is pinned by tests/test_output_law.cpp, and is keyed by geometry -- a Delay there would be lost by the next output
 toggle, would not be written when only a Delay changes, and would have no home for a screen that was never an output
 (FO VERIFICATION C3). An older build keeps the unknown key on its own writes (F13).

WHO WRITES: NEW src/output/OutputSettingsStore.h / .cpp (message thread): the records in memory, `look(id)`, `setLook(id, ...)`,
`touch(id, name, w, h)`, a 500 ms debounce timer (the stopped lane's `kSaveDelayMs` idea, re-typed, not cherry-picked), a flush
when the settings window closes and in `MainComponent`'s shutdown before `outputs_.shutdown()`. A failed write keeps the dirty
flag and is retried at the next change or flush; stderr only -- no on-screen text (Boris's list of allowed failure texts does
not include it). `OutputManager::persistWanted` and `shutdown` are untouched ("quitting writes nothing" stays true of the
wanted set).

settings.json's weakness (F13) -- MEND, small: `AppSettings::update`, when the file exists, is not empty and does not parse to
an object, first copies it beside itself as settings.json.unreadable (replacing an older copy) and only then rewrites. The
existing behaviour and its test ("a corrupt file ... rewritten valid") stay true; nothing is shown. Reason to mend rather than
respect: this lane puts the first values a person tuned by hand in a room into that file.

A SCREEN THAT IS NOT CONNECTED: its record stays in the file untouched; nothing reads it until the screen is seen again; when
its display comes back (open, hot-plug reopen, reconcile) the manager looks the UUID up again and applies the record. In the
window it is listed only if it is one of HIS outputs waiting to come back (an interrupted or saved-for-Restore target, matched
to a record by the Reopen ladder's size rule) -- see OA5.

THE DEAD `outputDisplay` FIELD: not read, not written differently, not used for any of this. Removing it from the deck writer
changes a file format and belongs to the saves lane (section 9). tests/test_output_law.cpp keeps forbidding the name, and the
new store and window are added to the files it scans.

Open question 50 (key and MIDI settings between launches): no line of this lane depends on it. If they go into settings.json,
the mend above protects them too; if they are not remembered, nothing here changes.

Tests: U-S1..U-S8, U-A1, U-D1..U-D3; gate row oa_settings_roundtrip.

### OA5 THE WINDOW

VERIFIED: F15, F16, F18; CLAUDE.md "Outputs" and "PopupMenu" patterns; Pitfalls 5, 40, 41, 57, 59.

FORKS: (a) ONE window: the outputs listed on the left, the eight rows of the selected one on the right (the shape of Resolume's
Advanced Output: a list of screens and one "Screen" panel -- Boris's screenshot shows that panel). (b) One panel per output,
opened from that output's menu item. (c) A section inside the main window's inspector.
CHOICE: (a). (b) loses: a click on a display item TOGGLES its output in both doors (F15) and that must not change; and a panel
cannot hang off an output window (it never takes the keyboard or a click, Pitfall 40). (c) loses: the inspector follows the
selected clip / layer / composition, and "Output Settings" there already means the canvas size saved in the show (F16).

RULED
 W-1 Class: NEW src/ui/OutputSettingsWindow.h / .cpp -- a normal, non-modal `juce::DocumentWindow` with the native title bar,
     title "Output Screens" (NOT "Output Settings": F16's name clash). Normal level, closable, resizable within a minimum; it
     may take the keyboard like any app window (it is not an output). It never opens, closes or moves an output.
 W-2 Door: one new item "Output Screens..." at the end of the ONE item list (after "Restore Last Outputs"), added in
     `output::buildOutputMenu` (new optional `settingsId` parameter, default 0 = today's list) so the Output menu and the TopBar
     "Outputs" button both get it; id `kOutputSettings` APPENDED after `kOutputRestoreLast` (src/ui/MenuBarModel.h:102; no id
     moves); always enabled. Handler: one `case` in `MainComponent::handleMenuCommand` that shows the window (creates it on
     first use) -- an opener of THIS window only.
 W-3 Left list, one entry per: every CONNECTED display, in menu order, titled with the menu's own label ("Display 2
     (1920x1080)") -- whether or not its output is on, so a screen can be set before it is switched on; then "Syphon" when
     available (OA3); then each of his outputs whose screen is NOT connected (interrupted or waiting for Restore), dimmed. Each
     entry carries a STATE word: "on", "off", "not connected". A state, not an event.
 W-4 Right panel "Screen", eight rows, top to bottom as Resolume: Device, Delay, Opacity, Brightness, Contrast, Red, Green, Blue.
     Rows 2-8 are `ResettableSlider`s, each with `setDefaultValue` (0 ms; 100 %; 0; 0; 0; 0; 0), right-click = reset, a text box
     showing "N ms", "N %", and two decimals for the five centred rows. The Delay slider's range is 0..maxDelayMs now (R-3).
     Rows are editable for an entry that is off or not connected (the values are saved for when it returns).
 W-5 The Device row: READ-ONLY text naming the screen -- the menu label plus the monitor's name ("Display 2 (1920x1080)  LG HDR
     4K"), "Syphon (Audio-DNA)" for Syphon, the remembered name plus "not connected" for an absent screen. In Resolume the Device
     menu picks which physical output a Screen object sends to (F19). Here an output IS a display and its settings are remembered
     WITH the screen (question 38's default), so "moving" an output is switching one display off and another on in the Outputs
     list; Boris's words ask for the settings of each output screen, not for a way to re-route one. Question 51 lets him ask for
     the menu.
 W-6 Model-driven: a pure `OutputSettingsModel` (NEW src/ui/OutputSettingsModel.h) builds the entries from {display list,
     identities, live flags, interrupted + saved targets, Syphon available / enabled, the store's records, maxDelayMs}. The
     window re-builds it on a 10 Hz timer and repaints or moves a slider ONLY when what it would PAINT differs (Pitfalls 41, 59;
     no periodic `repaint()`, Pitfall 57) -- so a hot-plug, an output toggled from the menu, or a value set through the test
     route shows up without any event wiring. `OutputManager::onLiveCountChanged` keeps its shape
     (tests/tool_uitoggle_snapshot.cpp:151 mimics it).
 W-7 A slider change -> `OutputSettingsStore::setLook` -> one callback in `MainComponent` -> `OutputManager::applyLook(id, word)`
     (stores the word in the live window's atom, recomputes the largest Delay -> `SharedFrameSet::setWantedDelayMs`) or
     `Renderer::setSyphonLook(word)`. No text appears anywhere; the picture on that screen is the feedback.
 W-8 Not bindable, not mappable, not recorded: the seven numbers are machine settings, not ControlPaths -- no key, MIDI, OSC,
     mapping or take touches them (question 54 lets Boris ask for Opacity on a control). No 7070 setter in this lane; /api/state
     reports them read-only.
 W-9 Any popup in the window uses `showMenuAsync` with the top-level parent (none is needed with a read-only Device row).

EVERY STATE THE VISUAL GATE MUST CAPTURE (test mode; the window is opened by a test route, captured by its Quartz window id; the
display list is a FIXTURE handed to the window's model only -- it can open nothing):
 V1  the Outputs list (TopBar door) with the new last item.
 V2  the window, one display (main), output off, all defaults, the display selected.
 V3  three displays in the list (fixture), second one "on", selected, all defaults.
 V4  every row off its default (Delay 250 ms, Opacity 60 %, Brightness -0.20, Contrast 0.35, Red -0.10, Green 0.05, Blue 0.40).
 V5  Delay at the top of its range, 1080p canvas (500 ms).
 V6  the same on a 4K canvas: the slider's range ends at 250 ms.
 V7  Syphon selected, state "off".
 V8  Syphon selected, state "on", non-default values.
 V9  a screen that is not connected: dimmed entry, Device row with its remembered name and "not connected".
 V10 a build / fixture with Syphon unavailable: no Syphon entry.
 V11 the window at its minimum size with a long monitor name (truncation, nothing clipped mid-glyph).
 V12 a row just reset by right-click (back at its default) next to changed rows.
 V13 the window beside the main window with the main window's inspector showing the composition's own "Output Settings"
     section -- the two names must not read as the same thing.

Tests: U-M1..U-M6, U-MENU1, L-1, L-2. Visual gate: after stage S6, before Boris sees it.

### OA6 WHAT IS NEVER DELAYED OR COLOURED

VERIFIED (F11): preview = `presentCanvas`, samples `canvasTex_` (Renderer.cpp:938-962); recorder = `glReadPixels` of
`canvasFBO_` (:884-888; VideoRecorder.cpp:130); PNG snapshot and Eyes `render_frame` = `processPendingCapture`, `glReadPixels`
of `canvasFBO_` (:897; Renderer.cpp:2350 ff.); a record-into-a-cell reader does not exist (FO Q5).

Why an output's settings cannot reach them:
 1. The settings exist only as a packed word in an atom owned by an OutputWindow (or `syphonLook_`), and the only function that
    takes one is `output::presentOutputFrame`, which draws into the framebuffer it is handed: a window's framebuffer, the
    probe's private FBO, or `syphonFBO_`. It never receives `canvasFBO_` as a target.
 2. The ring is written FROM the canvas by a read-only blit that leaves `canvasFBO_` bound (SharedFrameSet.cpp:65-72); deepening
    it adds no write to the canvas.
 3. The Syphon reader runs in the main context after the recorder and before the capture (Renderer.cpp:884-897). It is the one
    place where an output's present step shares a context with the canvas, so it is the one place a binding mistake could reach
    the capture. Guard: it re-binds `canvasFBO_` on exit as today (:2595), restores the viewport, and gate row
    oa_canvas_untouched runs with Syphon on and a wild look.
 Proof:
 - Source law (tests/test_output_law.cpp, new cases L-3..L-5): in Renderer.cpp `presentOutputFrame(` appears exactly once and
   inside `publishSyphonFrame`; `presentCanvas`, `processPendingCapture` and src/recording/VideoRecorder.cpp contain neither
   `OutputLook` nor `FrameLog` nor `syphonLook_`; OutputPresenter.cpp never names `canvasFBO`.
 - Live, offscreen (Harmony): oa_canvas_untouched -- with the probe's look and the Syphon look set to Delay 500, Brightness -1,
   Opacity 0, a `render_frame` capture (the capture owns the time override, Pitfall 52) is byte-identical to the same capture
   with everything default, while `output_probe` and the Syphon read-back in the same run DIFFER from it (the positive control:
   the settings were live).
 - The preview: `presentCanvas` is not edited by this lane at all (L-5 pins it textually); the capture row covers the canvas it
   samples.
 The future record-into-a-cell (open question 48 is about which Record boxes exist, not this): whoever builds it reads the
 canvas like the recorder does; Pitfall 68 (draft in OA9) says so in one line. Either answer to 48 changes nothing here.

### OA7 PROOF WITHOUT AN OUTPUT WINDOW

VERIFIED: F17; the SCREEN-SAFETY LAW (.harmony/HANDOFF.md, "SCREEN-SAFETY LAW" section) and FO Q8's rig rules.

 1. The ring's index and age arithmetic: pure unit tests on src/output/FrameHistory.h (no GL, no clock: stamps are numbers),
    each with a named mutant -- U-H1..U-H10.
 2. The present step, offscreen, pixel by pixel: tests/test_shared_frame_gl.cpp already has a writer context and two reader
    contexts with FBO targets and no window (F17). New cases publish a KNOWN sequence -- frame n is a solid colour that encodes n
    -- with an injected clock (frame n offered at n * 16,667 us), present through `presentOutputFrame` with a Delay, read the
    target back and DECODE which frame is shown. Same for the colour pass against `OutputLook::applyReference`. U-G1..U-G8.
 3. In the running app, still no window: the existing `output_probe` route (8080, test-server builds only) presents through its
    private context via the SAME function; it gains optional look fields and returns `shown_serial`, `shown_age_ms`,
    `history_slots`. `set_output_tap` forces the tap. New 8080-only routes: `POST /api/output_settings` (set a look for an id
    or "probe" / "syphon"), `POST /api/output_settings_window` (open / close / select / display-list fixture for the WINDOW
    model only), `POST /api/syphon_probe` (read `syphonTexture_` back on the GL thread into a PNG). None can open an output;
    L-2 pins that in source.
 4. The Delay-0 / all-default identity gate: U-ID (unit: the default path IS `presentSharedFrame`), the eight existing
    test_shared_frame_gl cases unchanged and green, and live oa_identity_default (probe bytes == canvas capture bytes, the
    existing o_probe_matches_canvas bar; `history_slots == 4`; `shown_serial == frame_serial`).
 5. The memory bar: measured IN the app, not from outside (no `sample`, no lldb -- RIG-RULES): `/api/state.outputs.history` =
    {slots, alloc_bytes (sum of `IOSurfaceGetAllocSize` over the current generation), retired_bytes, publish_class,
    max_delay_ms}. oa_memory compares slots with the table of OA1 and alloc_bytes with slots x w x h x 4 (+2 % for row padding),
    at 1920x1080 and 3840x2160 (the test canvas lock), and checks the return to 4 slots.
 6. The settings round trip: unit (U-S*) and live oa_settings_roundtrip on a scratch settings file (`AUDIODNA_SETTINGS_FILE`).
 7. `o_no_window_opened` (the existing Quartz window-list row) runs across every new row.
 What is left for Boris only: section 6.

### OA8 THE STOPPED BRANCHES

VERIFIED: F20; FP T2; rulings-bf2.md H-17 (c) and (e).
 CARRIED (as retyped lines, never as a merge or cherry-pick -- both branches stay unmerged):
 - docs/claude/testing-eyes.md:72, the kill advice -> "quit only the pid you launched" (lane commit dc59573, one line).
 - docs/claude/architecture.md:107 "Triple-Buffer Atomic Swap" -> the seqlock bus (inside lane commit f14eb31, hand-carried
   without its dial lines) AND lines :231 and :334, which the lane did not fix (FP VERIFICATION V2).
 - .harmony/APP-INVENTORY.md:31 test count: recounted on main after this lane's tests land (`ctest -N`), not taken from lane
   commit f4a7e34 (its number includes 69 dial tests).
 - One IDEA, no code: a debounced settings save (the lane's `SyncOffsetController` `kSaveDelayMs`), re-typed in
   `OutputSettingsStore`.
 All three doc fixes go into stage S7 of this lane. CLAUDE.md's own "triple-buffer FeatureBus" wording (FP Q6 item 2) is named
 for Harmony's decision in S7 (it is in the Sacred Rules text).
 DROPPED with the lane (nothing of it is a picture delay): `AnalysisDelayLine`, `SyncSlew`, `BeatLead`, `SyncVenues`,
 `SyncOffsetController`, the `syncOffsetMs` snapshot field, the /api/sync routes and OSC addresses, `SyncNudge` bindings and
 `bindingIsLive`, RecorderHost's show-time stamps, the sync probes, the [timing] test, Pitfall 68's lane text (lane/bf2 at
 740b6d6: 56468cd, 73cfc4a, f92bd6f, 0a4f4e7, cb63bc8, 2092982, 0343612, 585a03d, 6bd1253, 2cbab6a, b56ed5f, f14eb31's dial
 lines; lane/bf2-keys at 9eab9bd: e15d1d0, df98f78). `AppSettings::kSync` is not added.
 NAMED FOR ANOTHER LANE (not the dial, not outputs): the music-beat wheel and the Gain slider's 140 px (top bar -> the
 beat-nudge lane or its neighbour), the MIDI-learn title's em dash and `selectAt` / `bindingTag` / `stateForTests` (df98f78 ->
 a keys lane), the take-origin finding H-14 (a finding, no code).

### OA9 STAGES AND ORDER -- see section 4. Pitfall draft and the fence with the beat-nudge lane:

PITFALL NN (Harmony assigns 68), draft: "An output's Delay and colour live only in its own present step
(`output::presentOutputFrame`): the ring is the shared IOSurface pipeline deepened (one copy per frame, whatever the number of
outputs), read by TIME through `FrameLog`; all-default is today's plain blit of `front()` at 4 slots; the canvas, the preview,
the recorder, snapshots, captures and any future record-into-a-cell read the CANVAS and are never delayed or coloured; a screen's
settings are keyed by display UUID in settings.json `outputSettings`, never in a show -- before touching `presentOutputFrame`,
`SharedFrameSet::publish`, the Syphon publish, or adding a canvas reader."

FENCE with the beat-nudge lane (it owns the tempo area of the top bar):
 - This lane does NOT touch: src/ui/TopBar.h / .cpp, src/ui/TopBarModel.h, src/analysis/**, the BPM tracker, FeatureSnapshot,
   src/recording/**.
 - The beat-nudge lane must NOT touch: src/output/**, src/ui/OutputWindow.*, src/ui/OutputSettings*.*, the Output block of
   src/ui/MenuBarModel.h (ids 1600-1699), `Renderer::publishToOutputs` / `publishSyphonFrame`, the "outputSettings" key.
 - Files BOTH touch, by named hunk only: src/MainComponent.cpp / .h (this lane: one `case` in `handleMenuCommand`, one member,
   two lines in the constructor and in the shutdown order; nothing in the top-bar wiring), src/model/AppSettings.h (one key
   constant each), src/api/ApiServer.cpp and src/test/TestServer.cpp (separate routes), docs/claude/pitfalls.md and CLAUDE.md
   (this lane: Pitfall 68 and the "Outputs" lines), .harmony/APP-INVENTORY.md. Whichever lane merges second rebases; neither
   reformats those files.

---------------------------------------------------------------------------------------------------
## 4 STAGES + ORDER (one builder context per stage; every live row and every gate verdict is Harmony's, never a builder's)
---------------------------------------------------------------------------------------------------
A builder builds, runs ctest and the named mutants (RED first), and reports. A builder never launches the app for a gate row,
never opens an Output window, never touches Boris's running Audio-DNA or Resolume.

G0 (Harmony, on main, before any stage; no code): row oa_publish_rate -- test mode, `set_output_tap` on, read
   `/api/state.outputs.frame_serial` twice 10 s apart with the preview panel on the built-in display. It tells which publish
   class this Mac runs (60 or 120) and so which column of OA1's table the live rows use. INFO row; decides no design.

S1 HISTORY CORE (pure + pool). Owns: NEW src/output/FrameHistory.h; src/output/SharedFrameSet.h (SurfacePool part only);
   src/output/SurfacePool.cpp; NEW tests/test_frame_history.cpp; tests/test_surface_pool.cpp; tests/CMakeLists.txt (one target).
   Proves: U-H1..U-H10, U-P1..U-P3 with their mutants. The app's behaviour is unchanged (nobody asks for a depth yet).

S2 WRITER + READER. Owns: src/output/SharedFrameSet.h (SharedFrameSet part) / .cpp; src/output/OutputPresenter.h / .cpp;
   tests/test_shared_frame_gl.cpp. Proves: U-G1..U-G4, U-ID; the eight existing cases green and unedited. The app still passes
   a default look everywhere (no caller sets a Delay yet): identity by construction.

S3 COLOUR STAGE. Owns: NEW src/output/OutputLook.h; src/render/EmbeddedShaders.h (one string); src/output/OutputPresenter.cpp
   (the program and the non-default path); NEW tests/test_output_look.cpp; tests/test_shared_frame_gl.cpp (colour cases).
   Proves: U-L1..U-L5, U-G5..U-G7. S3 may run in parallel with S4 (no shared file) but AFTER S2 (shares OutputPresenter.cpp).

S4 IDENTITY + STORE. Owns: NEW src/output/DisplayIdentity.h / .mm; NEW src/output/OutputSettingsStore.h / .cpp;
   src/model/AppSettings.h / .cpp; NEW tests/test_output_settings_store.cpp, tests/test_display_identity.cpp;
   tests/test_app_settings.cpp; root CMakeLists.txt (the new sources; FO VERIFICATION C5). Proves: U-S1..U-S8, U-A1, U-D1..U-D3.

S5 WIRING (the first stage that changes what the app can do). Owns: src/ui/OutputWindow.h / .cpp (the look atom, the clock,
   `presentOutputFrame`); src/output/OutputManager.h / .cpp (`applyLook`, identity at open and at reconcile, the largest
   Delay); src/render/Renderer.h / .cpp (`publishToOutputs` guard + idle trim, `publishSyphonFrame`, `syphonReader_`,
   `setSyphonLook`); src/test/TestServer.h / .cpp (the three routes, the probe's fields, state); src/api/ApiServer.cpp
   (read-only state fields); src/MainComponent.h / .cpp (own the store; route its callback; flush at shutdown);
   tests/test_output_law.cpp (L-2..L-5); .harmony/probe-outputs.py / .sh (the new rows, written by the builder, RUN by Harmony).
   Proves by ctest: the law cases. Harmony then runs every oa_* row of section 5 plus every existing o_* row, and a TSan run of
   the [tsan] cases.

S6 THE WINDOW. Owns: NEW src/ui/OutputSettingsWindow.h / .cpp, src/ui/OutputSettingsModel.h; src/output/OutputMenuModel.h;
   src/ui/MenuBarModel.h / .cpp (the id, the call); src/MainComponent.cpp (one `case`, the window's owner);
   src/test/TestServer.cpp (the window route's handler body); NEW tests/test_output_settings_model.cpp;
   tests/test_output_menu_model.cpp; tests/test_output_law.cpp (L-1). Proves: U-M1..U-M6, U-MENU1, L-1.
   Then the VISUAL GATE (Harmony): a capture builder produces V1..V13 (test mode, window id captures only, no synthetic input,
   no Output window), five critic seats review, Harmony rules; only then does Boris see the window.

S7 DOCS (after the gates). Owns: docs/claude/integration.md ("Output windows": the settings, the key, the window, the routes),
   docs/claude/rendering.md ("The output tap": depth, the log, the present step), docs/claude/pitfalls.md (Pitfall 68),
   docs/claude/testing-eyes.md (new 8080 routes; line 72's kill advice, OA8), docs/claude/architecture.md (:107, :231, :334),
   CLAUDE.md (the "Outputs" pattern line, Key capabilities, the pitfall index line), .harmony/APP-INVENTORY.md (the Output
   menu row, the recounted test line). No source file.

ORDER: G0 -> S1 -> S2 -> (S3 || S4) -> S5 -> Harmony's live rows -> S6 -> visual gate -> S7 -> Boris (section 6).
What Harmony runs herself: G0; all oa_* and o_* live rows; the TSan run; every mutant's RED verdict she wants re-witnessed; the
visual gate; the merge decision.

---------------------------------------------------------------------------------------------------
## 5 TESTS + GATE ROWS (pre-registered; each RED first; a bar is met or reported, never loosened)
---------------------------------------------------------------------------------------------------
RED arm = the named mutant applied to the finished code, or "TREE-BEFORE" = the case fails to compile or fails on the tree
before the stage. Mutants are one-line edits, each must turn exactly its named case RED.

UNIT -- tests/test_frame_history.cpp, tag [frame_history] (S1)
 U-H1 "frame history: delay 0 picks the newest entry"                         RED: M-H1 pick returns the oldest.
 U-H2 "frame history: pick(t, d) equals pick(t - d, 0)" (1000 seeded schedules, stamps with +-3 ms jitter)
                                                                              RED: M-H2 compares stamp < instead of <=.
 U-H3 "frame history: filling holds the oldest entry"                         RED: M-H3 fallback returns the newest.
 U-H4 "frame history: a raised delay never shows an older serial"             RED: M-H4 drop the last-shown clamp.
 U-H5 "frame history: a lowered delay jumps forward at once"                  RED: M-H5 clamp applied both ways.
 U-H6 "frame history: the writer's next two slots are never picked"           RED: M-H6 guard 2 -> 0.
 U-H7 "frame history: a gap over one second starts a new epoch"               RED: M-H7 epoch compare removed.
 U-H8 "frame history: slots per tier and class" -- exact: class 60 -> 4, 12, 21, 36; class 120 -> 4, 18, 36, 66; and
      maxDelayMs: 1920x1080 -> 500, 2560x1440 -> 500, 3840x2160 -> 250, 5120x2880 -> 100, 7680x4320 -> 0 (class 60);
      1920x1080 -> 500, 3840x2160 -> 100 (class 120)                          RED: M-H8 budget compares total instead of extra.
 U-H9 "frame history: the ring shrinks only after 600 publishes below the tier"  RED: M-H9 hysteresis 600 -> 0.
 U-H10 "frame history: a concurrent reader never sees a torn entry" [tsan]    RED: M-H10 stamp and word in one non-atomic struct.

UNIT -- tests/test_surface_pool.cpp (S1)
 U-P1 "surface pool: a new slot count is a new generation of that many surfaces"   RED: TREE-BEFORE.
 U-P2 "surface pool: retainSurface refuses a slot beyond its generation's count"   RED: M-P2 bound = kMaxSlots.
 U-P3 "surface pool: allocBytes is at least slots x w x h x 4 and at most 2 % more" RED: M-P3 counts kSlots.

UNIT, offscreen GL -- tests/test_shared_frame_gl.cpp, tag [shared_frame_gl] (S2, S3); frame n = a solid colour encoding n
 U-ID "output frame: a default look is presentSharedFrame, byte for byte" (same target bytes; `shown.serial == front().serial`;
      the pool stays at 4 slots)                                              RED: M-ID default look takes the pick path.
 U-G1 "output frame: delay d shows the frame offered d ago" (d = 100, 250, 500 ms; injected clock; decoded n exact)
                                                                              RED: M-G1 delay ignored.
 U-G2 "output frame: raise holds, lower jumps, never backwards" (decoded n non-decreasing over a 0 -> 500 -> 0 sweep)
                                                                              RED: M-H4.
 U-G3 "output frame: a size change reaches a delayed reader d later, never black" (every decoded frame valid; old size until
      then)                                                                   RED: M-G3 reader rebinds to front's generation.
 U-G4 "output frame: a deeper ring keeps what is being shown; slot rotation holds at 12, 21, 36 slots"
                                                                              RED: M-G4 write_ modulo kSlots.
 U-G5 "output look: the pass matches applyReference within 1/255" (7 looks x a 256-step ramp, 1:1 target)
                                                                              RED: M-G5 brightness before contrast.
 U-G6 "output look: opacity 0 is black; -1 on a channel removes it"           RED: M-G6 opacity multiplies alpha only.
 U-G7 "output look: the forced pass with a default look is byte-identical at 1:1" (bar 0); at a portrait target the largest
      per-channel difference is REPORTED against the bar 2/255                RED: M-G7 half-texel offset in the quad.
 U-G8 "output frame: an FBO target at canvas size (the Syphon shape) shows the delayed, coloured frame; the read framebuffer
      it was called with is bound again on return"                            RED: M-G8 leaves the target bound.

UNIT -- tests/test_output_look.cpp (S3)
 U-L1 "output look: pack / unpack round trip over every range edge"           RED: M-L1 opacity 6 bits.
 U-L2 "output look: applyReference(default) is the identity for all 256 values" RED: M-L2 contrast pivot 0.5 -> 0.498.
 U-L3 "output look: the table" (hand-computed rows: contrast -1 -> 0.5; brightness +1 -> 1; red -1 -> r = 0; opacity 50 -> c/2)
                                                                              RED: M-G5.
 U-L4 "output look: out-of-range input is clamped"                            RED: M-L4 no clamp.
 U-L5 "output look: isDefault is exact"                                       RED: M-L5 ignores delay.

UNIT -- tests/test_output_settings_store.cpp, test_app_settings.cpp, test_display_identity.cpp (S4)
 U-S1 "output settings: JSON round trip"                                      RED: TREE-BEFORE.
 U-S2 "output settings: a missing number reads as its default; unknown keys are ignored; no id = skipped"
                                                                              RED: M-S2 missing opacity reads 0.
 U-S3 "output settings: a write never changes the outputs key or any other key" RED: M-S3 store writes kOutputs.
 U-S4 "output settings: the 33rd screen drops the one seen longest ago"       RED: M-S4 drops the newest.
 U-S5 "output settings: changes within 500 ms make one write"                 RED: M-S5 debounce 0.
 U-S6 "output settings: a failed write stays dirty and is retried"            RED: M-S6 clears dirty before the write.
 U-S7 "output settings: flush writes at once"                                 RED: TREE-BEFORE.
 U-S8 "output settings: syphon has its own record"                            RED: M-S8 syphon stored under screens[0].
 U-A1 "app settings: an unreadable file is copied aside before it is rewritten" RED: TREE-BEFORE (no copy exists).
 U-D1 "display identity: joined by index, checked by bounds"                  RED: M-D1 no bounds check.
 U-D2 "display identity: an empty or duplicated uuid falls back to the size id" RED: M-D2 duplicate kept.
 U-D3 "display identity: DisplayInfo equality is unchanged" (a static check on the six fields)  RED: M-D3 a seventh field.

UNIT -- tests/test_output_settings_model.cpp, test_output_menu_model.cpp, test_output_law.cpp (S5, S6)
 U-M1 "output screens: one entry per connected display, in menu order, with the menu's label"  RED: TREE-BEFORE.
 U-M2 "output screens: Syphon is listed iff available, with its state"        RED: M-M2 listed always.
 U-M3 "output screens: a waiting output whose screen is absent is listed, dimmed; other remembered screens are not"
                                                                              RED: M-M3 lists every record.
 U-M4 "output screens: the Delay row's range is maxDelayMs; the shown value is min(stored, max)"  RED: M-M4 range fixed 500.
 U-M5 "output screens: seven sliders, their defaults 0, 100, 0, 0, 0, 0, 0"   RED: M-M5 opacity default 0.
 U-M6 "output screens: the model's paint key changes iff something painted changes" (Pitfall 59)  RED: M-M6 key includes a tick.
 U-MENU1 "output menu: the list ends with Output Screens..., always enabled; every earlier item is unchanged"
                                                                              RED: TREE-BEFORE.
 L-1 "output law: the settings window and the store name no opener" (`openDisplay(`, `toggleDisplay(`, `restoreLast(`,
     `openOnDisplay(`, `setAlwaysOnTop(true)` absent from OutputSettingsWindow.*, OutputSettingsModel.h, OutputSettingsStore.*)
                                                                              RED: a mutant line calling openDisplay(0).
 L-2 "output law: the new 8080 routes name no opener"                         RED: likewise in the route body.
 L-3 "output law: presentOutputFrame's default branch is presentSharedFrame"  RED: M-ID.
 L-4 "output law: Renderer.cpp calls presentOutputFrame exactly once, inside publishSyphonFrame"  RED: a second call.
 L-5 "output law: presentCanvas, processPendingCapture and VideoRecorder.cpp name no output setting"  RED: a mutant reference.

LIVE ROWS (Harmony; .harmony/probe-outputs.sh, test mode, `set_output_tap`, a scratch settings file, never a window; each row
prints `<name> PASS|FAIL <numbers>`; `o_no_window_opened` is evaluated after every row)
 oa_publish_rate      INFO  "publish_hz=<x> class=<60|120>" (G0, on main).
 oa_identity_default  BAR: probe PNG bytes == canvas capture bytes (the existing o_probe_matches_canvas bar);
                      `history.slots == 4`; `shown_serial == frame_serial` in 30 of 30 probes.
                      RED arm: the S5 tree with the probe's look set to Delay 1 (slots != 4, serial differs).
 oa_delay_age         BAR: for d in 100, 250, 500 (1080p): after d + 500 ms of warm-up, 30 probes 100 ms apart;
                      |shown_age_ms - d| <= one publish interval + 2 ms in at least 29 of 30.
                      RED arm: M-G1 build (age stays under one interval).
 oa_delay_monotonic   BAR: Delay swept 0 -> 500 -> 0 in 10 ms steps, one probe per step: `shown_serial` never decreases
                      (0 decreases in 101 probes). RED arm: M-H4 build.
 oa_colour_pixels     BAR: 7 looks on a ramp source, 1:1 probe target: every pixel within 1/255 of `applyReference`.
                      RED arm: M-G5 build.
 oa_syphon_look       BAR: Syphon on, its look = V4's values: `syphon_probe` PNG within 1/255 of `applyReference` of the canvas
                      capture; with the look default: byte-identical to the canvas capture. RED arm: TREE-BEFORE (route 404).
 oa_canvas_untouched  BAR: probe look and Syphon look = {Delay 500, Brightness -1.00, Opacity 0}: `render_frame` PNG sha256 ==
                      the all-default baseline's (same time override), AND the probe PNG differs from it (positive control).
                      RED arm: a build whose Syphon branch does not re-bind `canvasFBO_` on return.
 oa_memory            BAR: canvas 1920x1080: Delay 0 / 100 / 250 / 500 -> slots 4 / 12 / 21 / 36 (class 60; the class-120 column
                      of U-H8 if G0 says 120) and `max_delay_ms` 500; canvas 3840x2160: slots 4 / 12 / 21 / 21 and
                      `max_delay_ms` 250; every `alloc_bytes` within [slots x w x h x 4, +2 %]; after Delay -> 0, slots == 4
                      within 15 s; after the tap goes off, `alloc_bytes` == the 4-slot figure within 1 s of the next frame.
                      RED arm: M-H8 build; and TREE-BEFORE (field absent).
 oa_tier_spike        BAR: `peak_frame_time_ms` over the 2 s around each depth change <= 16.6 ms at 1920x1080 and at 3840x2160
                      (5 changes each). Not met -> REPORTED, and the fallback of OA1 (off-thread prepare + lazy binding) is
                      scheduled; the bar stays. RED arm: a build that sleeps 20 ms in `ensure` (proves the row can fail).
 oa_tap_cost          BAR: the existing o_tap_cost measurement at Delay 500 <= its value at Delay 0 + 0.20 ms (same canvas).
                      RED arm: a build with a second blit per publish.
 oa_settings_roundtrip BAR: `output_settings` sets a look for the first display's id and for "syphon"; 1 s later the scratch
                      file's "outputSettings" holds exactly those numbers and its "outputs" value is byte-identical to before;
                      after a relaunch on the same file `/api/state` reports the same numbers; `outputs.live == 0` throughout.
                      RED arm: M-S3 build.
 oa_tsan              BAR: the [tsan] cases and probe-tsan's output scenario report 0 races. RED arm: M-H10 build.
 REGRESSION           every existing o_* row of probe-outputs and the eight existing test_shared_frame_gl cases: unchanged PASS.

VISUAL GATE: V1..V13 (OA5), capture builder then five critic seats, before Boris.

---------------------------------------------------------------------------------------------------
## 6 WHAT ONLY BORIS CAN CHECK (do -> expect -> what wrong looks like; his real screens, his ear)
---------------------------------------------------------------------------------------------------
B-1 The Delay is real. Do: a projector or second monitor on; a clip that flashes on the beat; open Output > "Output Screens...",
    pick that screen, set Delay 250 ms; film the laptop's preview and the screen together with a phone in slow motion.
    Expect: the screen flashes a quarter of a second after the preview, steadily. Wrong: no gap, a gap that wanders, or a flash
    that appears twice.
B-2 Delay 0 is today. Do: Delay 0, everything else untouched. Expect: the screen looks and feels exactly as before this change.
    Wrong: any softness, any lag you did not have, any tint.
B-3 Dragging. Do: drag Delay up slowly, then down, while a moving clip plays. Expect: on the way up the picture pauses a little
    and carries on; on the way down it skips ahead; never black, never a replay of what just happened. Wrong: a black flash, a
    visible jump backwards, a long stutter that does not stop.
B-4 The right screen is remembered. Do: set screen A to 100 ms and screen B to 300 ms; quit; start again; then unplug and
    re-plug each; then (if you have two of the same projector) swap their cables. Expect: each physical screen comes back with
    its own number. Wrong: the numbers are swapped or back at 0 -- tell Harmony which step did it (this is the one thing no test
    here can try; with two identical projectors the numbers may follow the socket, not the box).
B-5 The colours, against Resolume. Do: the same still on one screen from Resolume and then from Audio-DNA; in both set
    Brightness -0.3, then Contrast +0.3, then Red -0.3. Expect: the same kind of change in both. Wrong: one darkens the blacks
    and the other does not, or a row runs the other way -- say which row (Resolume does not publish its formulas; ours are the
    usual ones).
B-6 Opacity. Do: drag Opacity to 0 on one screen with two screens on. Expect: that screen fades to black, the other and the
    preview do not change. Wrong: the preview or the other screen dims too.
B-7 Syphon. Do: turn Syphon on, receive it in Resolume; set its Delay to 500 ms and its Blue to -1. Expect: Resolume's copy is
    half a second late and has no blue; Audio-DNA's own preview, a recording and a snapshot are normal. Wrong: the recording
    or the snapshot is late or tinted.
B-8 In the room. Do: stand where you mix; play a track with a hard kick; raise that screen's Delay until the flash and the kick
    land together for your eye and ear. Expect: one number does it and it holds all night. Wrong: you run out of range (say how
    far the screen and the speakers are), or it drifts.
B-9 4K. Do: set the composition to 3840x2160 and look at the Delay row. Expect: it stops at 250 ms. Wrong: the app slows down or
    the machine runs out of memory at any setting.

---------------------------------------------------------------------------------------------------
## 7 QUESTIONS FOR BORIS (new numbers; each has a default A; nothing waits on them)
---------------------------------------------------------------------------------------------------
51. The "Device" row in each screen's settings.
    A (default) It only names the screen (for example "Display 2 (1920x1080) LG HDR 4K"). You switch screens on and off in the
      Outputs list, as today.
    B It is a menu: picking another screen there moves this output to that screen.
52. Opacity on Syphon.
    A (default) It fades the picture to black, the same as on a screen.
    B It makes the picture see-through for the app that receives it.
53. When you raise a screen's Delay while a clip is playing.
    A (default) That screen's picture holds still for the time you added, then carries on.
    B It slows down gently until it has fallen back by that much.
54. Should a screen's Opacity (or any of these settings) be something you can put on a key, a knob or a MIDI pad?
    A (default) No. They are set once per room in the window.
    B Yes. Say which ones.
(55-59 not used.)
Readings Harmony tells him (not questions): the Delay runs 0 to 500 ms; on a 4K composition it stops at 250 ms because of video
memory; the picture moves one frame at a time (about 17 ms); Brightness, Contrast, Red, Green, Blue run from -1.00 to 1.00 with
0 in the middle and a right-click puts any row back; the settings of a screen that is not plugged in are kept; the window is
called "Output Screens" because "Output Settings" is already the composition's size.

---------------------------------------------------------------------------------------------------
## 8 RISKS (the strongest counterargument first; the cheapest refuting test for each choice)
---------------------------------------------------------------------------------------------------
R1 THE STRONGEST COUNTERARGUMENT: "Copy Resolume exactly -- 0 to 100 ms -- and none of the tier / budget / hysteresis / publish
   class machinery is needed: a fixed 12-slot ring (95 MiB at 1080p, 380 MiB at 4K), always the same depth." It is simpler, and
   at 343 m/s 100 ms already covers 34 m of air (FR Q8, INFERRED arithmetic), less whatever the projector itself adds. Why it
   loses HERE: Boris was told 0..500 (reading R11) and did not correct it; a festival desk at 50-60 m needs 150-175 ms; and a
   fixed 12 slots would break the Delay-0 constraint (today's 4 slots, today's memory) unless a "0 = shallow" mode exists
   anyway -- which is most of the machinery. What survives of it: if the council prefers, dropping the 500 tier (range 0..250,
   two tiers, `kMaxSlots = 36`) is a change of three constants in FrameHistory.h and one table row in U-H8 / oa_memory.
   Cheapest refuting test of the need: Boris's check B-8 in his usual room.
R2 The publish class. If this Mac's canvas really runs at 120 fps the ring needs twice the slots and 4K is limited to 100 ms.
   INFERRED from F7; unmeasured. Cheapest test: G0, on main, today, no code.
R3 Creating up to 36 (66) IOSurfaces and binding them on the GL thread at a tier change may hitch a frame. ASSUMED cheap
   (surfaces are lazily backed; binding copies nothing). Cheapest test: oa_tier_spike. The fallback is named in OA1.
R4 A picked slot being rewritten under a reader in another context. The 2-publish guard is the existing rule (F6) applied to
   the oldest pick instead of to front; all contexts are on one thread (F7). Cheapest test: U-H6 + U-G4 + oa_delay_monotonic
   under TSan.
R5 Cadence. A time-based pick could beat against the display link and show uneven steps. The property pick(t, d) = pick(t - d,
   0) says a delayed output steps exactly as today's output did d ago, provided no frame in between was lost from the ring
   (the 2 slots of slack). Cheapest test: U-H2; then Boris's B-1 (a wandering gap).
R6 The display UUID may not be as stable as ASSUMED (two identical projectors; a dock). Cheapest test: B-4. Cost of being
   wrong: a screen shows another screen's numbers; visible in the window, fixed by a drag; nothing crashes, nothing is lost.
R7 Our colour arithmetic is not Resolume's (not documented, F19). Cheapest test: B-5. Cost of being wrong: one formula line in
   OutputLook.h and the shader, one table in U-L3.
R8 The blit-to-quad switch when a colour row first leaves default may shift the picture by a fraction of a pixel on a scaled
   screen. Cheapest test: U-G7's reported number. If it exceeds 2/255 it is REPORTED and the choice becomes: always use the
   quad for a scaled target (which would touch the Delay-0 identity and needs Harmony's ruling).
R9 Syphon's one-frame step when its first setting leaves default (OA3). Stated; visible only to a receiver comparing frames.
R10 settings.json: the mend keeps a copy but does not make the file transactional; two launches of the app writing at once are
   still last-writer-wins (F13). Not made worse by this lane.
R11 The settings window can sit under an output on a one-display setup (the output covers the main screen). The existing
   Cmd+` raises the main window, not this one. Not solved here; named in section 9.
R12 `kMaxSlots = 66` fixed arrays in every reader state and in the writer: about 1.6 KiB each, no heap. Negligible; named
   because it changes `PresenterGLState`'s size.

NOT VERIFIED (could not be established by reading):
 N1 What the display UUID survives (re-plug, mode change, reboot, identical projectors, a dock) -- FO U3.
 N2 The canvas publish rate on Boris's Mac (60 or 120) -- G0.
 N3 The cost of creating / binding a deep generation on the GL thread; whether IOSurface memory is lazily committed.
 N4 The real frames-behind of an output and whether a display-sized window at swap interval 0 tears (FO U1, U2) -- unchanged by
    this lane, still unmeasured.
 N5 Whether `glBlitFramebuffer` (GL_LINEAR) and a rect-texture quad agree to 2/255 at a scaled target on this GPU.
 N6 Resolume's own arithmetic, displayed number scale, and whether Arena 7.22.9 still has no Delay row for Syphon (FR section 3).
 N7 That src/render/EmbeddedShaders.h can be included by the test target that compiles OutputPresenter.cpp without pulling the
    renderer in (INFERRED: it is a header of strings); if not, the string lives in OutputPresenter.cpp and the docs say so.
 N8 Whether the Syphon server accepts a texture whose content came from a rect-texture quad pass exactly as from a blit
    (INFERRED: it is the same `syphonTexture_`, GL_TEXTURE_2D, F12).
 N9 tests/test_output_law.cpp's exact token lists for MainComponent (read through FO Q8, not re-read line by line here): the
    S5 / S6 builder must run it before adding the window's owner to MainComponent.

---------------------------------------------------------------------------------------------------
## 9 WHAT IS NOT IN THIS LANE
---------------------------------------------------------------------------------------------------
- Any delay of AUDIO, of analysis, or of the beat; the "off beat by" nudge (the beat-nudge lane); Tap / Resync.
- Moving "Restore Last Outputs" and the hot-plug ladder from the geometry fingerprint to UUIDs (OA4 keeps them as they are).
- A Device menu that moves an output (unless question 51 = B); Resolume's slices, masks, warping, edge blend, test card,
  identify-displays (the dead ids `kOutputIdentifyDisplays` / `kOutputTestCard` stay dead).
- Saved output SETUPS / venue presets (Resolume's preset dropdown); a "forget this screen" action; an export of the settings.
- A production (7070) setter, OSC, MIDI, key bindings, mappings or take recording of an output's settings (unless 54 = B).
- Remembering Syphon's on / off between launches (it stays OFF at each launch, F12).
- NDI, capture-card or virtual outputs.
- Removing the dead `outputDisplay` field from the show and deck files (saves lane).
- Making settings.json transactional; the failed-write retry of the `outputs` key (F14).
- A way to raise the settings window above an output that covers the only display (R11).
- A measured glass-to-glass latency of an output, and the tearing question (FO U1, U2).
- Windows / Linux: outputs stay macOS-only (the #else branches are untouched).
CARRIED from the stopped sync-dial branches (retyped, OA8): testing-eyes.md's kill advice (dc59573); architecture.md's seqlock
line (from f14eb31) plus :231 and :334; the APP-INVENTORY recount (instead of f4a7e34); the debounced-save idea.
DROPPED with them: everything else on lane/bf2 (740b6d6) and lane/bf2-keys (9eab9bd) -- the commits listed in OA8. The music-beat
wheel, the 140 px Gain, the learn title's em dash and the dial-free overlay refactors of df98f78 are named for other lanes.

STATUS: DONE

---------------------------------------------------------------------------------------------------
## HARMONY ADOPTION (2026-10-04 13:43:25, session s-rta-1004)
ADOPTED IN FULL: .harmony/.reports/s-rta-1004/ruling-outputs.md (status DONE; 31 attacks ruled: 21 ACCEPT, 9 PARTIAL, 1 REJECT;
17 amendments, each OVERRIDES this plan's body). Precedence for every stage, review and gate of lane "outputs":
Boris's verbatim words (binding-decisions.md, the 2026-10-04 sections) > this adoption > ruling-outputs.md > this plan's body.
Workflow run wf_13e17572-2fd (plan: architect opus high; seats gl-thread 7 attacks / 3 MUST, gates 8 / 6, stage-hands 8 / 2,
scope 8 / 1 -- papers whole in attack-outputs-papers.md; ruling: architect opus max).
What I read myself before adopting: the ruling's section 0 (verdict), 6 (what only Boris can check), 7 (questions and
readings), and its returned stage list, decisions and measurements. NOT read by me: sections 1-5 (facts re-derived, the attack
table, the amendments' full text, the gate rows) -- they are the builders' and reviewers' spec and are read by them in full;
a gate string is copied only from section 5.
THE RANGE. The ruling overrules my reading R11 (0 to 500 ms) with Resolume's 0 to 100 ms, on Boris's "copy what resolume does
... Makes it simpler", and puts the range to him as question 51 (B = 0 to 250). ACCEPTED: R11 was mine, not his, and said the
maximum was still being looked up. The counter-argument (a festival desk 50-60 m out needs 150-175 ms of air) is his to weigh;
either answer is three constants (A-1).
HARMONY'S DECISIONS (the ruling's section 8):
HD-1  default: build 0..100 while question 51 is open.
HD-2  default: two worktrees (A: S1, S2, S3, then S5; B: S4, S6), one builder each, never two builders in one worktree.
HD-3  default: the lane does not merge until oa_canvas_untouched and oa_syphon_look ran on a build WITH Syphon. Lane builds
      are configured -DAUDIODNA_BUILD_SYPHON=ON as before; M2 confirms it on the rig.
HD-4  default: the two stale-doc fixes (testing-eyes.md:72; architecture.md:107 / :231 / :334) are a docs chore of their own
      after this lane, outside S7.
HD-5  default: the settings.json mend (an unreadable file read as empty) goes to the one-save lane (finding SF-1).
HD-6  Pitfall 68.
HD-7  default: kSizingHz 120; raised before S1 only if M1 reads above 125 publishes a second.
HD-8  default: bars met = eager binding stands; missed = S2b and / or S1b. The bars do not move.
HD-9  default: bar met = one colour pass; missed = the two-step path (+1 canvas frame per coloured output), told to Boris.
HD-10 default: after Boris's check B-0 -- sliders drag and keys keep working = the never-key window stays; else a normal
      window with MainComponent as KeyListener (a fix stage, re-gated).
HD-11 I re-witness every mutant a gate line names; no extra M-H4 app build in oa_delay_live.
HD-12 default: the read-only output state fields on 7070 and 8080.
HD-13 default: nothing until a changing UUID is seen (M4 / B-4); then the edid key.
HD-14 default: against the beat-nudge lane, the second lane to merge rebases (a builder's step 0).
HD-15 CHANGED: the stopped bf2 / bf2keys worktrees are removed only after the beat-nudge ruling is adopted too (it may carry
      parts of lane/bf2-keys) and after HD-4's lines are copied. The keying worktree is not part of this.
HD-16 The section 7 readings are told to Boris with question 51 as R45..R56 (boris-clarify-51.md).
FACTS I MEASURE (M1..M8) and Boris's own checks (B-0..B-10): as the ruling's section 8 and section 6 state them.
ORDER: G0 (mine, on main, no code) -> S1 -> S2 -> S3 beside S4 -> S6 (visual gate V1..V9) -> merge B into A -> S5 -> my gate
list -> S7 docs -> merge to main. NOT STARTED in this session: nothing of this lane is built.

## HARMONY ADOPTION, UPDATE ON BORIS'S ANSWER (2026-10-04 15:35:01)
Boris, verbatim (binding-decisions.md): "51 default is good" -> question 51 = A: the Delay runs 0 to 100 ms. HD-1 is closed
(the range built is the range ruled). Readings R45..R56 were told to him with the question and not corrected.
