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

---

# Fix round 1 (lane-name idlepaint-fix1) -- Harmony rulings J1-J5 (plan-idlepaint.md "HARMONY ADOPTION ADDENDUM -- fix round")

STATUS: PARTIAL

RESULT: J2, J3 and J5 are done. J4 is checked on GCC 15 and filed for GCC 11 / MSVC. J1 was diagnosed and then STOPPED under the packet's wrong-premise rule. The evidence:
- The pixels a whole-window pass changes come from the window's FIRST display pass.
- Main behaves the same way, and main's union never re-covered those regions.
- v4 as ruled (main idle vs the lane after a full pass) cannot go GREEN.
- The only lane-side fix would break v1 / v3 identity with main.

No app code changed this round.

FACTS:
- Commits on lane/idlepaint (42871bd ->):
  - df1221b J2 probe
  - ce6c2a8 J3 probe
  - a965a98 Pitfall NN correction (J1 diagnosis)
  - this report
- `git diff 42871bd -- src tests CMakeLists.txt` is empty. The final `build-lane` app differs from the saved 42871bd copy (`S/app-42871bd`) in 260 bytes, all at offsets >= 18234024. That is past the LC_CODE_SIGNATURE dataoff (18091376), so only the signature blob differs.
- Gates: `S/fix1/G1..G6.log`, `S/fix1/V3L.log`, `S/fix1/V3M.log`. J1 runs: `S/j1-b1..b5.log`, with captures in `S/j1-b*/<arm>/s*.png`.
- ctest serial `ctest --test-dir WT/build-lane -j1` -> "100% tests passed, 0 tests failed out of 920" (`S/fix1/ctest.log`).

METHOD:
- J1 instrumentation used a TEMPORARY env-gated diagnostic, saved verbatim in `S/j1-diag-final.patch`:
  - a per-pass log of CGContext / layer / window state;
  - one suspect toggled per launch.
- For "main", `git archive 5c5f21d` was exported to `S/mainsrc`, with a TEMPORARY `ADNA_TEMP_KICK_MS` one-shot repaint added, and built in `S/mainbuild`. The main checkout was not touched.
- Every capture is window-only (Quartz window id), decoded with PIL + numpy. Zoomed crops and the J3 diff PNG were looked at by eye.
- The diagnostic was reverted with `git apply -R`, then rebuilt: `strings` "ADNA_TEMP|J1DIAG" = 0, and `git status` is clean.

CONFIDENCE+VERIFY:
- High on the J1 attribution to pass order: deterministic, identical counts across 16 lane launches and 2 main launches.
- Medium on the mechanism inside AppKit / Core Animation. It is INFERRED: no public API shows a difference between the two passes.
- Re-prove:
  - `LANE=idlepaint . S/../lib/lock.sh`
  - `bash S/fix1/gbatch.sh <tag> <rows>`
  - `J1FIX=none python S/j1exp.py <out> <app> full ADNA_TEMP_T=kick:150` needs the diagnostic build (`S/app-diag`).

UNKNOWNS / NOT DONE:
- J1: no fix and no v4 row (STOPPED; options below).
- J4 on GCC 11 and MSVC.
- The exact Core Animation step that makes the first pass differ.

NUANCE: the r1 report's "text re-rendered, up to 91/255" was wrong on two counts:
- Text differs by exactly 1/255, at anti-aliased edges.
- The 91/255 is the FPS readout's number changing value, which is live content.
The real large classes are the Files grid's folder emoji (<= 66/255) and three TopBar slider-thumb rims (<= 29/255).

HANDOFF-NEEDS: Harmony rulings on:
- (a) J1: options A / B / C below.
- (b) J3: the rule rejects the SignalStrip top-corner anti-aliasing (1/255, 3x3 span 14-30, so below edgeSpan 32). v1 S1-S3, v1b SignalBar and v2 A-B FAIL on 8 px each (S3 also on 13 deck-cell px). Accept, re-rule the edge span, or order a corner fix.
- (c) v3: flake verdict, below.

