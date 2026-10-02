## BUILDER REPORT -- diag6 / BF6 "Timeline control is not working for parameter control" (diagnosis only, no fix)

STATUS: DONE
RESULT: Root cause, proven with printed values: the "Timeline" source is a 4-beat ramp on the global beat clock and never reads the clip's playhead. The engine never passes a clip clock (always nullptr), so nothing playhead-based can work today. Evidence is in this report; a scratch test is committed on diag/bf6 6ccbf2d.
FACTS: `src/connect/ConnPicker.cpp:90` (Timeline -> Envelope, Clock::Beats, cycleBeats 4, points (0,0)-(1,1)); `src/connect/ConnectionEngine.cpp:101` + `:133-138` (Beats clock = beatPhase + beatInBar + 4*bars); `src/connect/ConnectionEngine.cpp:343-358` + `:376` (tick passes `nullptr` as the ClipClock for every layer / clip / effect / source-param); `src/connect/ConnectionEngine.cpp:139-151` (nullptr clock -> position 0); `src/connect/ConnectionEngine.h:12-25` (ClipClock seam, "Lane 5 wires the real thing" -- never wired); `src/MainComponent.cpp:4106` (the only tick call site, message thread, 120 Hz); `src/MainComponent.cpp:1859` (test mode never starts AnalysisThread, so the beat clock is frozen at 0 there); `src/model/Clip.h:238` (playheadPosition, RelaxedDouble, GL-written); `.harmony/binding-decisions.md:591` (BF6 ruling)
METHOD: (1) Read the code path from the picker through the serializer, the engine and the twin to /api/composition. (2) Wrote a scratch Catch2 test against the real classes: sourceFromPicker -> ConnectionEngine::tick -> scalarLive / paramLive. (3) Ran the real app (worktree build, `open -g --args --test-mode`, under the lane lock) twice. It loaded a composition with a 12 s video clip: the clip opacity, Ripple param 0 and the layer opacity were connected to Timeline (the exact picker output), and the clip positionX to Clip Position. /api/composition and /api/bpm were sampled over time.
CONFIDENCE+VERIFY: HIGH. Re-run `build-diag6/tests/test_diag_bf6_timeline -s` in the diag6 worktree (expect tables [A]-[E] below). Live: re-run scratchpad `diag6/probe.py` and `diag6/probe2.py` against the worktree app in test mode (expect the tables below).
UNKNOWNS/NOT-DONE: No fix (out of scope). Live run 1 could not show the free-running musical beat clock because test mode has no AnalysisThread, and /api/set_bpm is a no-op there (bpm stayed 0.0). Run 2 drove the beat clock through the test-mode FeatureBus writer (/api/inject_features), a REST test hook. The UI picker was not clicked (no synthetic input). The composition JSON carries exactly the ConnSource the picker builds (`src/connect/ConnPicker.cpp:90-99`), and the unit test drives the real sourceFromPicker.
NUANCE: "Clip Position" in the same picker shares the root cause: it reads 0 forever (live: clip positionX pinned at -1920 px, the left edge). Separately, when a clip clock IS supplied, playhead 1.0 evaluates to 0, not 1, because frac(1.0) == 0 in playbackXform. A OneShot clip holding at its end would snap the value back to the start of the curve.
HANDOFF-NEEDS: none

### SUMMARY
**Verdict:** a parameter set to Timeline is clocked by the global beat clock (a 0->1 ramp every 4 beats, then a wrap). It ignores the clip playhead completely. The engine also has no way to read a playhead: `ConnectionEngine::tick` hands every clip, layer, effect and source-param connection a `nullptr` ClipClock.

What Boris sees:
- **No music, or tempo not running:** the value sits frozen at the bottom of its range.
- **Music playing:** the value ramps once a bar, unrelated to the clip.
- **Retrigger, pause or scrub the clip:** nothing happens to the value.

