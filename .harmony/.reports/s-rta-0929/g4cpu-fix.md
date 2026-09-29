STATUS: DONE_WITH_CONCERNS

# g4cpu-fix -- fix round 1 on lane/g4cpu (s-rta-0929), answering critic-g4cpu-r1

RESULT: I checked all five critic findings against the code and the lane report. Both MUSTs are correct about the tree.
HEAD has no code that reduces CPU, and there was no v5 before/after pair. The first MUST is fixed by reframing the
claim: this lane makes no CPU claim. The claim is now only that the TEST-ONLY instrumentation (c1) changes no pixels.
The second MUST and SHOULD 5 are fixed by building v5_routine_identity. It is a new default row of probe-idle-paint,
committed as 2df6876. It ran RED on main and GREEN on this build. At both injected beat positions, main and this build
match pixel for pixel (0 px differ) in the pad row, the strip column (V fill, band hairline, band name) and everywhere
else outside the fps readout and the SignalBar. The routine is visibly playing in every frame. One part of the critic's
request is not done: the "bound knob" region. No REST call or composition field can show a routine-held
UniversalParamControl in main's build. Section 3 gives the reason and the options. SHOULD 3 (a synthetic G3 test) is
conditional on a future hard gate, so it is not built. SHOULD 4 needs no fix.

FACTS (disk-cited):
- `git -C <WT> diff --stat ad35de1 42e3c93` shows 1 file changed: `.harmony/.reports/s-rta-0929/g4cpu.md` (+254),
  with 0 code files. ad35de1 is test-only (g4cpu.md:50-51). c2 was never committed (g4cpu.md:5-9).
- The new commit is 2df6876 `test(s-rta-0929 g4cpu-fix): probe-idle-paint v5_routine_identity ...`. It touches
  `src/ui/UiPaintCounters.h`, `src/MainComponent.cpp` and `src/api/ApiServer.cpp` (1 line each, all under
  AUDIODNA_TEST_SERVER), plus `.harmony/probe-idle-paint.{py,json}` and `.harmony/APP-INVENTORY.md`.
- RED run (main vs main, 13:24:37-13:24:57, load 7.60): log in scratchpad `g4cpu-fix/RED.log`.
- GREEN run (build-lane vs main, 13:33:02-13:33:22, load 6.28): log in scratchpad `g4cpu-fix/GREEN.log`.
- The evidence images are in scratchpad `g4cpu-fix/evidence/` (list in section 2).
- ctest serial 920/920 at 13:52 (section 4).

METHOD: I read each finding against `git diff`, the c1 source hunks and g4cpu.md. Then I built the v5 row that the plan
had pre-registered (plan-g4cpu.md:347, :354). I added one pre-registered line (cue) and always-written region PNGs. The
row ran RED on the pre-change app, then GREEN on this build against main. I looked at the frames myself. Last, I ran a
full serial ctest.

CONFIDENCE+VERIFY: HIGH that c1 changes no pixels in the routine-playing state at the two positions. The measurement is
0 px differ, max delta 0, at both P1 and P2, and the main-vs-main RED arm also reads 0 px, so the comparator's noise
floor here is 0. HIGH that the row has teeth: AFTER P1 vs P2 gives 9 violation clusters, all inside the pad row or the
strip column. MEDIUM on generalising this beyond the two captured positions and the test-mode fixture: it is two
frames, not a video. Verify with `IDLEPAINT_APP=<build-lane app> probe-idle-paint.sh <out> v5_routine_identity` under
the live lock.

UNKNOWNS-NOT-DONE: The bound-knob region (section 3). v5 was run once per arm (RED 1, GREEN 1). It is a pixel-identity
row, not a timing row, so I did not run a 5-run flake series. The c2 ruling (g4cpu.md section 6) is still Harmony's to
make; this round did not touch it.

NUANCE: v5 compares BEFORE and AFTER at the same injected clock. That makes it a same-pixels proof, not a same-cadence
proof. Cadence (how often each widget repaints) is the job of the a1 / g4 counters in the original lane report.