INBOX-RECHECK: none

## Fix round 1 -- ruling -> commit -> RED -> GREEN

| ruling | commit | RED (raw line) | GREEN (raw line) |
|---|---|---|---|
| J5 rebase onto main | none (no-op) | -- | `git rev-parse --short main` = 5c5f21d = the lane's merge-base. It already contains 3919ea5 and the docs commit 5c5f21d. Re-checked 05:12:35 before committing. |
| J2 g4 CPU -> INFO | df1221b | g4 BEFORE arm (main), per launch: `g4_routine_before r1: win_max_med 17.4 ms` ... `r5: 17.2 ms` (16.5-17.9 > 8.0; the r1 report's R3 gave 19.3) | `PASS  g4_routine (a loop routine on 3 layers, adoption I3): window max median 5.2 ms [4.9-5.4] (<= 8.0) \| main CPU median 163.6 ms/s [160.6-165.9] (INFO, ruling J2)` and `INFO  g4_routine CPU (INFO, ruling J2): main-thread CPU median BEFORE 335.1 ms/s [330.5-340.5] \| AFTER 163.6 ms/s [160.6-165.9] (i1's 150.0 ms/s is not applied to g4)` |
| J3 identity rule (v1 / v1b / v2) | ce6c2a8 | teeth: `PASS  v0_capture_teeth (J3): the J3 identity rule rejects the 1-px shift -- 25859 px differ (max delta 189), 25601 violate J3 ... in 81 cluster(s) (> 0)` | not GREEN everywhere, see the next table. PASS: v1 S4 waveform, v1b waveform, v2 A-C. FAIL: 8 px of SignalStrip-corner AA |
| J1 full-pass identity + fix | a965a98 (Pitfall NN text only) | diagnosis only: `INFO  v1_identity_test_mode S4 whole window (BEFORE's first-pass pixels vs AFTER's full pass, J1): 35483 px differ (max delta 66), 20590 violate J3` | STOPPED: no v4 row, no fix (below) |
| J4 atomic_ref portability | none (check only) | -- | `g++-15 (Homebrew GCC 15.2.0_1)`: `__cpp_lib_atomic_ref=201806 is_always_lock_free=1 pos=0.25`, exit 0. Apple clang 17.0.0 gives the same. GCC 11 (CI ubuntu-22.04) and MSVC were not available locally: FILED |

## Gates on the final app (= 42871bd code), 5 launches per perf arm; load average printed per launch (2.9-6.5)

| row | result (raw) | log |
|---|---|---|
| c0 | `PASS` heartbeat ok, peer_layer_backed 1, capture not blank | G1 |
| i1 | `PASS  i1_idle_card (IDLE-HB-card): window max median 4.3 ms [4.2-4.9] (<= 8.0) AND main CPU median 112.6 ms/s [111.9-133.2] (<= 150.0)` | G1 |
| g1 / g3 | `PASS  g1_anim_rates` (wf / sb 29.1-29.2/s, top 14.8-14.9/s, modes [0, 0], 0 fallbacks); `INFO  g3_peer_quiet: MainComponent paints/s 14.9, 14.9, 14.8, 14.9, 14.9 \| ClipInspector repaints/s 0.00 x5` | G1 |
| i2 | `PASS  i2_idle_many16 (IDLE-HB-many16): window max median 4.8 ms [3.8-5.2] (<= 8.0) AND main CPU median 128.5 ms/s [117.2-130.9] (<= 150.0)` | G2 |
| g4 | PASS window max / INFO CPU (table above) | G3 |
| v0 | `PASS` 78 clusters in the SignalBar, 2 in the waveform, 0 elsewhere; J3 teeth PASS | G4 |
| v1 S1 | `FAIL  v1_identity_test_mode S1 (default): BEFORE vs AFTER outside the fps mask -- 1251 px differ (max delta 1), 8 violate J3 (> 1/255 or off an anti-aliased edge) in 6 cluster(s)` | G4 |
| v1 S2 | `FAIL ... S2 (card): ... 110 px differ (max delta 1), 8 violate J3 ... in 6 cluster(s)` | G4 |
| v1 S3 | `FAIL ... S3 (many16): ... 90 px differ (max delta 1), 21 violate J3 ... in 19 cluster(s)` (the 6 strip corners + 13 single deck-cell px) | G4 |
| v1 S4 waveform | `PASS ... the waveform rect + 8 px BEFORE vs AFTER -- 0 px differ (max delta 0), 0 violate J3` | G4 |
| v1n (INFO) | S1 0 px; S2 33 px (max 1), 0 violate; S3 0 px | G4 |
| v1b SignalBar | `FAIL  v1b_native_vs_inpeer SignalBar: native layer vs forced in-peer at a frozen driven state -- 8 px differ (max delta 1), 8 violate J3 ... in 6 cluster(s)`; the round trip is 0 px (PASS) | G5 (TEMPORARY freeze hook copy `S/app-hook2`; reverted, strings 0) |
| v1b waveform | `PASS ... waveform: native layer vs forced in-peer ... -- 28 px differ (max delta 1), 0 violate J3`; round trip 0 px | G5 |
| v2 | modes PASS; `FAIL  v2_identity_fallback: A vs B outside the overlay's rect -- 8 px differ (max delta 1), 8 violate J3 ... in 6 cluster(s); the overlay IS visible over the SignalBar (75608 px changed there)`; `PASS ... A vs C ... 0 px`; covered 0 PASS; real PopupMenu PASS | G4 |
| v2b | `PASS` 10 fallbacks, 0 covered vblanks; `PASS` restore max 2 (<= 2); teeth `PASS` covered 11 | G6 |
| v3 | `FAIL  v3_identity_production_masked: ... 33 px differ in 19 cluster(s), max delta 1` in G6. Flake arms, 5 runs each: lane vs main 0 / 0 / 33 / 33 / 33 px (V3L); main vs main 33 / 0 / 33 / 33 / 0 px (V3M, compared offline with the same masks because the main app has no geometry route). The same x = 613 px column (1/255) appears in both arms. Verdict: main's own launch-to-launch noise, a flake. The existing row was not re-thresholded. | G6, V3L, V3M, `S/fix1/v3off.py` |

