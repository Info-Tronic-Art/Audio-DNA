# Validation Queue — Audio-DNA (RealTimeAudio)

<!-- Lets the NEXT session/agent mechanically confirm what the LAST one
     shipped — no re-reading a narrative, no re-deriving trust. Shape
     mirrors the proven `## NEXT-SESSION VALIDATION` queue in Harmony_Main's
     own session handoff. WORKFLOW: append a row per shipped item at
     session close; run every PENDING row at next session start; match ->
     PASS and retire; mismatch -> FAIL, do not retire, fix first. Never let
     a row sit PENDING across more than one session boundary. -->

## RIG FACTS (read before writing or running any row below)

- **`--test-mode` is REQUIRED or port 8080 never binds.** Port 7070 binds
  anyway regardless of `--test-mode` — a check against 7070 is a FALSE
  GREEN. [ATTESTED HANDOFF.md, RIG MECHANICS + RIG lines]
- **Health endpoint is `http://[::1]:8080/api/health` — `::1` ONLY.**
  `127.0.0.1` returns empty and is indistinguishable from a dead server.
  Not `/api/status`. [ATTESTED HANDOFF.md, RIG MECHANICS]
- **Python with Quartz is `.venv/bin/python`, NOT system `python3`.**
  [ATTESTED HANDOFF.md, RIG + RIG MECHANICS]
- **`fps` is an INVALID detach oracle.** It's stored only inside
  `renderOpenGL()` (Renderer.cpp:181), so it FREEZES at its last value on
  detach (measured 106.18 with the context provably dead); same defect in
  `/api/status` frameTimeMs. Use the `/api/render_frame` timing oracle
  instead (200 in <0.05s = attached; ~5s then 500 = detached).
  [ATTESTED HANDOFF.md, RIG MECHANICS]
