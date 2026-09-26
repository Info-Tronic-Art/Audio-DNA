# Reviewer verdict — .harmony/probe-mastersignal.sh (Master Signal live gate, s-rta-0925)

VERDICT: APPROVE

## What I checked

- Read the full script (`.harmony/probe-mastersignal.sh`, 428 lines) end to end.
- `bash -n .harmony/probe-mastersignal.sh` → clean, no syntax errors.
- Cross-checked every numeric oracle in the script against the production endpoint
  implementations on `lane/0925-mastersignal-s1` (the merged Step-1 tree this gate is meant to
  pass on) and against `main` (the pre-Step-1 tree it is meant to fail-first on).
- Hand-derived the B1 expected numeric ranges from the actual `ScalarMath`/`ConnectionShaper`/
  `ConnectionEngine::evaluate` code (not from the plan's prose) to confirm the assertions are not
  a tautology and have real margin.
- Confirmed screen-safety practice against the script text (no screencapture, no output-window
  path, `open -g`, graceful-then-forced teardown, Quartz window-list gate that FAILs rather than
  skips when the venv is missing).

## Endpoint verification (production ApiServer.cpp, `lane/0925-mastersignal-s1`)

- `POST /api/set_master_signal` route wired at `src/api/ApiServer.cpp:179-181` → `handleSetMasterSignal` at `:576-593`. Parses `value`, 400s on missing key, marshals through `onSetMasterSignal` via `callAsync`, returns `jsonOk()` — matches the script's `set_master_signal()` helper and its `'"ok":[[:space:]]*true'` grep.
- `GET /api/status` publishes `masterSignal` at `:295` as `composition_.eff(CompScalar::Signal)` — matches `V_BOOT`/`jstat` (MINIMAL boot check).
- `GET /api/composition` publishes the raw `masterSignal` field at `:325`, and `live.signal` + `connected` come from the shared `addLiveBlock<Composition,CompScalar>` template (`:24-45`) applied per clip/layer/composition — matches `jpath "d.get('masterSignal','NA')"` and `jpath "d.get('live',{}).get('signal','NA')"`.
- `handleComposition`'s per-clip block (`:357-372`) emits `decks[].layers[].clips[].live.scale` via the same `addLiveBlock` template keyed by `clipScalarDefs()` — exact path the script's `sample_clip_scale()` reads.
- Pre-change binary (`main`, unmodified): none of the above three code paths exist (`grep -n "masterSignal\|set_master_signal" src/api/ApiServer.cpp` on `main` returns nothing) — a 404/empty body on `set_master_signal`, and `masterSignal`/`live.signal` genuinely absent from both JSON responses. This confirms the script's fail-first claim is real, not asserted.
- OSC: `/audiodna/signal` handled in `src/osc/OscHandler.cpp:145-151`, wired to `manualWrite(compScalarPath("signal"), depth, GripKind::Decaying, Origin::Human)` at `src/MainComponent.cpp:2082-2085`; `startListening(8000)` unconditional at `src/MainComponent.cpp:2143` (`kOscListenPort = 8000` at `:2142`) — matches the script's UDP target `127.0.0.1:8000` and address pattern.
- Tick-order hoist: `composition_.eff(CompScalar::Signal)` read once at `MainComponent.cpp:3352` (inside `tickFeaturePipeline`), above `processFrame`/`updateValues`/`ConnectionEngine::Context` — matches the plan's D2/critic-fold claim the script's header cites; not directly exercised by an assertion in this script (that's S1-T5/HOIST-OK's job in the unit-test suite), correctly out of scope here.

## B1 math verification (not a tautology, real margin)

Derived independently from `ScalarParams.h`/`ConnectionEngine.cpp` (not from the plan's prose):
- `ClipScalar::Scale` uses `expScale(v) = 2^((v-0.5)*2)` (model-space) / `normExpScale` (norm-space), default norm 0.5f, and `Clip::scale` itself defaults to `1.0f` model-space (`Clip.h:174`).
- At `signal=0`: `evaluate()` returns `manualNorm` exactly (`applyDepth`'s `depth<=0` branch, `ConnectionEngine.cpp` around `:189`); `manualNorm = toNorm(clip.scale=1.0) = 0.5` exactly; `toModel(0.5) = 2^0 = 1.0` exactly → the script's `1.0 ±1e-4` check is exact, not approximate-by-luck.
- At `signal=1`: full LFO swing shaped into `[0.3,0.7]` norm-space → `toModel(0.3)=2^-0.4≈0.758`, `toModel(0.7)=2^0.4≈1.3195`. The script's thresholds (`min<0.9`, `max>1.1`) have ~14-15% margin on each side of the true extrema — not a hair's-width pass.
- At `signal=0.5`: blend is `manualNorm + 0.5*(shaped-manualNorm)` in norm space *before* the nonlinear `toModel`, giving model-space range ≈`[0.871, 1.149]`, swing ≈0.278, versus swing ≈0.562 at `signal=1`. `numlt_strict` correctly asserts a monotonic ordering here — verified algebraically, not just structurally.
- Hue Shift param0 has no `ScalarMath` remap (effect params are raw `[0,1]` via `ScalarMath::identity`), so at `signal=0` it freezes at its manual default `0.5` (from the fixture's `"params":[0.5]`) and at `signal=1` sweeps the full sine — consistent with the render_frame identical/differ pairs.
- `hue_shift` shader declares only `u_texture`/`u_hue_shift` (`EmbeddedShaders.h:269-275`, no `u_time`) — confirmed by direct read, so a frozen hue param really does produce a static frame with no time-based confound, matching the script's own comment.
- `beat_ripple` registers 3 params (`intensity, decay, count`) at `EffectLibrary.cpp:761-765`, matching the fixture's `"params":[0.5,0.5,0.3]` and declares `u_beatPhase` (multiple `EmbeddedShaders.h` hits) — B2's premise holds.
- `render_frame`'s `time` parameter defaults to `-1.0` when omitted (`ApiServer.cpp:1041`) and `Renderer::captureFrame` only overrides `timeOverride_` `if (timeOverride >= 0.0f)` (`Renderer.cpp:1861-1862`) — the script's `R()` helper omits `time` entirely, so each capture reads the actual live render state rather than a fixed synthetic time; this is the correct call for a live-state gate (not `test_render_pipeline`'s deterministic-time usage), and it means the md5-identical/differ assertions genuinely track the live connection values, not a frozen clock.

## Fail-first check (pre-Step-1 binary)

Walked all three named-FAIL rows by hand against `main` (pre-Step-1):
- MINIMAL boot: `/api/status` has no `masterSignal` key → `jstat` returns `"NA"` → `approx` rejects non-numeric via its regex guard → correctly reports FAIL, not a vacuous pass.
- `set_master_signal`: route doesn't exist on `main`; cpp-httplib's stock unmatched-route response has no custom 404 handler in `ApiServer.cpp` (grep confirms) → body won't contain `"ok":true` → FAIL.
- B1 @ signal=0: since the write silently fails, the scale connection (already live from the merged Step-0 base this Step-1 branch built on) keeps swinging at full amplitude; `approx(MIN0,"1.0",...)` and `approx(MAX0,"1.0",...)` both fail against real swinging samples → correctly reports FAIL (not skip, not vacuous pass).
- B3: OSC handler doesn't exist pre-Step-1 → `masterSignal` never moves toward 0.3 → loop exhausts 10 iterations → FAIL with `last=<value>` printed.
All four of the numeric helpers (`approx`, `numlt`, `numgt`, `numlt_strict`, `minmax`) reject non-numeric input via an explicit regex guard before doing arithmetic, so a missing field can never arithmetic-coerce into a false PASS — this is fail-closed by construction, verified by reading the functions, not merely trusted from the header comment.

## Screen safety

- Launch: `open -g --stdout ... --stderr ... "$APPBUNDLE"` — background launch, never raises/focuses (line 183).
- No endpoint used here (`render_frame`, `load_composition`, `trigger_clip`, `set_bpm`,
  `set_master_signal`, `/api/composition`, `/api/status`) reaches `Renderer`/`MainComponent`'s
  `openOutputOnDisplay` path (menu/selector/keyboard-only, per CLAUDE.md and confirmed no such
  call appears in `ApiServer.cpp`'s handler set touched here).
- No `screencapture` anywhere in the script; the only screen witness is a `Quartz.CGWindowListCopyWindowInfo` window-list query filtered on `"Audio-DNA Output"` in the window title (never the app's own main window) — a metadata query, not a pixel capture.
- Teardown is `osascript -e 'quit app "Audio-DNA"'` first, with a wait loop, `pkill` only as a
  last-resort fallback if the process is still alive after 30s — matches the stated
  `probe-step3.sh` convention.
- The output-window check is gated on `.venv/bin/python`'s presence and explicitly reports FAIL
  (not silently skips) when unavailable — correct per the s-rta-0924 "a silently-dropped check
  must not let a run print a clean tally" rule the header cites.
- Uses the bracket trick (`MacOS/Audio-DN[A]`) in every `pgrep`/`pkill`, avoiding the documented
  self-match gotcha.

## Minor observations (non-blocking)

- The 1-beat Hue sine has an exact 0.5s period at BPM 120, same as the `sleep 0.5` interval used
  for the B1@signal=1 render-frame-differ pair; if the two `R()` calls happened to land exactly
  one full Hue period apart, Hue's *own* contribution to the pixel diff would coincidentally
  match, leaving the diff entirely dependent on the Scale connection (2-beat, 1s period, which at
  a 0.5s offset is a half-cycle, so it will differ except at symmetric sine nodes). This is not a
  script defect — real command/network/render latency makes an exact 0.500000s gap between two
  separate curl round-trips essentially unreachable — but it means the render-frame-differ
  assertion's actual load-bearing signal is the Scale connection, not necessarily Hue. Worth a
  one-line comment if this script is revised again; not blocking approval.
- `sample_clip_scale`/`comp_json` re-fetch and re-parse the full `/api/composition` JSON via a
  fresh `python3` process per sample (10-20 times per phase). Correct, just not the fastest
  possible implementation — no impact on correctness.

## Verdict

No blocking findings. The script's numeric oracles were independently re-derived from source
(`ScalarParams.h`, `ConnectionEngine.cpp`, `ConnectionShaper.cpp`) rather than trusted from the
plan's prose, and they hold with real margin, not by coincidence. Every endpoint the script calls
was located and read at the cited line in `src/api/ApiServer.cpp` / `src/osc/OscHandler.cpp` /
`src/MainComponent.cpp` on `lane/0925-mastersignal-s1`, and confirmed absent on `main` for the
fail-first requirement. Screen-safety rules are followed to the letter (no full-screen capture, no
Output window path, graceful-then-forced teardown, fail-closed venv gate). `bash -n` is clean.

FILES: .harmony/probe-mastersignal.sh
ISSUES: none blocking; 2 non-blocking observations (see above)

STATUS: DONE
METADATA: reviewer=reviewer-subagent, builder_packet=probe-mastersignal, date=2026-09-26