## J1 -- the diagnosis (why a whole-MainComponent pass changes pixels)

Classes, from lane idle vs the lane after `POST /api/debug/ui_repaint_all` (`S/j1-b1/base`, `S/j1ana.py`):

| class | px | max delta | px violating J3 |
|---|---|---|---|
| Files grid folder emoji (U+1F4C1, FilesBrowser::paintGrid) | 24,507 | 66 | 20,224 |
| Three TopBar slider thumbs (Fade, Master Signal, Master) | 1,648 | 29 | 343 |
| All text | 9,756 | 1 (at AA edges) | 2 |
| FPS readout | 91 | 91 | -- (the number changed value: live content, masked by the fps mask) |

Zoomed crops (`S/j1-zoom.png`):
- The first-pass emoji is a little smaller and softer: bbox 1 px narrower, 3 % less gradient energy.
- The slider thumbs have the same centroid (to within 0.003 px) but different rim coverage.

Which render is which:
- The first-pass look comes from the peer's FIRST drawRect. It is pass n=1: the whole window at 0 ms, before the window is on screen, occlusionState 8192.
- Any later pass draws the steady look:

| trigger | when | result (s0 == after the pass) |
|---|---|---|
| one-shot full repaint (`kick`) | 300 / 1000 / 2500 / 5000 ms | folders 0, sliders 0 (`S/j1-b3`) |
| `kick:150` (it landed in pass n=2, still off screen) | 75 ms | folders 0, sliders 0 (`S/j1-b4/k150`) |
| partial passes (`halves`: 2 half-window passes 200 ms apart) | -- | the same change as a full pass (`S/j1-b1/halves`) |

So on-screen vs off-screen is not the variable, and neither is the size of the dirty rect.

