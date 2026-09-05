# Lane 7 — docs-tests-build audit (2026-09-05)

Scope: root `*.md`, `docs/`, `tests/`, `src/test`, `cmake/`, `CMakeLists.txt`.
Baseline: `9139dd4` (2026-07-16 renorm) → `HEAD`.

## 1. Scope of drift

Correct pathspec (a naive `git log -- '*.md'` over-matches recursively and
pulls in every `.harmony/*.md`, which is NOT this lane — had to restrict to
the actual root files):

```
git log --oneline 9139dd4..HEAD -- CLAUDE.md CONTEXT.md PHASE_GUIDE.md README.md \
  THIRD_PARTY_LICENSES.md docs/ tests/ src/test cmake/ CMakeLists.txt | wc -l
```
→ **37 commits** touch lane paths (of the ~127 total since baseline).
Breakdown: root *.md 3, docs/ 1, tests/ 30, src/test 2, cmake+CMakeLists 12
(some commits touch more than one bucket, so buckets don't sum to 37).

`git diff --stat 9139dd4..HEAD -- <lane paths>`: **45 files changed,
7225 insertions(+), 305 deletions(-)**. Biggest single deltas:
`tests/test_undo_commands.cpp` (+2222, new file), `tests/visual/test_output_window_level.py`
(+1325, new file), `CLAUDE.md` (+232/-?), `tests/CMakeLists.txt` (+196/-?, mostly
new test-target blocks), `CMakeLists.txt` (+202/-?).

## 2. Recounted numbers

| What | Doc says | Source says | Changed? | How counted |
|---|---|---|---|---|
| Catch2 unit test **targets** | CLAUDE.md L291: "12 Catch2 unit targets" | **17** | yes | `grep -c 'add_executable(test_' tests/CMakeLists.txt` → 17 unconditional Catch2 executables (`test_ring_buffer, test_waveform_snapshot, test_renderer_source_confinement, test_spectral_features, test_feature_bus, test_smoother, test_integration_pipeline, test_mapping_engine, test_bpm_stabilization, test_downbeat_detector, test_composition, test_undo_commands, test_routing_engine, test_thumbnail_cache, test_compositor, test_autopilot, test_preset_manager`). An 18th, `syphon-check`, is a non-Catch2 CLI gated behind `AUDIODNA_BUILD_SYPHON`. |
| Total tests (ctest) | CLAUDE.md L291: "114 tests, all pass" | **203** (task-given, independently reconciled) | yes | `grep -rn 'TEST_CASE' tests/*.cpp \| wc -l` → 202 Catch2 `TEST_CASE`s (no `TEMPLATE_TEST_CASE`/`SCENARIO` in the suite, so 1 TEST_CASE = 1 ctest entry via `catch_discover_tests`) + 1 non-Catch2 `add_test(NAME syphon_check_negative ...)` in `tests/CMakeLists.txt` = **203**, matching the run count given in the task. |
| Test files listed in CLAUDE.md's tree vs. on disk | CLAUDE.md tree (L292-306) lists 12 `.cpp` files | **17 exist** in `tests/*.cpp` | yes | `ls tests/*.cpp` — tree is missing `test_undo_commands.cpp` (59 cases, the single largest test file, 2222 lines), `test_thumbnail_cache.cpp` (5 cases), `test_autopilot.cpp` (4 cases), `test_preset_manager.cpp` (10 cases), `test_renderer_source_confinement.cpp` (2 cases). |
| `tests/visual/` pytest files | CLAUDE.md L306: "11 test_*.py" | **13** | yes | `ls tests/visual/test_*.py \| wc -l` → 13 (adds `test_mapping_tick.py`, `test_output_window_level.py` since baseline; also new non-`test_` helper `vj_controller.py` and `ax_press.py` in the same dir, not counted in the 13). |
| "Eyes" TestServer REST endpoints | CLAUDE.md L299 + FEATURES.md L1335 imply **17** | **19** | yes | `grep -n '\.Get(\|\.Post(' src/test/TestServer.cpp` → 19 distinct routes registered (`/api/health, load_image, set_effect, set_effect_chain, inject_features, render_frame, state, reset, load_source, update_source_params, sources, load_milkdrop_preset, signals, add_route, remove_route, routes, set_macro, add_mapping, remove_mapping`). Confirmed with a second, narrower pattern to rule out over-count. `src/test/TestServer.cpp` grew +140/-? lines in-scope, consistent with new endpoints landing. |
| Production `ApiServer` REST endpoints (port 7070) | CLAUDE.md: "22 endpoints — all functional" | **24** | yes | `grep -c '\.Get(\|\.Post(' src/api/ApiServer.cpp` → 24. Flagging for completeness since the count sits in CLAUDE.md (my lane), but `src/api/` itself is lane-6 (io-api) territory — defer to that lane's finding if it already caught this. |
| Effect count self-consistency | Prior norm found CLAUDE.md internally contradicting itself at 110/115/135/96 | **135, consistently, everywhere checked** | no (fixed) | `grep -n -E '\b(110\|115\|135\|96)\b effects' CLAUDE.md` and a scan of every "135" occurrence in CLAUDE.md (intro, tree comment ×2, "Effect Categories (135 total)", the 2026-03-23 audit note) — all say 135, none say 110/115/96. **This specific prior defect class is fixed.** |
| `src/` subdirectory listing completeness | Prior norm found CLAUDE.md's tree naming a nonexistent `src/keyboard/` while omitting `src/core, src/test, src/routing, src/signal, src/sync, src/binding, src/midi, src/model` | **All 8 present; no `src/keyboard/` anywhere** | no (fixed) | `ls src/` (18 real subdirs) vs `grep -n 'src/' CLAUDE.md` tree block (L190-282) — every one of `core, test, routing, signal, sync, binding, midi, model` has its own tree entry now; `grep -n 'keyboard' CLAUDE.md` only matches prose about the *retired v1 concept*, never a tree/path entry. **This specific prior defect class is also fixed.** |

## 3. Drift findings (most severe first)

### FINDING 1 — CRITICAL — FEATURES.md's own top-of-file summary says Undo/Redo is a no-op; it has shipped and is user-reachable by keyboard shortcut

- **FEATURES.md says:** header line 7 (the "Synced 2026-07-17" note, still standing, unedited since): *"Undo/redo remains a no-op (Wave 2)."* Section 8 body text is inconsistent with its own header — it describes `UndoManager` as live ("Command pattern, records state changes, supports undo/redo chain") but never corrects the header claim, and section 8's own "Test coverage" bullets don't mention any undo test file at all.
- **Source says:** Undo v1 (9 steps) shipped and is wired end-to-end: `Cmd/Ctrl+Z` = Undo, `Cmd/Ctrl+Shift+Z` = Redo (`src/MainComponent.cpp:2326` region), dynamic "Undo <description>"/"Redo <description>" menu text via `menuBarModel_->getUndoState`/`getRedoState` (`MainComponent.cpp:1573-1579`), `undoManager_.perform(...)` called live from `MainComponent.cpp:3606` and `:3613`. Five concrete `Command` subclass files exist and are non-empty: `src/core/ClipCommands.h` (3 subclasses), `DeckCommands.h` (11), `EffectCommands.h` (1), `TriggerCommands.h` (1), `CompositeCommand.h` (1) — directly contradicting `.harmony/file_map.md`'s separate claim (see Finding 3) that there are "zero Command subclasses."
- **Evidence:** `grep -n 'Undo\|Redo' src/MainComponent.cpp` around L1573-1579, L2326; `grep -rln ': public Command' src/core/*.h`; `git log --oneline 9139dd4..HEAD` shows the whole Undo v1 arc (`7c8d286` plumbing → `0a1c882` step-9 cleanup, 9 feat/test commits) landed inside the audit window. `.harmony/APP-INVENTORY.md:266` independently corroborates: *"Undo / redo | BUILD-COMPLETE 2026-07-19→25 ... Cmd+Z + dynamic menu live; tests 170."*
- This is CRITICAL by the stated rubric: FEATURES.md asserts something false about behavior a user can reach directly from the keyboard.

### FINDING 2 — MAJOR — `.harmony/file_map.md` documents nine symbols as present (with detailed dead/ghost/stub annotations) that were deleted before the file was even written

- **file_map.md says** (all still in the live file today): `GenreSmoothing.h` DEAD-but-present; `Smoother.h`'s `OneEuroFilter` DEAD-but-present; `MappingSuggester.h/cpp` GHOST-but-present; `ChainedSignal.h/cpp` GHOST-but-present; `UniformBridge.h/cpp` present (as "Params → glUniform calls"); `src/output/SyphonInput.h/.mm` ORPHANED-but-present; `SpoutOutput.h`/`NdiOutput.h`/`NdiInput.h` STUBS-but-present; a `shaders/*.frag/*.vert` directory of "5 disk files."
- **Source says:** none of these exist. `ls`/`test -e` on all nine paths returns MISSING; `shaders/` the directory does not exist (`ls shaders/` → No such file or directory); `grep -rn 'OneEuroFilter' src/` → zero hits anywhere in the tree.
- **Evidence:** direct existence checks on the 9 paths above, all MISSING; `ls shaders/` → `No such file or directory`.
- **Why this is worse than ordinary drift:** `git log --diff-filter=A -- .harmony/file_map.md` shows the file was **authored 2026-07-30** (`6777667`), i.e. two weeks *after* the "Wave 0" deletion pass (2026-07-17) that removed exactly these nine symbols — and it has never been touched since. It was stale on the day it was written, describing a pre-Wave-0 snapshot of the tree. The prior report's "flagged STALE" was correct but understated: this isn't drift-since-last-touch, it's wrong-from-birth.
- Also stale in the same file, same severity class: it says `OscHandler` is "INERT (`startListening()` never called ...only 5/11 callbacks wired)" — actually LIVE since Wave 1-B (`startListening(kOscListenPort)` called at `MainComponent.cpp:1755`, 11/11 wired per CLAUDE.md/FEATURES.md and unchallenged by any contrary grep); says `SyphonOutput` is "DEAD-WIRED (`publishTexture()` never called)" — actually called live from `src/render/Renderer.cpp:1907`; says `ApiServer`'s `/api/set_bpm` is "a no-op stub" — actually wired per the Wave 0 sync note and unchanged since; says Tests are "11 Catch2 unit test targets, 113 tests" — actual is 17/203 (see recount table).
- **Net verdict on file_map.md: essentially every substantive claim in it about dead/ghost/stub status is now wrong, in the "actually fixed" direction.** Its only correct-and-current line is the throwaway note that `KeyboardPanel`/`KeyEditor` don't exist and CLAUDE.md shouldn't list them — and CLAUDE.md no longer does (see recount table, "fixed" row), so even that note is now obsolete. This file should be either fully rewritten from source or deleted; a reader trusting it today would believe undo/redo, OSC, and Syphon output are all broken when none of them are.

### FINDING 3 — MAJOR — FEATURES.md's test-coverage bullets omit 5 whole test files (80 of the suite's 202 TEST_CASEs, including its single largest file)

- **FEATURES.md says:** across every "Test coverage:" bullet in the file (checked with `grep -n <filename> .harmony/FEATURES.md` for each), the following files are named **zero times**: `test_undo_commands.cpp` (59 cases — Undo v1, GL-fenced), `test_preset_manager.cpp` (10 cases — the dual-key effect/param retargeting fix, commit `57aa436`), `test_thumbnail_cache.cpp` (5 cases — FilesBrowser LRU cache), `test_autopilot.cpp` (4 cases — End-of-Video/On-Beat/column-selection), `test_renderer_source_confinement.cpp` (2 cases — owner-thread-confined map for `activeSources_`, commit `22fcedc`).
- **Source says:** all 5 files exist, compile, and are wired into `tests/CMakeLists.txt` via `add_executable`/`catch_discover_tests`.
- **Evidence:** `for t in test_undo_commands test_thumbnail_cache test_autopilot test_preset_manager test_renderer_source_confinement; do grep -n "$t" .harmony/FEATURES.md; done` → no output for any of the five.
- This means anyone reading FEATURES.md's per-feature "Test coverage" bullets to gauge what's tested will materially undercount coverage for Composition/Undo (§8), Sources/Media persistence, FilesBrowser, and the render-thread source-confinement hardening — the opposite failure mode from the usual "claims tested, isn't," but still a MAJOR accuracy gap since it makes the suite look thinner and less concurrency/regression-focused than it is.
- One partial mitigating note: §16 line 1442 says "no smart autopilot behavior tests" — checked `test_autopilot.cpp`'s actual `TEST_CASE` titles (End-of-Video mode, On-Beat mode, column selection) and none of them exercise genre-driven/energy-aware "smart" autopilot specifically, so that particular claim is *not* falsified, just adjacent to a file the doc never mentions.

### FINDING 4 — MINOR — `cmake/Sanitizers.cmake` (new since baseline) is fully wired but undocumented anywhere in FEATURES.md

- **FEATURES.md says:** nothing — `grep -n -i 'sanitiz\|asan\|ubsan\|tsan' .harmony/FEATURES.md` returns zero hits in the whole 2723-line file, including its "Shared Infrastructure / Build System" section (L2717-2721) which enumerates CMake/Catch2/Eyes but not this.
- **Source says:** it's real and fully wired, not a stub. `cmake/Sanitizers.cmake` defines `ADNA_SANITIZE` (CACHE STRING: `address`/`undefined`/`thread`, mutually-exclusive-thread-vs-address enforced via `FATAL_ERROR` at configure time, MSVC rejected) and an `apply_sanitizers(<target>)` function; `CMakeLists.txt:12` includes it, `CMakeLists.txt:518` applies it to the app target, and `tests/CMakeLists.txt` applies it to **all 17** Catch2 test executables (one `apply_sanitizers(...)` call per target, verified by grep).
- **Evidence:** `grep -rn 'apply_sanitizers(' CMakeLists.txt tests/CMakeLists.txt` → 18 call sites (1 app + 17 tests); `git log --oneline 9139dd4..HEAD -- cmake/` shows `ba0ae70 Add ASan/UBSan/TSan build-variant wiring (ADNA_SANITIZE)` in-window.
- Severity is MINOR (not user-facing behavior, dev-only build tooling) but it's a real, non-trivial capability with zero documentation footprint — worth a line in "Shared Infrastructure / Build System."

### FINDING 5 — MINOR — CLAUDE.md's `tests/` tree comment undercounts both targets and total tests (see recount table rows 1-2)

Folded into the recount table above; restated here only for severity bookkeeping. `CLAUDE.md:291` — *"12 Catch2 unit targets (114 tests, all pass; run: `cd build && ctest`)"* — actual is 17 targets / 203 tests (as run and reconciled in §2). Same line also lists only 12 of the 17 `.cpp` files in the tree below it (missing the 5 named in Finding 3) and says `tests/visual/` has "11 test_*.py" against an actual 13.

## 4. Dead surfaces traced

None found *newly* dead in this lane's paths. The one control-like surface I traced here — `ADNA_SANITIZE` (a CMake cache var, not a UI control) — has a real consumer: `apply_sanitizers()` is invoked on every build target that should carry it (app + all 17 test executables), so it is NOT a dead surface; flagging it above only for being undocumented, not for being unwired.

I did not find a "toggle with no consumer" pattern anywhere in my own lane's files (tests/CMakeLists.txt, cmake/, src/test) — that defect class, per the task brief, is more likely to live in UI/control-plane lanes (2/4/5), which is out of my scope. I looked specifically because the task flags it as the recurring defect class, and came up empty inside my paths.

## 5. Could not determine

- Whether `syphon-check` (the 18th, conditional CMake target in `tests/CMakeLists.txt`, gated on `AUDIODNA_BUILD_SYPHON`) is exercised in CI — `.github/workflows/build.yml` exists (checked `ls -la .github/workflows/`) but I did not parse its build-flag matrix to confirm `AUDIODNA_BUILD_SYPHON`/`AUDIODNA_BUILD_TEST_SERVER`/`ADNA_SANITIZE` are ever turned on in CI vs. only locally. Flagging as unresolved rather than guessing what CI covers.
- Whether the ApiServer 22→24 endpoint recount (production REST API, `src/api/`) has already been caught by lane 6 (io-api) — that file is outside my assigned paths, I only found the discrepancy incidentally while auditing CLAUDE.md text. Reported once here for completeness; may be a duplicate of a lane-6 finding.
- Whether `.harmony/APP-INVENTORY.md`'s own "188/189 unit tests" running count (its own internal history log, L31) was ever reconciled to the current 203 — I did not audit APP-INVENTORY.md line-by-line since it's not in my path list; noted only because it happens to independently corroborate the Undo v1 wiring in Finding 1.