HANDOFF-NEEDS: (1) Re-issue the critic seat with the reframed claim (section 1) and the evidence in section 2.
(2) Decide whether the bound-knob gap (section 3) needs a two-build hook arm.

INBOX-RECHECK: none

---------------------------------------------------------------------------------------------------------------------
## 1. Findings -- verdicts

| # | sev | finding | verdict | action |
|---|---|---|---|---|
| 1 | MUST | "CPU drops" is false for HEAD | CORRECT about the tree (see FACTS: diff ad35de1..42e3c93 is doc-only; c1 is test-only). The lane report never claimed CPU drops (g4cpu.md:5-12: c2 STOPPED, not committed). The claim came from how the critic seat was framed. | REFRAMED. This lane claims no CPU change. The claim under review is now: "c1 (TEST-ONLY attribution witnesses) plus g4cpu-fix (TEST-ONLY preview_rect) change nothing Boris sees, even while a routine plays". A CPU look-check waits for Harmony's section-6 ruling on c2. |
| 2 | MUST | there was no v5 before/after pair or diff | CORRECT (g4cpu.md:35 lists v5 as not done; the supplied PNG was a v1 S2 card frame) | FIXED: v5_routine_identity built and run RED then GREEN, with region diff PNGs always written (section 2). The bound-knob region is not reachable (section 3). |
| 3 | SHOULD | the G3 source-stamping window is empirical only | CORRECT as stated. Its own fix is conditional ("if a future lane relies on G3 for a hard gate"). Today G3 feeds only INFO lines (a1). | NOT BUILT; recorded as found-not-fixed. If Harmony wants it, it is small: a header-only ctest of `uipaint::bump` / `passEnd` with AUDIODNA_TEST_SERVER defined. Such a test would prove the bookkeeping boundary, not JUCE's pass scheduling. |
| 4 | SHOULD | the c2 experiment was run after the STOP | CORRECT and already disclosed (g4cpu.md:9, :126-128) | No fix required (the critic says so). Plan-template note for Harmony: say whether a diagnostic experiment after a STOP is in bounds. |
| 5 | SHOULD | the frame does not show a routine playing | CORRECT: that frame was the card fixture with no routine | FIXED by v5. All four frames show the chartreuse V fill on L1-L3, the chartreuse "Sweep" band name, and the pad sweep at "2/4" (P1) and "3/4" (P2). A pre-registered line counts routine-cue pixels in the strip column: 12213 / 14973 / 12213 / 14973 (each >= 300). |

## 2. v5_routine_identity (commit 2df6876)
Spec: plan-g4cpu.md:347 + :354 (v5 config), unchanged except for two things. First, the preview is masked in the
teeth. The plan's teeth said "every violation in the pad row or the strip column", but the three layers' opacity changes
the preview between P1 and P2, so the preview's change is printed as INFO instead. Second, one line was added: `cue`,
routine-cue pixels in the strip column >= `minCuePx` 300, set before the first run. Masking the preview needs
`preview_rect` in `recordUiGeometry` / `GET /api/debug/ui_paint`. That is one TEST_SERVER line per file.

Flow (test mode, both apps): load g4's routine fixture, trigger L1-L3 column 1, then
`inject_features {bpm 120, totalBeatCount 100, beatPhase 0}` and `routine/fire 0`. Wait for `running`. Inject beat
106 + 0.5, wait 0.7 s, check `routine/status position` (6.5), capture P1. Inject beat 110 + 0.5, check position (10.5),
capture P2. Captures are window-only, by Quartz window id.