### 1. What "Timeline" IS today (code, verified by reading)
| Question | Answer | Cite |
|---|---|---|
| What the picker writes | `ConnSource{Kind::Envelope, env.clock = Beats, cycleBeats = 4, curve = (0,0)->(1,1) Linear}` -- a fixed ramp. There is no curve editor (ruling 8 "BUILD THE DRAWABLE TIMELINE CURVE" is unbuilt; the menu comment still says "placeholder for future"). | `src/connect/ConnPicker.cpp:90-99`, `src/ui/UniversalParamControl.cpp:513-514`, `:563-570` |
| What it reads | `bn = beatPhase + beatInBar + 4 * barsSinceResync()` from the FeatureSnapshot; `pos = frac(bn / 4)`; `raw = curve.eval(pos)` | `src/connect/ConnectionEngine.cpp:101-102`, `:133-138`, `src/connect/ConnectionShaper.cpp:16-23` |
| Time base | Musical beats from the analysis thread's BPM tracker (FeatureBus). Not the clip's playhead, not wall time. | same |
| Who advances it | AnalysisThread (production) publishes beatPhase / bars. Test mode does not start it, so the clock is frozen at 0. | `src/MainComponent.cpp:1859-1863` |
| Which thread evaluates | Message thread, 120 Hz `tickFeaturePipeline` -> `connectionEngine_.tick(composition_, ctx)`. This publishes into the LiveValue twins (`scalarLive`, `paramLive`, `sp.live`), which the renderer reads through `eff()` / `effParam()`. | `src/MainComponent.cpp:4100-4108`, `src/connect/ConnectionEngine.cpp:246`, `:273` |
| The playhead-clock seam | `ConnSource::Envelope::Clock::ClipPosition` and `ClipClock` exist and work in `evaluate()`. `tick()` passes `nullptr` at every site ("Lane 5's job"), so the clip position is always 0. | `src/connect/ConnectionEngine.h:12-25`, `src/connect/ConnectionEngine.cpp:343-358`, `:376`, `:139-151` |
| Any second writer? | No. EffectStackView::tickModulation is display-only, and the engine is the only writer of the twins. | `src/ui/EffectStackView.cpp:137-148` |
| Dead twin | `ClipPositionSignal` (registry signal "Clip Position", Type::Audio) has no caller of `updateFromClip` / `setCurrentPosition` (grep: none outside its own header), so it reads 0 forever. It is listed in the Audio submenu. | `src/signal/ClipPositionSignal.h:26-42`, `src/signal/SignalRegistry.cpp:74-80`, `src/ui/UniversalParamControl.cpp:420-437` |

### 2. Reproduction -- values printed

**Unit (real classes; scratch test `tests/test_diag_bf6_timeline.cpp`, diag/bf6 6ccbf2d).** Setup: a clip's opacity, Ripple p0 and its layer's opacity all get `sourceFromPicker(Timeline)`, then one `ConnectionEngine::tick` per row. Full output: scratchpad `diag6/unit-out.txt`.
```
Timeline -> ConnSource kind=4 (Envelope) clock=0 (Beats) cycleBeats=4.0 pts=2
[A] beat clock FROZEN at beat 0, clip playhead sweeps 0 -> 1
  playhead 0.00 .. 1.00   clipOpacity 0.0000  fxParam0 0.0000  layerOpacity 0.0000   (all 11 rows identical)
[B] clip playhead FROZEN at 0.25, beat clock 0 -> 4 beats
  beats 0.0 0.5 1.0 1.5 2.0 2.5 3.0 3.5 4.0
  value 0.000 0.125 0.250 0.375 0.500 0.625 0.750 0.875 0.000   (clip, fx and layer identical)
[C] "Clip Position" source, playhead 0 -> 1:   0.0000 at every row (tick passes clock == nullptr)
[D] Timeline curve re-clocked to Envelope::Clock::ClipPosition, playhead 0 -> 1: 0.0000 at every row
[E] same connection through ConnectionEngine::evaluate WITH a ClipClock:
  clock 0.0 0.2 0.4 0.6 0.8 1.0
  y     0.0 0.2 0.4 0.6 0.8 0.0     <- follows the playhead; wraps to 0 at exactly 1.0
```