- **ctest baseline is 203/203 — RE-RUN it, never inherit the number.**
  A FORCED REBUILD must precede any ctest claim (stale-binary false-green
  is a documented failure mode). [ATTESTED HANDOFF.md — "FORCED REBUILD
  before any ctest claim; baseline 203/203, re-run it, never inherit it"]
- **SCREEN-SAFETY LAW: output-window gates are owner-attended only.**
  Never end a session with the output window open; never `pkill` the app
  while it is open (close via `Output > "Disabled"` first, let it tear
  down, then quit); verify the actual screen with `screencapture -x` and
  LOOK at the image — `pgrep` returning empty does NOT prove the screen is
  clean. [ATTESTED HANDOFF.md, "SCREEN-SAFETY LAW" section]
- AppleScript/osascript can reach JUCE's MENUS only — it cannot recurse
  into JUCE's nested AX elements for in-window controls. Use
  `tests/visual/ax_press.py` (AX title) for in-window controls.
  [ATTESTED HANDOFF.md, RIG MECHANICS]
- `~/projects/RealTimeAudio copy` is a STALE DUPLICATE REPO (HEAD f128bdc,
  Jul 11) — confirm you are in the real one before running anything below.
  [ATTESTED HANDOFF.md, RIG line]

## NEXT-SESSION VALIDATION
| item shipped | validate-command | expected | status |
|---|---|---|---|

<!-- No shipped-item rows yet — scaffold written at normalize time.
     Populate at this repo's first session close. -->

| `3736f02` recorder core + audio tap + monotonic beat timebase; `a50788b` UAF/desync fixes | `cd /Users/boriskarpman/projects/RealTimeAudio && cmake --build build --config Release --clean-first -j$(sysctl -n hw.ncpu) && cd build && ctest` | build exits 0; **306/306 pass, 0 failed** (session start was 285/285) | PASS s-rta-0923 (clean forced rebuild rc=0, 306/306) |
| `a50788b` AudioTap stop()/push() use-after-free fix | `cd /Users/boriskarpman/projects/RealTimeAudio/build && ./tests/test_audio_tap_sync --order rand` (NOTE: `ctest -R audio_tap` matches NOTHING — ctest names are test-case names; run the binary) | all pass; the concurrency case is the one that dies with SIGABRT if the fix is reverted | PASS s-rta-0923 (7 cases / 675 assertions) |
| `3736f02` totalBarCount is monotonic across a structural reset | `cd /Users/boriskarpman/projects/RealTimeAudio/build && for t in test_bpm_stabilization test_oscillator_bar_fold test_connection; do ./tests/$t; done` (the old `ctest -R` regex matched only 6 unrelated cases) | all pass; these pin the two counters DIVERGING, at oscillator AND connection-engine level | PASS s-rta-0923 (24/88, 9/486, 27/116 cases/assertions) |
| `3736f02` totalBarCount reaches the LIVE app, not just the tests | launch in PRODUCTION mode (NOT `--test-mode` — it never starts the analysis thread), then read `totalBarCount` from the signals/bpm JSON on 7070 while audio plays | field present and INCREASING; must NOT reset when a structural transition zeroes `barCount` | PASS s-rta-0923 — production launch on built-in mic, /api/bpm totalBarCount 2 -> 8 over 10 s at 154.9 BPM (~6.5 bars expected) |
| `RecorderHost` step-3 live gate (record→store→take wiring, production mode, port 7070, built-in mic, no Bluetooth) | `STEP3_BUILD_DIR=build bash .harmony/probe-step3.sh <outdir>` (the probe launches the app itself via `open -g` and REFUSES if one is already running; default build dir is `build-gate`) | all rows PASS (93 rows; 2 opt-in SKIPs are not rows); the drift/jitter rows within T2 bounds | PASS 93/0 s-rta-0926 (Harmony run) — the s-rta-0925 92/1 row "end(withAudio): inputSource == file" was probe timing: the source switches ~225 ms after Play (measured 221/230/233 ms), the probe now polls up to 1 s (8e1906d) |
| Onset render-path fix (OnsetPulse monotonic delta, no lost/duplicated pulses) | `bash .harmony/probe-onset-render.sh` | all rows PASS | PASS 13/0 s-rta-0924b |
| `downbeatDetected` is a beat-long level, not a one-hop pulse (any polling cadence) | `bash .harmony/probe-downbeat-level.sh` | all rows PASS | PASS 14/0 s-rta-0925 |
| Master Signal fader (post-analysis signal depth; pixel oracle) | `bash .harmony/probe-mastersignal.sh` | all rows PASS | PASS 22/0 (×3 runs) s-rta-0925 |
| Manual Resync (oscillators re-align to the new downbeat) | `bash .harmony/probe-resync.sh` | all rows PASS | PASS 16/0 s-rta-0925 |
| AudioTap stop() finalize-truncation regression (40-cycle stress loop) | `bash .harmony/probe-finalize-loop.sh` (or `bash .harmony/probe-finalize-loop.sh 40` to pin the cycle count) | 0 truncations across N cycles | PASS 0/40 truncations (pre-fix stderr 2/40) s-rta-0924b |
| T2 hardware timing, long take (10/20 min drift + jitter, built-in CoreAudio device only — no Bluetooth, no wired-interface run on this rig) | `STEP3_LONG=1 STEP3_LONG_MINUTES=20 bash .harmony/probe-step3.sh` | drift <= 1 ms + 2 sigma, p95 jitter <= 15 ms | PASS 10 min 75/0; 20 min 82/0, slope drift +0.28 ms (stderr 0.46) s-rta-0924b |
| `applyClipEffects` clip+layer-effect blank-frame bug (one clip effect + one layer effect at opacity 1 rendered blank) | `bash .harmony/probe-effects-parity.sh <fresh outdir>` (launches the app itself via `open -g`; decode pixels, never compare PNG hashes; use a FRESH outdir — the probe does not yet delete old PNGs) | all rows PASS, non-blank decoded frames | PASS 5/0 s-rta-0926 (Harmony run, frame looked at); parity lane 0/104 V1 frames blank over 12 launches on 4fca2c5, pre-fix control 12/12 blank (`.harmony/.reports/s-rta-0926/parity-diagnosis.md`). The s-rta-0925 "2/3 FAIL" was a misread of one run's "2 PASS / 3 FAIL"; that single all-zero run stays UNEXPLAINED (notebook s-rta-0926) |
| L-R Routines slice 1 (save/fire/restore/replay/loop/stop over REST, gesture-begin stacking; commits 58b14d7 + ebbff22) | `bash .harmony/probe-routines.sh <fresh outdir>` (launches the app itself via `open -g`, production mode port 7070, manual BPM 120 so no audio device is needed; 12 rows incl. a stacking row) | all rows PASS; `rest.png` (row 6, restore) visually matches `ref.png` within the probe's tuned `mad` bound | PASS 74/0 ×3 s-rta-0926 (Harmony, build 2bf1d56; startBeatInBar 3.47 / 3.45 / 3.80; mad(ref,rest) 0.0; rest.png looked at). RED first: 23/46 on the pre-1b binary; 69/5 on ebbff22 (grid rows +0.4 s late at startBeatInBar 3.84 = the RecorderClock stamp bug, fixed e5ceb98). Run it also with `ROUTINES_RECORD_PAUSE=1.8` (Record late in the bar). Manual-BPM runs can flake when the mic hears rhythmic sound (open bug: manual mode still phase-resets on detected beats) |
| Crossfade between two effected clips now blends (was: outgoing clip held for the whole transition, then hard-cut; ScratchPool `pickEffectTarget` fix, commit 138af1e) | `bash .harmony/probe-crossfade.sh <fresh outdir>` (launches the app itself via `open -g`; 10 cases a-j: 6 crossfades + 4 single-state; decode pixels, never hash) | all rows PASS; case a mid-frame strictly between OUT/IN references (was d(OUT,IN) with dA=0.00 pre-fix) | PASS 27/0 ×3 runs s-rta-0926 (`.harmony/.reports/s-rta-0926/xfade-report.md` section 6, VERIFIED — `runs/xfade.J8kP7u`, `xfade.xDWkwb`, `xfade.DUVzGU`); pre-fix RED confirmed on MAIN (case a/e/f/g/j FAIL, `.harmony/.reports/s-rta-0926/xfade-report.md` section 1) |
| `applyClipEffects` blank-frame fix, HARDENED (adds V5/V6 rows + a parity-to-reference row per variant; same ScratchPool fix as the crossfade row above, commit 138af1e) | `bash .harmony/probe-effects-parity.sh <fresh outdir>` (now 39 rows: V1 + V5 + V6 + parity-to-reference per variant) | all rows PASS; parity-to-reference (clip+layer frame == the same effects run as one clip chain) within tol 1.0 | PASS 39/0 ×2 on the fix, 39/0 ×1 on MAIN (stays GREEN as required) s-rta-0926 (`.harmony/.reports/s-rta-0926/xfade-report.md` section 6, VERIFIED) — supersedes the 5/0 row above, which predates hardening |

## STANDING VALIDATION COMMANDS (rig primitives — not tied to one shipped item)

| purpose | command | expected | how known |
|---|---|---|---|
| confirm real repo, not stale copy | `cd /Users/boriskarpman/projects/RealTimeAudio && git rev-list --count origin/main..HEAD` | prints a count (nothing pushed); HEAD descends from the commit named in the latest HANDOFF.md dated section | [RAN] returned `120` on 2026-09-04 |
| confirm test venv python | `/Users/boriskarpman/projects/RealTimeAudio/.venv/bin/python --version` | prints a Python 3.x version (this is the Quartz-capable interpreter — NOT system `python3`) | [RAN] returned `Python 3.14.3` on 2026-09-04 |
| build (Release) | `cmake -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build --config Release -j$(sysctl -n hw.ncpu)` | exits 0 | [ATTESTED CLAUDE.md + HANDOFF.md] — do NOT run; builds are excluded from this agent's scope |
| unit test suite | `cd build && ctest` | 203/203 pass (baseline) — MUST be re-run after a FORCED REBUILD, never inherited from a prior session's claim | [ATTESTED HANDOFF.md] — do NOT run here; requires the build step above |
| launch app in test mode (REQUIRED for 8080 to bind) | `open --stdout /tmp/adna-out.log --stderr /tmp/adna-err.log build/AudioDNA_artefacts/Release/Audio-DNA.app --args --test-mode` | process starts; port 8080 binds (7070 binding alone is a FALSE GREEN — it binds with or without `--test-mode`) | [ATTESTED HANDOFF.md, RIG MECHANICS] — do NOT run; this agent may not launch the app |
| health check | `curl -s "http://[::1]:8080/api/health"` | non-empty JSON response; `127.0.0.1` here returns empty and looks like a dead server — always use `::1` | [ATTESTED HANDOFF.md, RIG MECHANICS] |
| detach oracle (attach/detach of render context) | `curl -s -m 12 -w "%{http_code} %{time_total}" -X POST "http://[::1]:8080/api/render_frame" -d '{"output_path":"/tmp/x.png"}'` | 200 in <0.05s = attached; ~5s timeout then 500 = detached. Do NOT use `fps` or `/api/status` frameTimeMs for this — both freeze on detach at their last live value | [ATTESTED HANDOFF.md, RIG MECHANICS] |
| SignalBar drive (headless, no human) | `.venv/bin/python tests/visual/ax_press.py "▼"` (expand -> preview DETACHES) / `"▲"` (collapse -> preview REATTACHES) | preview detaches/reattaches per the detach oracle above | [ATTESTED HANDOFF.md, RIG MECHANICS] |
| output window level probe | `AUDIODNA_NO_SPAWN=1 .venv/bin/python -m pytest tests/visual/test_output_window_level.py -v` (attach with `AUDIODNA_NO_SPAWN=1`) | test reports the output window's on-screen level correctly | [ATTESTED HANDOFF.md — "RIG: `tests/visual/test_output_window_level.py`" x2] |
| 4 probe states (mapping tick) | `cd tests/visual && AUDIODNA_NO_SPAWN=1 OW_PROBE_STATE=<preview\|preview_output\|signalbar\|signalbar_output> ../../.venv/bin/python -m pytest test_mapping_tick.py -v -s` | ALL 4 states pass | [ATTESTED HANDOFF.md, RIG MECHANICS] |
| output-window menu toggle (menus only, not in-window controls) | `osascript -e 'tell application "System Events" to tell process "Audio-DNA" to click menu item "Fullscreen: 1728x1117 (main)" of menu 1 of menu bar item "Output" of menu bar 1'` | window enters fullscreen on main display | [ATTESTED HANDOFF.md, RIG MECHANICS] — OWNER-ATTENDED ONLY per SCREEN-SAFETY LAW |
| close output window (EOS, mandatory before ending any session with it open) | `osascript -e 'tell application "System Events" to tell process "Audio-DNA" to click menu item "Disabled" of menu 1 of menu bar item "Output" of menu bar 1'` then `screencapture -x /tmp/eos-screen.png` and LOOK at it | window closes; screenshot shows a clean screen, no black overlay/TCC dialog/stuck window | [ATTESTED HANDOFF.md, SCREEN-SAFETY LAW section] — OWNER-ATTENDED ONLY |
| existence checks (file targets referenced above) | `ls tests/visual/test_output_window_level.py tests/visual/ax_press.py` | both files listed, no "No such file" | [RAN] both present on 2026-09-04 |
