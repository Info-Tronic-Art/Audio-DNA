# s-rta-0926b lane bpm2: Builder report

STATUS: DONE
RESULT: All three decisions are implemented. (1) LINK-RAMP (b): a tempo that comes from Link never realigns the beat phase, whether it changed or not. (2) In a default build, Link is honestly unavailable. LinkSync never reports enabled and never hands out a tempo. The TopBar Link toggle stays in the same place but is disabled, dimmed, and has the tooltip "Ableton Link is not included in this build". (3) The nits are fixed: the exact-float `tempoChanged` comparison is deleted, the stale test comment is fixed, and the docs Link section now matches the code. Every new test was RED on main and is GREEN on the fix. Full serial ctest passes, and the three live probes are GREEN.
FACTS:
- RED on main 5f84899 (build-lane before the fix; logs in scratchpad `run-red-*.txt`):
  - LINK-RAMP test: `Link ticks 60, tempo steps applied 59, phase jumps 59 (first at hop 4)`, failing `REQUIRE( jumps == 0 )`.
  - Existing Link test tail (the changed tempo 128 now has to stay continuous): failed `REQUIRE_FALSE( phaseJumped(...) )`.
  - test_link_sync: `UI ticks 60, Link tempos fed 60, isEnabled 1, getBPM 120, tracker bpm 120, manual 1`, failing `REQUIRE_FALSE( link.isEnabled() )`.
  - test_topbar_link_toggle: `bounds 682 2 50 36, enabled 1, alpha 1, tooltip "Sync tempo with other Ableton Link-enabled apps on the network"`, 5 failed. The failures were isEnabled, alpha, tooltip, a click firing onLinkToggled, and the toggle ending up checked.
- GREEN on the fix: test_bpm_stabilization `[s-rta-0926b]` `All tests passed (39 assertions in 4 test cases)`, test_link_sync `All tests passed (10 assertions in 1 test case)`, test_topbar_link_toggle `All tests passed (9 assertions in 1 test case)`.
- Full serial ctest on build-lane: `100% tests passed, 0 tests failed out of 591`.
- Live runs on the build-lane app (HEAD source), holding `/tmp/audiodna-live.lock` as bpm2 for each run and releasing it after: probe-manual-bpm `18 PASS / 0 FAIL`, probe-resync `16 PASS / 0 FAIL`, probe-downbeat-level `14 PASS / 0 FAIL`.
- Compile check with `-DAUDIODNA_BUILD_LINK=ON` (scratch dir, target AudioDNA, JUCE/Catch2/httplib/Syphon from local _deps): `CONFIGURE EXIT 0`, `BUILD EXIT 0`, 115 s. `AUDIODNA_HAS_LINK=1` appears in flags.make, and TopBar.cpp.o, LinkSync.cpp.o, MainComponent.cpp.o and BPMTracker.cpp.o were all built. The scratch dir has been deleted.
- UI screenshots: window-only captures of the main window (Quartz id, `screencapture -l<id> -o -x`), plus TopBar crops. I diffed the before and after crops. The only changed pixels are x 1379-1468 (retina px, the Link toggle); every other TopBar control is identical.
METHOD: I read the wave-1 report and review, BPMTracker's request path, LinkSync, every Link site in MainComponent, and TopBar's toggle and resized(). I grepped all of src for any Link route or persisted state. I wrote the tests first and ran them RED on unchanged main. Then I implemented the fix and ran GREEN, the full build and serial ctest, the screenshots (before on /Users/boriskarpman/projects/RealTimeAudio/build/..., after on build-lane), the Link-ON compile, and the three live probes.
CONFIDENCE: high. VERIFY: re-run the three new tests and the probes on the merged build, and look at the two crops.
UNKNOWNS / NOT DONE:
- There is no LIVE row for toggling Link. Link cannot be toggled over REST or OSC: a grep of src/api, src/osc, src/midi, src/binding, src/model, src/core, src/recording and src/test finds no Link route. The TopBar toggle is the only way to reach it, and synthetic clicks are forbidden.
- The Link-ON build was compile-checked only. Real Link peers were not exercised.
- The tooltip text is asserted in the unit test but cannot be seen in a screenshot, because showing it needs a mouse hover and synthetic input is forbidden.
NUANCE:
- `tempoChanged` (nit 3): after ruling (b) the comparison has no job left, so I deleted it rather than switching it to the 0.01 BPM epsilon. Only `realign` (setManualBPM = Tap / set_bpm) zeroes the phase now.
- `getBPM()` now returns 0 whenever Link is not enabled. This matches its documented contract ("Returns 0 if Link is not active"). Its only caller sits inside MainComponent's `isEnabled()` gate, so a Link-ON build behaves the same.
- Persistence: the Link on/off state is not saved anywhere, so every launch starts with it off. I grepped all of src and found no Link key in settings, presets, compositions, takes, bindings or the menu. There is no saved "on" to load, and even a direct `setEnabled(true)` is ignored in a default build (test_link_sync). Takes do record tempo points with action "link", but those come from REST/OSC set_bpm (and from Link ticks in a Link-ON build). They are tempo values, not the enabled state, and their replay (setManualBPM, realigns) is unchanged.
- The TopBar decides the toggle's state with `LinkSync::isAvailable()`, a constexpr taken from the same `AUDIODNA_HAS_LINK` define, so there is one source of truth. That define is PRIVATE to the AudioDNA target. Test targets never define it, so they always see the default build.
- The disabled look follows the Record panel's disabled controls: `setEnabled(false)` + `setAlpha(0.4f)` (its `kDisabledAlpha`). The LookAndFeel toggle painter is not changed.
HANDOFF-NEEDS: none