What does not differ between pass 1 and later passes, logged per pass (`J1DIAG`):
- user->device transform [2 0 0 2];
- the context is not a bitmap context (the async display list);
- layer drawsAsynchronously 1, opaque 1, contentsScale 2.0, contentsFormat RGBA8;
- window screen "Built-in Retina Display", backingScaleFactor 2.0, colour space "sRGB IEC61966-2.1", frame 1728x1079.

Suspects, one TEMPORARY env toggle per launch. Every row still read folders 24,507 / 66 and sliders 1,648 / 29:

| toggle | what it did | log |
|---|---|---|
| smooth0 | font smoothing off | `S/j1-b1` |
| subpix0 | subpixel positioning / quantization off | `S/j1-b1` |
| interp | kCGInterpolationHigh | `S/j1-b1` |
| opaque | peer layer.opaque = YES | `S/j1-b1` |
| sync | drawsAsynchronously = NO from pass 2 | `S/j1-b1` |
| syncearly | drawsAsynchronously = NO before pass 2 | `S/j1-b4` |
| halves | partial passes instead of a full one | `S/j1-b1` |
| ADNA_UI_NATIVE_LAYERS=0 | no native subviews | `S/j1-b1` |
| warm | a CoreGraphics `createComponentSnapshot` of MainComponent in its constructor, before the first paint, to rule out JUCE first-use caches | `S/j1-b5/warm` |

Main (`S/j1-b5/mh1`, `mh2`: a scratch build of main 5c5f21d plus a TEMPORARY one-shot repaint 13 s after the first paint):
- Before the pass, main+hook equals the shipped main at idle: 33 px, max 1, 0 J3 violations.
- The pass changes the same 24,507 emoji px (max 66), 1,648 slider px (max 29) and ~9.8k text px at 1/255.
- The change PERSISTS: 4 s later only the fps readout differs (mh1 209 px, mh2 99 px, all fps).
- Main after its pass vs the lane (42871bd) after its pass: folders 0, sliders 0. The remaining 120 px (max 3) are the waveform / strip corners.
- Main idle vs lane idle: 77 px, max 1 (8 J3 px: the strip corners).

In short, the lane equals main in both states. Main's 30 Hz union never covered the Files grid or the right-hand TopBar sliders, so the critic's premise ("on main the union overwrote it within a frame") does not hold for any pixel above 1/255.

Root cause: inside AppKit / Core Animation (INFERRED). The first display pass into the peer's layer rasterises emoji bitmaps and ellipse rims differently from every later pass. Every input observable through public API is equal. No JUCE patch is involved.

