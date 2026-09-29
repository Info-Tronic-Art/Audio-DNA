STATUS: DONE_WITH_CONCERNS

# lane idlepaint (s-rta-0928b) -- the idle whole-window repaint

WT = /Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/rta0928b-idlepaint, branch lane/idlepaint
S = /private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/e9ff9dc6-f159-4d37-b0a9-871bf522d258/scratchpad/idlepaint
Started from main 328301d; rebased twice (onto the mediaopen merge a56f835, then onto the video merge 5c5f21d). Lane head = the commit carrying this report.

RESULT: The idle UI gate passes on the final build. The window-max stall went from 21.8 to 4.5 ms and main-thread CPU from 347 to 135 ms/s on the card fixture. On the 16 x 4K deck: 16.4 -> 4.2 ms and 500 -> 111 ms/s. The two concerns are real and not fixed:
(1) With a routine playing on 3 layers (adoption I3's g4), the window max passes (19.3 -> 5.4 ms) but CPU is 163 ms/s against a 150 limit. I stopped there as I3 rules (design finding).
(2) The pixel-identity rows as the plan wrote them (array_equal, <= 4 corner clusters at <= 2/255) FAIL. Every difference is 1/255 at anti-aliased pixels, apart from one INFO class described in NUANCE.

FACTS:
- Gate rows, all `.harmony/probe-idle-paint.sh`. Logs: `S/runs/{G2,F2,F3,F4,R1,R2,R3,R4}.log`. The table in section GREEN / RED below has the numbers.
- ctest full serial: `ctest --test-dir WT/build-lane -j1` -> "100% tests passed, 0 tests failed out of 920" (`S/ctest-final2.log`).
- The overlay switch (adoptions I1 / I4) was measured in vblanks with a real parented PopupMenu plus a test panel, while g4's routine played:
  - 0 vblanks with a layer covering an overlay, across 10 fallbacks (`S/runs/G2.log`).
  - The layer returns <= 2 vblanks after the overlay closes. The final run's max was 2; one earlier run on the pre-5db815f binary had max 3 (`S/runs/F6.log`).
  - Teeth: the plan body's late hide reads 11-12 covered vblanks.
- Screen Recording works: the step-0 capture of window 15056 was 3456x2158 with 3033 colours (`S/step0/`).

METHOD: The plan (A)-(E) as adopted, plus the rulings I1-I11:
- LayerStrip: F4 + I2 + I3.
- NativeLayerCache + OverlayWatch (pure), NativeLayerHost (ObjC++) for SignalBar and WaveformDisplay.
- ClipInspector: F6.
- TEST-ONLY counters and routes, and the probe.
Every new probe row and ctest went RED first against the pre-change app or tree. Perf numbers come only from launches with no compiler running; any compiler taints the launch and it is re-run. The load average is printed on every launch. The attribution of the one regression I introduced used an interleaved A/B across three apps (`S/ab.sh`).

CONFIDENCE+VERIFY: High on the gate numbers (5 launches per arm, stable spreads) and on the overlay switch (a vblank witness plus teeth). Medium on identity: the causes of the 1/255 differences are INFERRED (see NUANCE). Re-prove with `LANE=idlepaint . S/../lib/lock.sh`, then `IDLEPAINT_APP=<app> IDLEPAINT_APP_BEFORE=<main app> bash WT/.harmony/probe-idle-paint.sh <out> <rows>`.

UNKNOWNS / NOT DONE:
- g4 CPU 163 > 150: stopped per I3.
- The v1 / v1b / v2 strict identity criteria: reported, not re-thresholded.
- Why painting MainComponent's own paint() inside the waveform layer cost +60 ms/s (measured, then reverted).
- Why a whole-MainComponent repaint re-renders text with different glyph edges.
- The GL-side writers of Clip::playheadPosition still write it plainly (s166 L5's open item).

NUANCE:
- Removing the union exposes a pre-existing JUCE/AppKit behaviour. A pass that repaints the WHOLE MainComponent (POST ui_repaint_all; in real use a resize or leaving binding mode) re-renders text that JUCE had not repainted since startup, with different glyph edges: 36k px, up to 91/255. It is the same in both builds. On main the 30 Hz union overwrote it within a frame; on the fixed build it stays until something repaints that region.
- v1 S4 is therefore scoped to the waveform (the thing I7 asked about): 0 px differ. The whole-window count is an INFO line.

HANDOFF-NEEDS: Harmony rulings on
(a) g4's CPU finding: accept 163, or order the plan's lever -- per-strip SignalBar repaints, then fold the band / V-fader repaints;
(b) the identity tolerance: every remaining BEFORE/AFTER and native/in-peer difference is at most 1/255 at anti-aliased pixels, and BEFORE vs BEFORE itself reads 33 px at 1/255 in one of two runs;
(c) whether the full-pass text re-rendering needs its own lane.
The critic panel seat (I12) and Boris's LOOK list (plan 3.4) come next.

INBOX-RECHECK: none

---

## SUMMARY

At idle the message thread no longer repaints the window. On the final build:
- SignalBar and WaveformDisplay draw in their own layer-backed NSViews, 29/s each.
- The LayerStrips and ClipInspector repaint only on change: 0/s at idle.
- The TopBar's 15 Hz wheel is the only in-peer periodic pass: MainComponent paints 14.9/s, and every one is a TopBar-only pass.
- Idle card: stall 21.8 -> 4.5 ms, main-thread CPU 347 -> 135 ms/s.
- 16 x 4K images: 16.4 -> 4.2 ms, 500 -> 111 ms/s.
- The same binary with its layers switched off (x1) reads like main (22.0 ms / 346 ms/s), so the layers are the mechanism.

An in-peer overlay over a native panel switches that panel to JUCE painting synchronously: no covered vblank in 10 cycles under a playing routine. The layer is shown again only after it has drawn.

## GREEN / RED (medians over 5 launches unless noted; load average 3-6 throughout, printed per launch in the logs)

| row | main 5c5f21d (RED) | final build | verdict |
|---|---|---|---|
| i1 IDLE-HB-card (<= 8 ms AND <= 150 ms/s) | 21.8 ms [21.6-22.1] / 347.0 ms/s [345.1-348.8] (R1) | 4.5 ms [3.9-5.1] / 134.9 ms/s [131.5-136.2] (G2) | RED -> GREEN |
| i2 IDLE-HB-many16 | 16.4 [16.3-16.5] / 499.6 [492.0-502.6] (R2) | 4.2 [4.0-5.3] / 110.9 [109.3-118.6] (F2*) | RED -> GREEN |
| g4 a loop routine on 3 layers (I3, same limits) | 19.3 [18.9-20.1] / 351.8 [350.0-357.4] (R3) | 5.4 [5.3-5.7] / 162.6 [157.5-174.5] (F3*) | window max PASS, **CPU FAIL -- stopped per I3** |
| g5 manual 120 BPM + live input (REPORT, I6) | 22.0 / 345.8 (R4) | 4.6 / 133.5 (F4*) | INFO |
| x1 same binary, ADNA_UI_NATIVE_LAYERS=0 (3 launches) | -- | 22.0 / 345.7 (F6*) | INFO: the layers are the mechanism |
| g1 rates (rides on i1) | counters absent -> FAIL | waveform 29.1-29.2/s, signal bar 29.1-29.2/s, TopBar 14.9/s, modes native, 0 fallbacks | RED -> GREEN |
| g2 strip playhead + I2 | absent | 146 repaints / 5 s playing; painted advances 146 / 146 ticks (1.000); 0 at idle (F1*) | RED -> GREEN |
| g3 (INFO) | MainComponent 39-40 paints/s (c0 app) | 14.9/s (TopBar-only passes), ClipInspector 0.00/s | -- |
| c0 preflight | peer_layer_backed absent | heartbeat ok, peer_layer_backed 1 (I8), capture ok | -- |

\* F-rows ran on the binary one commit before 5db815f (`S/app-f`). 5db815f changes only the moment a layer returns from a fallback: one extra peer pass per overlay close, and one at startup. i1 was re-run on the final binary (G2).

Earlier RED on the commit-0 build (pre-rebase, with heartbeat and counters only): i1 17.2 ms / 323.2 ms/s, i2 16.7 / 507.7 (`S/runs/C-i1-red-c0.log`, `E`).

g4 detail: band hairline repaints 28.7/s (3 strips x ~9.6/s). MainComponent paints 27/s, against 14.9/s at idle: the routine's hairlines and V faders repaint in-peer.

## Pixel identity (window-only captures of window id by Quartz, 2x, decoded with PIL + numpy)

| row | result |
|---|---|
| v0 teeth (1-px shift of the layers) | PASS: 77 clusters in the SignalBar, 2 in the waveform, 0 elsewhere (F5) |
| v1 BEFORE (main) vs AFTER, test mode | FAIL under the plan's rule, max delta **1** in every state (G1): S1 1251 px / 360 clusters (LayerStrip controls painted once at startup vs main's continuous repaints), S2 77 px, S3 91 px (SignalStrip top corners, waveform corners, deck-cell pixels) |
| v1 S4 (I7): waveform rect + 8 px after a whole-MainComponent pass | PASS: 0 px |
| v1 S4 whole window (INFO) | 36,113 px: text re-rendered by the whole pass (NUANCE) |
| v1n noise floor, BEFORE vs BEFORE | S1 0 px, S2 33 px (19 clusters at 1/255), S3 0 px; the previous run gave 0 / 0 / 0 |
| v1b native vs forced in-peer, frozen driven state (TEMPORARY waveform hook) | FAIL under the plan's rule, max delta 1: SignalBar 8 px (the 6 SignalStrip top corners), waveform 28 px (its 4 corners in 12 small clusters). Native after the round trip == native before: 0 px, both panels (G3) |
| v2 panel over the bar | mode 2 within 1 s, waveform stays native, A == C exactly, 0 covered vblanks. FAIL: A vs B has 8 px (the strip corners, 1/255) outside the overlay's rect |
| v2 real parented PopupMenu | PASS: it falls back and returns (the menu auto-dismisses in a background app), 0 covered vblanks |
| v2b (I1 / I4) under g4's routine | PASS in G2: 0 covered, restore max 2, teeth 11. FAIL in F6 (pre-5db815f binary): restore max 3 once in 10 cycles |
| v3 production, masked | PASS: 0 px (G1). F6 on the pre-5db815f binary read 33 px at 1/255, the same deck-cell pixels as the noise floor |

The strongest counter-reading is that the layers change what users see. Against that:
- Native vs in-peer at the same frozen state differs by at most 1/255 on 36 px across both panels.
- The rest of the window matches main at 1/255.
- The one large class (the whole-pass text) is AppKit/JUCE behaviour that main also shows, for one frame, after such a pass.

## FILES CHANGED

- `src/ui/UiPaintCounters.h` (new): relaxed-atomic witnesses plus the panel geometry (TEST_SERVER).
- `src/ui/NativeLayerCache.h` (new): the CachedComponentImage.
  - Native: swallow the repaint and mark the layer.
  - Fallback: transparent.
  - RestorePending: show the layer only after it drew; the peer then repaints beneath a non-opaque widget.
  - FallbackPending: teeth only.
- `src/ui/OverlayWatch.{h,cpp}` (new): tooltip, post-baseline root children, explicit overlays, top-level children. A child that was removed while not showing is ignored.
- `src/ui/NativeLayerHost.{h,mm}` (new):
  - NSViewComponent + ADNANativeLayerView, using the peer's layer set-up, hitTest nil, never key, no accessibility element.
  - Hides by alpha, synchronously, and hands the widget area straight to AppKit.
  - Env switches ADNA_UI_NATIVE_LAYERS=0, ..._TEETH=shift|asynchide, ADNA_UI_OVERLAY_WITNESS=1 (VBlankAttachment), all TEST_SERVER only.
- `src/ui/LayerStrip.{h,cpp}`:
  - transportViewOf + timerTick; the transport is repainted on change, read once per update (std::atomic_ref, I2).
  - The band hairline is repainted when its width changes (I3); the per-tick clip-name repaint is gone.
- `src/ui/ClipInspector.{h,cpp}`: PaintKey / paintKeyNow; refresh() repaints on change (F6).
- `src/MainComponent.{h,cpp}`:
  - OverlayWatch and the two hosts at the end of the ctor; SignalBar setOpaque(true).
  - A paint counter; recordUiGeometry.
  - TEST-ONLY hooks: test menu / panel, forced fallback, whole repaint.
- `src/ui/TopBar.cpp`: one include and one counter line.
- `src/api/ApiServer.{h,cpp}`: GET /api/debug/ui_paint; POST ui_test_menu {on,x,y,kind}, ui_native_fallback, ui_repaint_all. TEST-ONLY. The heartbeat is mediaopen's, kept from main at the rebase (I9).
- `CMakeLists.txt`: NativeLayerHost.mm in if(APPLE); the ui sources.
- `tests/`:
  - test_native_layer_cache (8 cases)
  - test_overlay_watch (6)
  - test_layer_strip_transport_view (6)
  - test_clip_inspector_paint_key (3)
  - `tests/CMakeLists.txt` EOF blocks
- `.harmony/probe-idle-paint.{sh,py,json}`: 14 rows.
- Docs:
  - `docs/claude/pitfalls.md` NN
  - `CLAUDE.md`: index NN + a UI Patterns line, 24,980 B, paid by compressing the TASKPLAN_V2 and ARCHITECTURE notes
  - `docs/claude/architecture.md`: the tree line + "UI Painting (macOS)"
  - `.harmony/APP-INVENTORY.md`: the TEST-ONLY paragraph

Commits (lane/idlepaint on main 5c5f21d):
- 65954cd probe + counters + TEST-ONLY routes
- ff3e399 LayerStrip F4 / I2 / I3
- b56bf10 NativeLayerCache + OverlayWatch + ctests
- 4c57c82 NativeLayerHost + wiring
- c132312 ClipInspector F6
- 066675b parent-paint experiment
- 296438b probe comparator corrections
- 344732d revert of 066675b (+60 ms/s)
- 1bbb952 probe launches override
- a2bc015 docs
- 5db815f repaint beneath a returning non-opaque layer
- 0ef74c5 docs follow-up
- plus this report

## TESTS

- RED first:
  - test 3 against 5e1747b: "no member named 'transportViewOf'".
  - Tests 1-2: the headers are absent at 851b4b1.
  - Test 4 against 2a4a416: "no member named 'paintKeyNow'".
  - The new 5db815f assertion fails by construction on the old cache: the log ended "S 1".
  - Probe rows: RED on main and on the commit-0 app (table above).
- GREEN: every ctest target.
- Full serial ctest after the last code change: 920/920 (`S/ctest-final2.log`).
- The existing test_layer_strip_follows_model keeps passing (Pitfall 41).
- TEMPORARY hook (v1b): `ADNA_TEMP_WAVE_FREEZE` in WaveformDisplay::timerCallback. It was built into a copy (`S/app-hook`), then reverted and rebuilt. `strings` on the build-lane app gives 0, and `git status` shows it clean.

## ISSUES

1. g4 CPU 163 > 150 (I3: STOP, design finding). The plan's own levers (3.3), in order:
   - the SignalBar repaints only the strips whose displayed value changed;
   - the routine's band / V-fader repaints (in-peer, 3 strips) stop joining the TopBar pass union.
   Not attempted.
2. The identity rows' strict criteria FAIL at 1/255 (table). Classes:
   - (a) SignalStrip top-corner and waveform-corner anti-aliasing, layer vs peer.
     I tried and measured (no effect): drawing in the peer's user space, and excluding later opaque siblings.
     Painting the parent under the waveform fixed its corners but cost +60 ms/s (A/B: 128-134 vs 185-197 ms/s), so it was reverted.
   - (b) LayerStrip controls painted once at startup vs main's continuous repaints.
   - (c) deck-cell 1/255 noise, which also appears BEFORE vs BEFORE.
3. After a whole-MainComponent repaint, text that JUCE had not repainted since startup is re-rendered with different glyph edges. It persists on the fixed build: 36k px, up to 91/255 at glyph edges; visually a hair thinner (`S/exp1-014023-after/pair-files.png`).
4. v2b restore latency: max 2 vblanks in the final run, and 3 in one of 10 cycles on the previous binary. I4 asks for <= 2.
5. Rig deviations:
   - I once queued a second smoke launch by mistake and killed it while it was still waiting for the lock; it never held the lock.
   - I rewrote the lane history twice. After the first rebase a tests/CMakeLists.txt resolution was interleaved, so every commit was re-created with main + its own blocks. Commit 0's message was reworded because main already carried the heartbeat. Nothing was pushed.
   - The graphify post-commit hook runs a python rebuild after each commit; I kept perf batches away from commits after noticing.
   - The PopupMenu kind: a background app's PopupMenu is dismissed within ~50 ms (juce_PopupMenu.cpp MouseSourceState::checkButtonState). ui_test_menu therefore gained kind "panel" for the captures, and the real menu is exercised by counters.
   - I1's "hidden" is alpha 0 rather than setHidden: a hidden NSView does not draw, and I1 needs the layer to draw before it shows.
   - The work log (`.harmony/s-rta-0928b-work.md`) is not edited here. Its rows are below, for Harmony to append.

## RISKS

- The OverlayWatch baseline: a legitimate MainComponent child added after startup that crosses a panel would keep that panel in fallback. That costs perf, not pixels; g1 checks modes 0 at idle.
- An in-peer overlay type the watch does not see would be covered by a panel layer (plan R2).
- An SSR-like caveat: none (desktop app). The live behavioural gate is the probe plus Boris's LOOK: menus over the signal bar, a tooltip over the bar, a clip drag over the waveform, a window resize, the binding overlay.

## METRICS

- About 70 app launches across 25 lock holds, each hold <= ~12 min.
- Builds on -j3.
- The lane took about 4 h of wall time.

## KNOWLEDGE CONTEXT

- Tools used: grep plus direct source reads, including JUCE 8.0.4.
- Impact authority: grep (conservative). Nothing was deleted.
- God nodes in scope: MainComponent (ctor tail, paint, resized). Risk: ELEVATED (cross-cutting UI).

## Work-log rows (for Harmony)

- 65954cd..0ef74c5 idlepaint: IDLE-HB-card RED 21.8 ms / 347 ms/s -> GREEN 4.5 / 135.
- many16 16.4 / 500 -> 4.2 / 111.
- g4 19.3 / 352 -> 5.4 / 163 (CPU FAIL, stopped per I3).
- g2 PASS; v0 / v2b / v3 PASS.
- v1 / v1b / v2 strict identity FAIL at 1/255.
- ctest 920/920.

## Notebook lines (for Harmony to append)

- `## 2026-09-29 a background JUCE PopupMenu dies in ~50 ms | a menu shown while the app is not frontmost is dismissed by MouseSourceState::checkButtonState (no JUCE comp has focus); a probe needs a plain test component for captures | discovered: juce_PopupMenu.cpp:1438-1446`
- `## 2026-09-29 removing JUCE's union repaint exposes paint history | regions JUCE never repaints keep their first paint; a whole-MainComponent repaint re-renders text with different glyph edges (36k px, <= 91/255) -- main hid both behind its 30 Hz union | discovered: S/exp1-014023-after`
- `## 2026-09-29 layer vs peer AA differs by 1/255 at curved edges | a separate layer-backed NSView anti-aliases rounded corners 1/255 differently from the peer; peer user space and clip mirroring do not change it; painting the parent under a non-opaque panel does, but cost +60 ms/s | discovered: S/exp3-*`
- `## 2026-09-29 graphify post-commit hook loads the CPU | each git commit launches a python graphify rebuild; keep perf probes away from commits | discovered: git commit output`

## PACKET QUALITY

- Clarity: CLEAR. The plan plus adoption were detailed.
- HAD_TO_INFER:
  - I1's "hidden" vs drawing: a hidden NSView cannot draw, so I used alpha.
  - I1's vblank witness: I used VBlankAttachment rather than a capture burst.
- Missing context:
  - A background PopupMenu dies within 50 ms.
  - The graphify commit hook loads the CPU.
  - The mediaopen and video merges both landed mid-lane (two rebases).
- Unused context: none.
- Self-brief files: CLAUDE.md, the plan, the two attacks, diag-idle and its tools, JUCE sources. All useful.

### STATUS
DONE_WITH_CONCERNS

### NEXT ACTION
Harmony rules on HANDOFF-NEEDS (a)-(c), then the critic panel (I12), Boris's LOOK list (plan 3.4), and the merge (Pitfall NN to be numbered).