## Files changed (branch lane/bpm-thread-0926b, commits 82e8ddd, ae0548b, b5b4d24, plus this report)
- `src/analysis/BPMTracker.cpp`: `applyTempoRequest` zeroes the phase only `if (realign)`, and `tempoChanged` is removed. `BPMTracker.h`: the followExternalTempo and kTempoRealign comments now describe ruling (b).
- `src/sync/LinkSync.h/.cpp`: new `static constexpr bool isAvailable()`. `setEnabled` stores `enabled && isAvailable()`. `getBPM()` returns 0 unless enabled. A header comment covers the default build and persistence.
- `src/ui/TopBar.cpp`: includes `sync/LinkSync.h`. When `!LinkSync::isAvailable()`, the toggle is disabled, set to 0.4 alpha and given the tooltip. `resized()` is untouched, so the bounds are the same.
- `src/MainComponent.cpp` (Link lines only): a comment on the timer's Link block, and a comment at the followExternalTempo call site.
- `tests/test_bpm_stabilization.cpp`: a new LINK-RAMP case, and the existing Link test's changed-tempo tail now asserts continuity.
- `tests/test_link_sync.cpp` (new) and `tests/test_topbar_link_toggle.cpp` (new), with both targets added in `tests/CMakeLists.txt`.
- `tests/test_downbeat_detector.cpp:191`: the stale comment is fixed; setManualBPM is a request applied at the first hop.
- `docs/claude/performance-controls.md`: the Link section now covers the only entry point, no persistence, tempo-only behaviour that never realigns, phase-following not implemented, and the default-build behaviour.

## Tests (RED -> GREEN, verbatim)
- RED (main): test_bpm_stabilization `test cases:  4 |  2 passed | 2 failed`; test_link_sync `test cases: 1 | 1 failed`; test_topbar_link_toggle `test cases: 1 | 1 failed` / `assertions: 8 | 3 passed | 5 failed`.
- GREEN: see FACTS. Serial ctest: `100% tests passed, 0 tests failed out of 591`.
- The TopBar bounds assertion `(682, 2, 50, 36)` at 1728x40 is a guard that there is no layout change. Its value was read from the pre-change TopBar during the RED run, so by design it passes on main too.
- test_link_sync uses the real LinkSync and the real BPMTracker. MainComponent.cpp cannot be linked into a unit test, so the test performs the timer's call sequence itself (isEnabled -> update -> getBPM -> manual + followExternalTempo). This is labelled in the file.

## Live gates (build-lane app; lock per run, released after; the .venv symlink was removed afterwards)
- probe-manual-bpm: `18 PASS / 0 FAIL`
- probe-resync: `16 PASS / 0 FAIL`
- probe-downbeat-level: `14 PASS / 0 FAIL`
- No render claims are made, so no frames were decoded. No Output window was opened, and no full-screen capture, debugger or synthetic input was used. There were no system dialogs.

## UI evidence (shots)
- /Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/wf_41d6317f-a47-2/.harmony/.reports/s-rta-0926b/bpm2-shots/before-main-topbar-link.png: the main build, with Link a live, bright toggle.
- /Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/wf_41d6317f-a47-2/.harmony/.reports/s-rta-0926b/bpm2-shots/after-lane-topbar-link.png: the lane build, with Link dimmed and disabled in the same spot.
- The `-crop.png` versions of both show the TopBar strip (Tap / Resync / Manual / Link / multipliers).

## Found, not fixed (outside the fence)
1. `src/MainComponent.h:490-492`: the comment "an unchanged tempo then does not realign" is still true, but it is incomplete after ruling (b), since a changed tempo does not realign either. It is a one-line comment, but MainComponent.h is outside the fence.

## Open forks
- none new. Same-value set_bpm remains Boris's open question and is unchanged here.

## Notebook note (for Harmony to fold)
`## 2026-09-26 s-rta-0926b bpm2 | Files: src/sync/LinkSync.h, src/ui/TopBar.cpp | AUDIODNA_HAS_LINK is a PRIVATE define of the AudioDNA target only; every test target sees the default (no-Link) build. LinkSync::isAvailable() is the single switch (TopBar reads it). Headless JUCE Button clicks: Button::mouseDown/Up are protected, so call them through a juce::Component& to simulate a click in a test. Valid while LinkSync keeps isAvailable().`

## Packet quality
- Clarity: CLEAR.
- Missing context: none that blocked the work. The packet's "LIVE row if Link can be toggled over REST/OSC" does not apply, because no such route exists (grep evidence above).
- Deviations: none. The Link-ON compile took 115 s. build-lane stays untracked (KEEP), which is not new.

INBOX-RECHECK: none