RED on the pre-change app (IDLEPAINT_APP = IDLEPAINT_APP_BEFORE = main's build, 13:24:38), verbatim:
```
    v5/before: load True routine state running positions [6.5, 10.5]
    v5/after: load True routine state running positions [6.5, 10.5]
PASS  v5_routine_identity P1 (routine position 6.5 beats): BEFORE vs AFTER outside the fps mask and the SignalBar -- 0 px differ (max delta 0), 0 violate K2 (> 1/255) in 0 cluster(s)
PASS  v5_routine_identity P2 (routine position 10.5 beats): BEFORE vs AFTER outside the fps mask and the SignalBar -- 0 px differ (max delta 0), 0 violate K2 (> 1/255) in 0 cluster(s)
FAIL  v5_routine_identity teeth: pad_row_rect / strip_col_rect / preview_rect absent from GET /api/debug/ui_paint (the app predates s-rta-0929 g4cpu-fix)

2 PASS / 1 FAIL / 0 SKIP (13:24:57, load 7.52 8.58 8.17)
PROBE-IDLE-PAINT RED
```
The main-vs-main arm is also the comparator's noise floor for this state: 0 px.

GREEN (IDLEPAINT_APP = build-lane at 2df6876's source, IDLEPAINT_APP_BEFORE = main's build, 13:33:03), verbatim:
```
    v5/before: load True routine state running positions [6.5, 10.5]
    v5/after: load True routine state running positions [6.5, 10.5]
PASS  v5_routine_identity P1 (routine position 6.5 beats): BEFORE vs AFTER outside the fps mask and the SignalBar -- 0 px differ (max delta 0), 0 violate K2 (> 1/255) in 0 cluster(s)
PASS  v5_routine_identity P2 (routine position 10.5 beats): BEFORE vs AFTER outside the fps mask and the SignalBar -- 0 px differ (max delta 0), 0 violate K2 (> 1/255) in 0 cluster(s)
PASS  v5_routine_identity cue: routine-cue pixels in the strip column (V fill + band name), BEFORE P1 / P2, AFTER P1 / P2: 12213 / 14973 / 12213 / 14973 (each >= 300: a routine plays in every frame)
    v5-teeth: K2 violation cluster bbox (254,667)-(293,690) px 960 max delta 211
    v5-teeth: K2 violation cluster bbox (254,859)-(293,882) px 960 max delta 211
    v5-teeth: K2 violation cluster bbox (254,475)-(293,498) px 960 max delta 211
    v5-teeth: K2 violation cluster bbox (358,858)-(395,859) px 76 max delta 152
    v5-teeth: K2 violation cluster bbox (358,474)-(395,475) px 76 max delta 152
    v5-teeth: K2 violation cluster bbox (600,360)-(625,395) px 936 max delta 30
    v5-teeth: K2 violation cluster bbox (582,360)-(597,395) px 545 max delta 30
    v5-teeth: K2 violation cluster bbox (358,666)-(395,667) px 76 max delta 152
    v5-teeth: K2 violation cluster bbox (652,372)-(660,384) px 72 max delta 173
PASS  v5_routine_identity teeth: AFTER P1 vs P2 -- 9 violation cluster(s): 3 in the pad row (bbox 40 pt wide, >= 15), 6 in the strip column (>= 1), 0 elsewhere (== 0; the preview is masked: INFO below)
INFO  v5_routine_identity teeth: the preview (the three layers' opacity) AFTER P1 vs P2 -- 205111 px differ (max delta 38), 201440 violate K2 (> 1/255) in 1 cluster(s)

4 PASS / 0 FAIL / 0 SKIP (13:33:22, load 6.99 7.59 7.97)
PROBE-IDLE-PAINT GREEN
```
Reading the teeth clusters (capture px at 2x): 3 x 960 px at x 254-293 are the three V faders' fill tops. 3 x 76 px
2-px lines at x 358-395 are the three band hairlines. 545 + 936 px at x 582-625 are the pad's teal sweep. 72 px at
x 652-660 is the pad's bar digit (2/4 -> 3/4). Nothing moved anywhere else, TopBar included.

Evidence for the critic seat (all in scratchpad `g4cpu-fix/evidence/`; rows = pad row / strip column / preview, each
drawn as A | B | diff, where magenta = differs > 1/255, yellow = differs <= 1/255, dimmed = equal):
- `v5-P1-regions.png`: main vs this build at routine position 6.5. The diff panels are all dimmed: nothing differs.
- `v5-P2-regions.png`: the same at 10.5, also all dimmed.
- `v5-teeth-regions.png`: this build at P1 vs P2. Magenta marks the V fill tops, the hairlines, the pad sweep + digit,
  and the preview picture.
- full frames `v5-before-P1/P2.png`, `v5-after-P1/P2.png`; `diff-v5-teeth.png`, `diff-v5-teeth-preview.png`.
LOOK (I viewed v5-after-P2.png and the three region PNGs): the ROUTINES row pad reads "Sweep 3/4" with its teal sweep.
All three strips (L1-L3) have the chartreuse V fill and the chartreuse "Sweep" band name over the thumbnail. The
preview shows the test card. The Clip tab reads "No clip selected". There is no Output window, and no dialog.

## 3. Not done: the bound-knob region
A "bound knob" is a UniversalParamControl whose value a routine's hand holds. It shows the value in the routine cue with
ROUTINE in its hint slot. The only such control this fixture can drive is the Layer tab's opacity control, and that tab
shows only after a user selects a layer. There is no REST route for layer or clip selection, and the composition file
carries no selection. I checked: ApiServer routes have no select/inspector endpoint, and DeckView::selectLayer is
reached only from mouse / keyboard paths. probe-routine-display gets there with `AUDIODNA_DEBUG_LAYER`, a TEMPORARY
hook that exists only in its `--hook` builds. Main's build does not have it, so a main-vs-this-build frame of a bound
knob would need the same temporary hook built into BOTH a pre-change tree and this tree. That means two extra builds,
plus a revert-and-rebuild. I did not do it in this round. Source-level fact (VERIFIED by reading ad35de1's diff): c1's
only change in UniversalParamControl is one counter line after the existing `repaint()` in `updateValueDisplay()`
(`uipaint::bump(...paramControlRepaints, SrcParam)`), and it touches no paint code there. The Dashboard knobs visible
in the Clip tab are MacroPanel knobs. Routines cannot hold macros ("macro/routine capture is LATER",
`src/recording/Program.cpp:104`). Options for Harmony: (a) accept the source argument for this region; (b) order the
two-build hook arm.

## 4. ctest
`ctest --test-dir build-lane -j1` at 13:52:21-13:52:43, under the lock, on build-lane built from 2df6876's source (load 9.33):
`100% tests passed, 0 tests failed out of 920` (21.38 s). The count is unchanged: this round added no ctest.

## 5. Screen safety and rig
- After each batch: `audio-dna windows 0, Output-named 0` and `UserNotificationCenter windows on screen: 0` (RED 13:24:57,
  GREEN 13:33:22, ctest).
- Launches used only `open -g ... --args --test-mode`, through the probe. There was no Output window, no synthetic
  input, no TEMPORARY hook (so there is nothing to strip), and no debugger / sample. test_output_window_level.py was not
  run.
- The lock was held per batch and released every time. The helper enforced the 45 s re-acquire cooldown. Most of the
  wait was on the asyncload / diag-vfps lanes' holds.
- The `.venv` symlink was removed before each commit. build-lane is kept.

## 6. Notes for Harmony to append (.harmony/notebook.md)
- The routine clock holds still at an injected `totalBeatCount + beatPhase` in test mode: `routine/status position`
  landed exactly on 6.5 / 10.5 in all four launches, 0.7 s after each inject. A test-mode capture at an injected
  position is deterministic to the pixel between two launches and two builds (0 px differ). | .harmony/probe-idle-paint.py
  row_v5
- No REST or composition path selects a layer or clip. The Layer tab (the only routine-held UniversalParamControl this
  fixture can drive) is reachable in a probe only through a TEMPORARY hook (`AUDIODNA_DEBUG_LAYER`, probe-routine-display
  --hook). A main-vs-lane pixel check of a bound knob therefore needs the hook in both builds. | src/api/ApiServer.cpp
  routes, src/ui/DeckView.cpp selectLayer

## PACKET QUALITY
- Clarity: CLEAR. The findings named their evidence lines, and the rig rules were explicit.
- Missing context: the critic asked for "bound-knob regions", which the fixture cannot show in main (section 3).
- Unused context: none.
- Self-brief files: g4cpu.md, critic-g4cpu-r1.md, plan-g4cpu.md (v5 row + config), the v5_row.py draft in the
  scratchpad, probe-idle-paint.{py,sh,json}, probe-routine-display.sh (the cue decoder), and ad35de1's diff. All were
  useful.
