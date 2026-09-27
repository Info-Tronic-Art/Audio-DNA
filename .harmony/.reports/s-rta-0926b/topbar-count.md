# s-rta-0926b -- LANE Y (topbar-count) report

Branch: `lane/topbar-count-0926b`. Base: `main` @ `fdf46b9` (one docs-only commit past the plan's
pinned `50b4bce` -- `git diff --stat 50b4bce fdf46b9` touches only `.harmony/s-rta-0926b-work.md`,
so every cite in `plan3-final.md` section 3 (ITEM B) verified line-for-line against `fdf46b9`
without re-anchoring: `TopBar.cpp:315-316` (the `timerCallback` copy), `:521-547`
(`paintBarPhraseDisplay`), `:580-581` (the `resized()` comment/bounds), `tests/CMakeLists.txt:1384-
1409` (`test_routine_bank_model`'s CMake block, cloned), `.harmony/APP-INVENTORY.md:56`. All
matched exactly.

Executed ITEM B only (section 3 + section 5 lane Y row). Two commits:
- `b79db9c` -- the source + test change.
- `8e4f038` -- UI evidence (screenshots).

## Files touched (exactly the plan's fence, verified by `git diff --stat fdf46b9 HEAD`)
- `src/ui/TopBarModel.h` (new) -- `kBarsPerCount = 4`; `barReadoutText(bool hasBpm, uint16_t
  barCount)` pure function, juce_core only (the `RoutineBankModel.h` posture).
- `src/ui/TopBar.cpp` -- `#include "TopBarModel.h"`; removed the orphan `displaySnap_.phrasePhase
  = snap.phrasePhase;` copy in `timerCallback`; `paintBarPhraseDisplay` now paints ONE line via
  `barReadoutText(hasBpm, displaySnap_.barCount)` at 11pt, `centredLeft` (horizontally left,
  vertically centred per JUCE's `Justification::centredLeft`) in the full `barPhraseBounds_` --
  the old two-line "Bar N" / "Phr 0.XX" split is gone; the `resized()` comment above
  `barPhraseBounds_ = area.removeFromLeft(44)` is now "Bar readout (Bar 1..4)" (bounds/44px
  unchanged).
- `tests/test_topbar_model.cpp` (new) -- pins `barReadoutText(true,0)=="Bar 1"`,
  `(true,3)=="Bar 4"`, `(true,4)=="Bar 1"`, `(true,7)=="Bar 4"`, `(true,65535)=="Bar 4"`,
  `(false,9)=="Bar -"`.
- `tests/CMakeLists.txt` -- one new block for `test_topbar_model`, cloned verbatim from
  `test_routine_bank_model`'s (juce_core only, never juce_gui_basics).
- `.harmony/APP-INVENTORY.md:56` -- "beat wheel + bar/phrase + FPS/DSP readouts" ->
  "beat wheel + bar-in-four + FPS/DSP readouts".

Nothing else touched. `MainComponent.*`, `RoutineEngine.*`, `BPMTracker.*`, LookAndFeel files
untouched (grep of the diff confirms -- see stat above, 5 files only).

## RED-then-GREEN
- RED (compile, on base `fdf46b9`): `git show fdf46b9:src/ui/TopBarModel.h` and
  `git show fdf46b9:tests/test_topbar_model.cpp` both fail with "path exists on disk, but not in
  'fdf46b9'" -- confirmed neither file existed pre-change, so `#include "ui/TopBarModel.h"` in the
  new test would not compile. This matches the plan's own RED definition (B.4: "RED: the header
  does not exist (compile)").
- GREEN: full `cmake --build build-lane -j3` (cold, exit 0, see below), then serial `ctest` in
  `build-lane`: **601/601 tests passed** (0 failed), `Total Test time (real) = 10.45 sec`. The two
  new cases:
  ```
  Start 546: barReadoutText wraps 1-2-3-4-1
  546/601 Test #546: barReadoutText wraps 1-2-3-4-1 ... Passed 0.00 sec
  Start 547: barReadoutText with no BPM reads Bar -
  547/601 Test #547: barReadoutText with no BPM reads Bar - ... Passed 0.00 sec
  ```
  `test_topbar_link_toggle` (Test #589, "default build: the TopBar Link toggle is disabled,
  dimmed, ...") **stays GREEN**: `589/601 ... Passed 0.05 sec`. 601 = the wave-2-gate's documented
  599 (docs commit `fdf46b9`: "ctest 599/599") + the 2 new `test_topbar_model` cases.

## Build
`cmake -S . -B build-lane -DCMAKE_BUILD_TYPE=Release` with the four `FETCHCONTENT_SOURCE_DIR_*`
overrides from the rig rules -- configured clean (one pre-existing unrelated `FetchContent_Populate`
deprecation warning for `syphon`, not from this change). `cmake --build build-lane -j3` -- cold
build, ~14 min wall clock this run, exit 0, 100% built. `build-lane/` kept per instructions (not
deleted, not committed -- untracked, matches the rig's existing `.gitignore` posture for other
`build-*` dirs which also aren't all listed there).

## UI EVIDENCE (`.harmony/.reports/s-rta-0926b/topbar-count-shots/`, committed `8e4f038`)
All captures window-only via Quartz `CGWindowListCopyWindowInfo` (owner "Audio-DNA", layer 0,
name != "Output") -> `screencapture -l<id> -o -x`; app launched `open -g` only; quit via
`osascript -e 'tell application "Audio-DNA" to quit'`, confirmed dead by `pgrep`, no pkill needed
either time. Live-app lock (`/tmp/audiodna-live.lock`) acquired as `topbar-count` after `probehygiene2`
then `uitoggle` held and released it (polled 20s, ~13 min wait, well under the 45 min cap),
released by me at the end (`owner` file confirmed mine before `rm -rf`). `.venv` symlink created for
the capture run, removed before every commit (confirmed absent before both commits).

- `before-main-full.png` / `before-main-topbar-crop.png` -- **pre-change** app
  (`/Users/boriskarpman/projects/RealTimeAudio/build/AudioDNA_artefacts/Release/Audio-DNA.app`,
  read-only, never touched), `POST /api/set_bpm {"bpm":120}`, tempo running ~15s
  (`barCount` 7->32 across the session). Crop shows the pre-change TWO-line readout: **"Bar 13" /
  "Phr 0.58"** plus the beat wheel and "Tempo 120".
- `after-lane-full.png` / `after-lane-topbar-crop.png` -- **build-lane** app (this lane's build),
  same `set_bpm 120`. Crop shows the new ONE-line readout: **"Bar 4"**, no "Phr" text anywhere,
  font fits the 44px column at 11pt with no visible clipping.
- `after-lane-seq-{1..5}-barCount{N}-expectBar{M}-{full,crop}.png` + `after-lane-seq-strip.png` --
  one frame per bar across 5 consecutive bars, captured by polling `/api/bpm` for each `barCount`
  edge (0.15s settle after the JUCE 15Hz timer, well under the bar length) and screenshotting
  immediately: `barCount` 49/50/51/52/53 (`beatInBar==0`, `barPhase` ~0.08 at each capture --
  confirmed still inside the just-started bar, no double-advance). Read from the pixels
  (`after-lane-seq-strip.png`, all 5 crops concatenated): **"Bar 2", "Bar 3", "Bar 4", "Bar 1",
  "Bar 2"** -- the 4-to-1 wrap is directly visible mid-sequence, proving the 1-2-3-4-1 cycle live
  (`49%4+1=2, 50%4+1=3, 51%4+1=4, 52%4+1=1, 53%4+1=2` -- pixels match the formula exactly).
  (First attempt at this sequence used a 0.3s settle and mis-captured item 1 one bar late due to
  screencapture + sleep overhead crossing a 2.0s bar boundary near the edge; re-shot with a
  throwaway initial read + 0.15s settle to fix it -- kept only the corrected set.)

Absolute paths (all under
`/Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/wf_156f9cfa-a36-2/.harmony/.reports/s-rta-0926b/topbar-count-shots/`):
- `before-main-full.png`
- `before-main-topbar-crop.png`
- `after-lane-full.png`
- `after-lane-topbar-crop.png`
- `after-lane-seq-1-barCount49-expectBar2-full.png`
- `after-lane-seq-1-barCount49-expectBar2-crop.png`
- `after-lane-seq-2-barCount50-expectBar3-full.png`
- `after-lane-seq-2-barCount50-expectBar3-crop.png`
- `after-lane-seq-3-barCount51-expectBar4-full.png`
- `after-lane-seq-3-barCount51-expectBar4-crop.png`
- `after-lane-seq-4-barCount52-expectBar1-full.png`
- `after-lane-seq-4-barCount52-expectBar1-crop.png`
- `after-lane-seq-5-barCount53-expectBar2-full.png`
- `after-lane-seq-5-barCount53-expectBar2-crop.png`
- `after-lane-seq-strip.png`

## Fence compliance
`git diff --stat fdf46b9 HEAD` (code commit) touches exactly: `.harmony/APP-INVENTORY.md`,
`src/ui/TopBar.cpp`, `src/ui/TopBarModel.h` (new), `tests/CMakeLists.txt`,
`tests/test_topbar_model.cpp` (new). NOT touched: `MainComponent.*`, `RoutineEngine.*`,
`BPMTracker.*`, any LookAndFeel file, `ApiServer.cpp`, `RoutineBankModel.h`, `probe-step3.sh`,
`CLAUDE.md`. No probe rows were added or edited (ITEM B has no probe-hygiene2-owned files in its
fence; the concurrently-editing lane's files were never touched).

## Sacred rules
No audio/analysis thread code touched (UI-only change, message thread `TopBar::paintBarPhraseDisplay`
and `timerCallback`, both pre-existing methods).

## Rig compliance checklist
- [x] Isolated worktree, new branch `lane/topbar-count-0926b` from `main` (base `fdf46b9`).
- [x] Built in `<worktree>/build-lane` with the four `FETCHCONTENT_SOURCE_DIR_*` overrides; never
  touched the read-only `/Users/boriskarpman/projects/RealTimeAudio/build`.
- [x] Multi-step shell sequences run via `bash <file>`.
- [x] `.venv` symlink created for probe/screenshot runs, removed before every commit.
- [x] Live-app lock acquired/released correctly (owner file matched `topbar-count` before removal).
- [x] App launched `open -g` only; quit via `osascript`; no lldb/gdb/dtrace/Instruments; no
  full-screen capture (window-only via Quartz window id); no synthetic input; no unexpected system
  dialog encountered.
- [x] New test RED on pre-change code first (compile-absence proof via `git show`), then GREEN.
- [x] Full ctest serial count: 601/601 passed.
- [x] One commit per plan item (source+test, then evidence) + this report, `git add -f` for every
  path under the gitignored `.harmony/*`.
- [x] Did not merge or push.
- [x] `git status` clean of tracked-file changes (only `build-lane/` untracked, kept per
  instruction).
- [x] Live-app lock released, no Audio-DNA process left running.

STATUS: COMPLETE.
