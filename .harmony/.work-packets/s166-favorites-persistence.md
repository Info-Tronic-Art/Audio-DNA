# S166-FAV — MilkDrop favorites and user presets never persist. Make them persist.

REPORT_FILE: /Users/boriskarpman/projects/RealTimeAudio/.harmony/.reports/s166/builder-favorites-persistence.md

## THE LANE IN ONE LINE
`ProjectMPresetManager::saveUserData()` and `loadUserData()` are fully implemented
and have **ZERO call sites anywhere in the repo** — so starring a MilkDrop preset
as a favorite, and any user preset data, is lost the moment the app closes.

## VERIFIED BEFORE THIS PACKET WAS WRITTEN (re-verify, do not trust me)
- `src/sources/ProjectMPresetManager.cpp:85` `void ProjectMPresetManager::saveUserData(const std::string& jsonPath) const`
- `src/sources/ProjectMPresetManager.cpp:111` `void ProjectMPresetManager::loadUserData(const std::string& jsonPath)`
- Declarations at `src/sources/ProjectMPresetManager.h:39-40`.
- A repo-wide grep for both names returns ONLY those four lines — definition and
  declaration, no caller. **Confirm this yourself with a SECOND grep of a different
  shape before building** (this repo has manufactured false VERIFIEDs from a single
  grep pattern; see gotchas).

## WHAT DONE MEANS
1. User data (favorites + whatever `saveUserData` serializes) is LOADED at startup
   and SAVED when it changes, so a starred preset survives a quit and relaunch.
2. The chosen on-disk location is stable, inside the app's normal user data area,
   and created if absent. Do not hardcode a path that only works on this machine.
3. A missing file on first run is NORMAL — no error, no crash, no empty-state bug.
4. A malformed or truncated file does not crash the app and does not wipe good data.
5. Saving does not stall the UI or the render thread.

## THE THREADING CONSTRAINT — READ THIS BEFORE YOU DESIGN THE SAVE
`PresetSelector::processFrame` became the FIRST GL-THREAD reader of
`presets_[].favorite` in commit `5d1e791` (session s-rta-0905, lane L7-JUKE), which
woke a race that had been dormant while every reader was message-thread paint code.
That session closed it by extending a **confinement mechanism**. Find that mechanism
(start from `5d1e791` and `PresetSelector`), understand its stated "valid while"
clause, and make sure your load/save path does not break it. A previous session's
invariant expired exactly this way once already — do not be the second.
If your change makes any NEW thread touch `presets_` or `favorite`, say so loudly in
your report; that is a finding, not a detail.

## FENCE — you may edit ONLY these
- `src/sources/ProjectMPresetManager.cpp`
- `src/sources/ProjectMPresetManager.h`
- the call-site file(s) you must touch to invoke load/save — name them in your report
  BEFORE you edit them, and keep it to the minimum
- a new or existing test file under `tests/`
Anything else: STOP and report, do not edit.

## BUILD AND TEST RULES — this repo's documented false-green traps
- **Capture the build exit code and read it BEFORE quoting any test number.** A stale
  binary passing tests is a documented failure mode here.
- ctest baseline is **222/222** as re-run on disk at s166 boot. Re-run it; never
  inherit it. Build dir is `build`, Release.
- **Prove your test is load-bearing**: neutralize your fix, rebuild, and confirm the
  new test FAILS; then restore the fix byte-identically (verify with md5) and confirm
  it passes again. Report both numbers. A test that passes with the fix removed is
  not evidence.
- Do NOT launch the app. Do NOT run the visual/AX tests. Build + ctest only.

## REPORTING
Write the report to the REPORT_FILE path above with: STATUS (DONE / DONE_WITH_CONCERNS
/ BLOCKED), what you changed and why, the build exit code, the before/after ctest
numbers, your load-bearing proof, PACKET QUALITY (was this packet right? what did it
get wrong?), and anything you found that is outside the fence.