**Live app** (worktree build `build-diag6`, `open -g ... --args --test-mode`, lane lock `diag6`). Fixture: scratchpad `diag6/comp_timeline.json`, a 12 s 320x240 video (`ramp12.mp4`) on deck 0 layer 0 column 0, triggered via `/api/trigger_clip`. Clip opacity, Ripple param 0 and layer opacity are on Timeline; clip positionX is on Clip Position. Logs: scratchpad `diag6/run1/probe-out.txt`, `diag6/run2/probe-out.txt`.

Run 1 (no tempo command; test mode, so beat clock = 0):
```
 t     playhead  clip.opacity  ripple.p0  clip.posX(px)  layer.opacity || bpm  beatPhase
 0.01  0.0438    0.0000        0.0000     -1920.0        0.0000        || 0.0  0.0000
 2.42  0.2443    0.0000        0.0000     -1920.0        0.0000        || 0.0  0.0000
 4.81  0.4435    0.0000        0.0000     -1920.0        0.0000        || 0.0  0.0000
 set_bpm 120 -> {"ok":true}, but bpm stays 0.0 (no AnalysisThread in test mode)
 5.84  0.9942    0.0000 ...    (playhead loops 0.99 -> 0.02; value never moves)
 retrigger -> playhead 0.0000 -> 0.2360 over 2.85 s; value 0.0000 throughout
```
Run 2 (beat clock driven through the test-mode FeatureBus writer):
```
H: beat HELD at 2.0          playhead 0.0787 -> 0.3526 over 3.3 s   value 0.5000 on every row (clip, fx, layer)
S: beat STEPPED 0 -> 4.5     playhead 0.4004 -> 0.6795 (smooth)      value 0.000 .125 .250 .375 .500 .625 .750 .875 0.000 .125
   (each step lands exactly beat/4 mod 1; clip.posX stays -1920 throughout)
```
Rig hygiene: 0 Output-named windows during both runs. App not running after each quit. The lock was released and is absent afterwards. 0 UserNotificationCenter windows (Quartz kCGWindowListOptionAll) 26 s after the last quit.

### 3. Root cause
Two faults together. Both are proven by the values above.
1. **Wrong clock in the picker translation.** `sourceFromPicker(Timeline)` builds `Envelope{clock = Beats, cycleBeats = 4}` (`src/connect/ConnPicker.cpp:90-99`). The value is therefore a function of the beat clock only:
   - [B] and S: it moves exactly with the beats while the playhead is held or moves at its own rate.
   - [A], H and run 1: it ignores the playhead completely.
2. **The playhead is never handed to the engine.** Even a correctly clocked connection (`Envelope::Clock::ClipPosition` or `Kind::ClipPosition`) reads position 0. `ConnectionEngine::tick` passes `nullptr` for the ClipClock at every layer and clip site (`src/connect/ConnectionEngine.cpp:343-358`, `:376`). The "Lane 5" wiring announced in `src/connect/ConnectionEngine.h:16-20` never landed. Evidence: [C] and [D] give 0 for every playhead; [E] follows the playhead once a clock is passed; live clip.posX is pinned at -1920 px.

