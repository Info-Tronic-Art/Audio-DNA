# Reviewer Verdict — s4fix
STATUS: DONE
VERDICT: APPROVE

## Scope check (locked plan items F1-F8, all MUST/SHOULD-now)
All 8 items VERIFIED present in commit 45667cb on branch lane/s4fix, in the exact files/lanes the plan
assigned (H=RecorderHost.{h,cpp}+its test, M=RecordPanelModel.h+its test, V=RecordPanel.cpp, B=MainComponent.cpp):

- F1 (clock restarts per take): `clock_ = RecorderClock{}` added in `arm()` before `recorder_.start()`, and
  `clock_.tick()` moved to be the first statement inside `if (recording_)` in `tick()`
  (RecorderHost.cpp:229-234, :392-401 diff). Root cause fix, not a symptom patch — matches the plan's
  diagnosis (RecorderClock latches on the first-ever `tick()` call; host previously owned one clock for its
  whole life). New `[host][clock]` test case + 2 sections match the plan's text verbatim.
- F2 (Playing length): `publishStatus()` now takes `max(event-derived length, asset frames/rate)` for
  WithAudio and `max(length, loadedTake_->meta.duration)` for WallClock (RecorderHost.cpp diff). Header
  comment updated correctly. Two new fail-first sections added.
- F3 (plain-word notices + "Saved" notice): all 7 MainComponent.cpp funnel sites and the 3 RecorderHost.cpp
  refusal strings match the plan's replacement table exactly, including the two branch conditions
  (`result.error == "not recording"`, `result.error == "no take loaded"` — both verified to match the actual
  strings the host returns). `grep -n '"perf/' src/MainComponent.cpp` returns 0 hits (re-verified
  independently).
- F4 (name field whole width): Row C is now the name alone (capped 260px), Row C2 the two toggles below —
  matches plan geometry exactly.
- F5 (Record Over hint before press): `else if (withAudio && !recording)` notice branch added, correct
  precedence (a fresh refusal notice still wins). Test checks both the hint and the override.
- F6 (Stop Recording tooltip during overdub+playing): ternary matches plan text exactly, test pinned.
- F7 (Armed + playing mirrors the plain-recording branch): implemented and tested.
- F8 (AA contrast): `juce::Colours::black` on `kMeterRed`, matches plan's computed 5.46:1.

## Independent verification (read-only, build + ctest self-run in build-lane)
- Rebuilt `build-lane` clean (`cmake --build build-lane --config Release -j6`) — exits 0, all targets
  including `test_recorder_host` and `test_record_panel_model` rebuilt.
- Ran `ctest` in `build-lane`: **100% tests passed, 438/438**, 7.12s — matches the builder's claim.
- Fail-first reasoning independently re-derived (not re-executed under mutation — this MINIMAL-boot sandbox
  fences all writes outside the named REPORT_FILE, including `$TMPDIR` copies, so a live revert-and-run
  proof was not possible this session): for F1, `RecorderClock::tick()` latches `haveTicked_` on the FIRST
  ever call; the new test drives 101 pre-arm ticks starting at wallNow=0 before arming, so without the
  `arm()` reset the very first tick already fixed `startWall_=0`, and `host.status().t` after the post-arm
  tick at wallNow=100 would read 100.0, not 0.0 — confirms the claimed RED. For F2, before the fix
  `lengthSeconds = max(0, length-first)` with `length==0` (no lanes) trivially returns 0.0 for both new
  sections — confirms the claimed RED. This is INFERRED from source, not a live-toggled re-run; the build+
  ctest pass itself is VERIFIED live.

## Root cause vs symptom
F1 and F2 fix the data at the source (the clock instance lifecycle; the length computation in
`publishStatus()`), not the presentation layer — consistent with the plan's own rejection of a
model-side patch (the model is a pure function of `Status` and cannot know arm time or audio length).

## Thread/UI rules
- `clock_ = RecorderClock{}` runs inside `arm()`, gated by `RECORDER_HOST_ASSERT_MESSAGE_THREAD()` at the
  top of the function — correct thread. `RecorderClock` re-creation is by value assignment in place, and
  `PerformanceRecorder` holds the clock by reference taken at `recorder_.start()` (called immediately after
  the reset, same statement block) — address is stable, no dangling reference.
- No RT/audio-callback code touched. `RecordPanel::resized()` layout change is UI-thread only, no new
  heap/mutex use.
- No new `juce::Slider` added (F8 is a colour token change only) — ResettableSlider convention N/A here.

## Deviations disclosed by builder — assessed
1. **Probe pin (`.harmony/probe-step3.sh`) not added.** Correctly deferred — the plan explicitly marks this
   file as Harmony's, not the builder's. Confirmed `git diff` shows the probe script untouched. This is a
   real gap (the wall-clock timing regression is only caught by the new ctest section until the probe pin
   lands) but it is scoped correctly and flagged, not silently skipped — non-blocking for this lane.
2. **`perfStop()` finalize-error concatenation beyond plan text.** The plan's replacement table specifies a
   bare `Saved: <name>` notify; the builder appends `, but its audio had a problem: <error>` when
   `result.error` is non-empty, to avoid the "Saved" notice silently overwriting a finalize-error notice the
   host just posted. This is a small, sensible superset of the plan (not contradicting it), disclosed
   up-front, and does not touch any pinned test string. Acceptable — not scope creep in the harmful sense.
3/4. Old on-disk takes and the visual re-shoot are correctly called out as out of this lane's scope
   (read-only-on-source instruction; no app launch performed, matches the packet's read-only constraint).

## Verdict
All 8 planned fixes landed correctly, at the root cause, in the right files/lanes, with fail-first tests
that plausibly RED before (independently reasoned) and are GREEN now (independently re-run, 438/438). No
blocking issues found.

FILES: src/recording/RecorderHost.{h,cpp}, src/MainComponent.cpp, src/ui/RecordPanel.cpp,
src/ui/RecordPanelModel.h, tests/test_recorder_host.cpp, tests/test_record_panel_model.cpp
ISSUES: none blocking. Non-blocking notes: probe-step3.sh pin still outstanding (Harmony's file, not this
lane's); perfStop's combined "Saved + finalize-error" notice is a small disclosed superset of the plan text.
METADATA: reviewer=reviewer-agent, builder_packet=s4fix, date=2026-09-25
