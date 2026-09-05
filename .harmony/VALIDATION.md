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