(Run 1's frozen-at-0 value is the test-mode artefact of fault 1, the no-AnalysisThread case. In production with no music, or before the tracker runs, the beat clock gives the same frozen bottom-of-range value.)

### 4. What would make the value follow the clip's playhead
These are recommendations, not implemented, and the details are inferred from the code. They match the BF6 ruling (`.harmony/binding-decisions.md:591-593`).
- **Wire a real ClipClock in `ConnectionEngine::tick`.**
  - Clip-owned connections (`clip.scalarConns`, `clip.effects[].paramConns` / `dryWetConn`, `clip.sourceParams[].conn`): a clock reading `clip.playheadPosition.load()` normalized to `inPoint..outPoint`. That is the same math as `ClipPositionSignal::updateFromClip`, `src/signal/ClipPositionSignal.h:33-41`.
  - Layer-owned connections (`layer.scalarConns`, `layer.layerEffects`): the clock of the layer's active clip (`layer.runtime().activeClipColumn`).
  - During a crossfade the layer has two live clips (Pitfall 35), so the incoming / active one should be the clock.
  - Thread-safe as is: `playheadPosition` is a RelaxedDouble, one relaxed load from the message thread (`src/connect/ConnectionEngine.h:12-15`, `src/model/Clip.h:238`).
- **Re-clock Timeline:** `sourceFromPicker(Timeline)` -> `env.clock = Envelope::Clock::ClipPosition` (one pass of the curve = one pass of the clip). Beat-locked motion stays available under BPM Sync.
- **Composition controls:** they have no playhead. Boris says it is not needed, so either hide Timeline in the composition picker or keep it on the beat clock. This is a ruling call for Harmony / Boris.
- **End-of-clip wrap:** for the ClipPosition clock, do not `frac()` a position of exactly 1.0 (`src/connect/ConnectionShaper.cpp:29-30`; [E] shows 1.0 -> 0). Otherwise a OneShot clip parked at its end snaps the parameter back to the curve start.
- **The same wiring fixes "Clip Position"** (`Kind::ClipPosition`), which is broken today for the same reason.
- **Saved files:** existing Timeline connections are saved as `"clock":"beats"` (`src/connect/ConnSerialization.cpp:120-124`). Whether old files keep beats or migrate is a decision for Harmony.
- **Not addressed by this:** the drawable curve itself (ruling 8) is still unbuilt. The fix above only makes the fixed 0->1 ramp follow the playhead.

### FILES CHANGED
- `tests/test_diag_bf6_timeline.cpp` (worktree diag6, branch diag/bf6) -- scratch diagnostic test (prints tables A-E). Not a regression test.
- `tests/CMakeLists.txt` (worktree diag6, branch diag/bf6) -- scratch target `test_diag_bf6_timeline`.
- Committed on diag/bf6 only: `6ccbf2d`. Nothing on main. This report is in the main checkout and is not committed.

### TESTS
- test_diag_bf6_timeline: 14 assertions, 2 cases, PASS. The assertions pin the CURRENT (broken) behaviour: value constant under a moving playhead; value moves with beats; Clip Position == 0.
- Live probes: run1 / run2 as above (scratchpad `diag6/probe.py`, `diag6/probe2.py`).

### SLIM CHECK
nothing to cut -- the only changes are a scratch test and its target on the diag branch.

### METRICS
Build: worktree configure + full app build OK (ccache-warm); scratch test built and ran in < 1 min. Live: 2 launches, ~20 s each.

### KNOWLEDGE CONTEXT
- Tools used: grep. Impact authority: grep (not authoritative; diagnosis only, so nothing was deleted). God nodes: n/a. Risk: NORMAL. Queries: 0 graph.

### PACKET QUALITY
- Clarity: CLEAR
- Missing context: test mode does not start AnalysisThread, so /api/set_bpm is a no-op there and the live beat clock is frozen. The beat clock had to be driven with /api/inject_features (a REST test hook, not UI input).
- Unused context: recording.md / rendering.md (not needed; the engine source was enough).
- Self-brief files: CLAUDE.md (useful), binding-decisions.md BF6 entry (useful).

INBOX-RECHECK: none
### STATUS
DONE
### NEXT ACTION
Harmony: plan the fix in section 4. Rule on the composition-level Timeline and on migrating saved `"clock":"beats"` Timeline connections.