Why STOPPED (the packet's wrong-premise rule):
- (i) v4 as ruled compares main's FIRST-pass pixels with a later pass of any build. No lane change can make it GREEN except re-drawing the first-pass look.
- (ii) The only lane-side fix at the root would be ONE repaint right after the first pass. That is a single pass, neither periodic nor in the background. Measured: `kick:150` gives 0 px of emoji / slider change on a later full pass. But it makes lane idle differ from main idle by the same 24,507 emoji px (max 66), so v1 / v3 BEFORE-vs-AFTER identity would FAIL. The two rulings conflict.

Options for Harmony:
- A: accept. The behaviour is main's own and identical in the lane.
- B: adopt the one-shot post-first-pass repaint, and re-base v1 / v3 on a main full-pass reference (a scratch hook like `S/mainsrc`'s).
- C: a separate AppKit investigation lane.

## J4 -- portability

- `S/j4/atomic_ref_probe.cpp` is the LayerStrip.cpp:749 construct (`std::atomic_ref<double>(clip->playheadPosition).load(relaxed)`) plus `#ifndef __cpp_lib_atomic_ref #error`.
- It builds and runs under Homebrew GCC 15.2.0 (libstdc++) and Apple clang 17.0.0.
- NOT verified: CI's ubuntu-22.04 GCC (11.x) and windows-latest MSVC. No local toolchain, and no local docker image of either. FILED as a CI-only pre-merge check.
- `src/features/FeatureBus.h:20` ("std::atomic_ref is unavailable on this toolchain") is stale for the current Apple clang (verified above). Not edited: outside this lane.
- Reviewer NIT, filed for L5: `ClipInspector::paintKeyNow` reads the same field plainly.

## Rig (fix round 1)

- 13 lock holds, each <= 7.3 min. Each release was followed by >= 40 s before the next acquire; the helper enforced 45 s.
- 79 launches, all `open -g`. No Output window: `outwins` gave 0 Output-named windows after every hold. No synthetic input. No debugger or full-screen capture.
- The scratch main build and a diagnostic rebuild compiled during the J1 pixel batches b4 / b5. Those batches produced pixel identity only, no perf numbers. Every perf batch (G1-G3) took the lock via `acquire_quiet_lock`.
- The `.venv` symlink was created for the gate runs and removed before the first commit.
- TEMPORARY hooks and where they are now:
  - the J1 diagnostic (reverted);
  - the v1b waveform freeze (a copy in `S/app-hook2`, reverted);
  - the main one-shot (scratch source only).
  - After the revert rebuilds: `strings` on the build-lane app gives 0 for "ADNA_TEMP|J1DIAG", and `git status` shows only `build-lane/`.

## Work-log rows (fix round 1, for Harmony)

- J1: diagnosed, STOPPED. The whole-pass change is the window's first display pass (emoji <= 66, slider rims <= 29, text 1/255), identical in main.
- J2 df1221b: g4 PASS 5.2 ms / CPU INFO 163.6 vs main 335.1.
- J3 ce6c2a8: v1 S1-S3 / v1b SignalBar / v2 A-B FAIL on 8 strip-corner px at 1/255 (span < 32).
- J4: GCC 15 PASS, GCC 11 / MSVC filed.
- J5: no-op.
- i1 4.3 / 112.6, i2 4.8 / 128.5.
- v3 is a flake: 3/5 in both arms.
- ctest 920/920.

## Notebook lines (fix round 1, for Harmony to append)

- `## 2026-09-29 the macOS window's first display pass draws differently | the peer's FIRST drawRect rasterises colour-emoji glyphs (<= 66/255) and ellipse rims (<= 29/255) differently from every later pass; the context / layer / window state logged equal and 7 toggles change nothing; any repaint after pass 1 (even at 75 ms, off screen) draws the steady look | discovered: S/j1-b1..b5`
- `## 2026-09-29 an fps readout in a capture diff reads as a big glyph delta | the TopBar FPS number changes value between captures (91/255) -- mask it before calling a diff a rendering change | discovered: S/j1-regions.py`
- `## 2026-09-29 a main-with-hook build without touching the main checkout | git archive <sha> | tar -x into scratch, add the TEMPORARY hook there, configure with FETCHCONTENT_SOURCE_DIR_* -> an independent app (built in ~2 min here) | discovered: S/mainbuild.sh`

## PACKET QUALITY (fix round 1)

- Clarity: HAD_TO_INFER.
  - J1's gate (main idle vs lane after a full pass) and its fix sentence conflict with the v1 / v3 identity contract once the cause is known. This was surfaced, not improvised.
  - J3's "spans > 32 levels" is read per channel, in either capture.
- Missing context:
  - The r1 "91/255" was the FPS readout.
  - The main app has no full-pass trigger, so a scratch main+hook build was needed.
- Unused context: none.
- Self-brief files: the plan's addendum, critic-idlepaint-r1.md, review-idlepaint-juce-r1.md, the r1 report and the r1 scratch evidence. All were useful.

### STATUS
PARTIAL

### NEXT ACTION
Harmony rules on:
- J1 option A / B / C;
- the J3 SignalStrip-corner FAILs (accept, re-rule the edge span, or order a corner fix);
- the v3 flake.

Then the critic re-review of J1 with the main+hook evidence, and the merge (Pitfall NN -> 57).
