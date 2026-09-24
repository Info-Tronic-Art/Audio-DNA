# Critic — plan-step4-record-panel.md

Read-only pass against source at `main`. Method: re-verified every high-risk cite (RecorderHost.h/.cpp,
MainComponent.cpp perf lambdas + applyAudioTransport, ApiServer.h, RecordPanel.h/.cpp, BrowserPanel.h,
tests/CMakeLists.txt, Player.h/.cpp, Program.cpp, RecorderClock.cpp) by reading the file at the cited
line, not by trusting the plan's quotes.

## Findings

### MAJOR — "Stop Playback" during overdub leaves the audio transport running; row 11 is misdescribed as a clean state

Evidence: `RecorderHost::stopPlay()` (`RecorderHost.cpp:650-661`) never touches `overdub_`; `disarm()` is
the only place that clears it (`RecorderHost.cpp:342`). `MainComponent::applyAudioTransport("stop", ...)`
(`MainComponent.cpp:5148-5158`) refuses unconditionally when `recorderHost_.status().overdub` is true,
notifying `"audio stop refused: overdub in progress (R-A8)"` and returning WITHOUT calling
`audioEngine_.stop()`. The plan's `perfStopPlay()` (S4-B item 5, §4.5) calls
`recorderHost_.stopPlay(); applyAudioTransport("stop", Origin::Human);` unchanged (only the new
source-mode-restore guard is added after it). So the exact REST sequence the plan's own matrix names as
row 11 ("Overdub recording, playback already stopped (REST-only)") — arrived at by calling
`/api/perf/stop_play` while overdub recording is active — produces `RecorderHost::Status.playing == false`
while `audioEngine_`'s transport keeps running (it is what is still driving the overdub's clock via
`transportFrames`, per `tick()`'s `sampleForClock` branch at `RecorderHost.cpp:387-391`). The panel's
status line for row 11 ("Recording over <folderName> …") never says the audio engine is still playing.
This is not a new bug introduced by step 4, but step 4's own click-through matrix (§4.3) presents row 11
as a legitimate, well-understood state and the Interaction-Logic critic brief (§6 item 1) only asks about
"REST-only states the panel misreports" as a *display* concern — it does not name the transport-still-
running hazard, and no probe row in §4.6/§6 exercises it (the closest walk-through in §6's bash script
never calls `stop_play` while overdub is armed). **Amendment**: add row 11 to the probe/live-check list
explicitly (`POST /api/perf/record {overdubAssetId}` → `POST /api/perf/stop_play` while still recording →
assert `GET /api/perf/status` shows `playing:false` AND separately confirm via `GET /api/status` or an
audio-engine transport readout that the engine did NOT actually stop) so the discrepancy is at minimum
documented as accepted behavior, not silently shipped as if row 11 were consistent.

### MAJOR — a third, undisclosed behaviour change rides S4-B, undercounting the plan's own risk section

The plan's APPROACH section (line 48) and §9 item 6 both say "Two deliberate behaviour changes ride
S4-B" (selector sync; source-mode restore on Stop Playback) and the risk list (§8) has exactly R-1/R-2
covering those two. But §4.5 item (6) adds a third: `perfStop()` on success now also calls
`dispatch.notify("Saved: <takeFolder>")` — a new notify that did not exist in the current
`apiServer_->onPerfStop` lambda (`MainComponent.cpp:2063-2067`, which today notifies ONLY on failure).
This is a real behavior change to the shared REST path (every `/api/perf/stop` call now emits a stderr
notify line it didn't before, per `RecorderHost::dispatch.notify` → `std::cerr`), reachable by REST
independent of the panel, yet it has no named risk (R-1..R-8 don't cover it) and no regression check
(`probe-step3.sh` was written before this change existed, so it cannot know to assert its ABSENCE of a
false-positive "Saved" notify on failure, or its presence on success). **Amendment**: either fold this into
R-1 explicitly (probe-step3.sh must not choke on an extra stderr line) and add one probe assertion for the
new notify text, or state plainly in §9 that "three" behaviour changes ride S4-B, not two.

### MINOR — S4-M's proposed JUCE module links are unverified against the actual transitive include set

Traced the full include chain from `src/recording/RecorderHost.h` (`RecorderHost.h:1-19`) through
`PerformanceRecorder.h`, `RecorderClock.h`, `AudioStore.h`, `Player.h`, `Program.h` → `Take.h`,
`model/ControlPath.h`, `recording/Lane.h`, `recording/PerfState.h`, `recording/TempoMap.h`,
`connect/AutomationCurve.h`: every one of them includes only `<juce_core/juce_core.h>` plus STL headers —
none needs `juce_events` or `juce_graphics`. The plan's §4.3 test-target spec says link
"`juce::juce_core juce::juce_events juce::juce_graphics` (whatever RecorderHost.h's transitive includes
need — builder trims to the minimum that configures...)" — the parenthetical already hedges this away, so
it's not wrong, just imprecise: verification here shows the minimum is `juce_core` alone (matching the
`test_thumbnail_cache`/`test_recorder_host`-style targets already in the file, none of which link
`juce_events`). Not a blocker since the plan explicitly defers to the builder to trim, but worth tightening
so the builder isn't tempted to leave the unneeded links in (which is exactly the kind of thing that could
quietly reintroduce a GUI/display dependency later if `juce_graphics` is misread as license to add
`juce_gui_basics` "since it's adjacent").

## Non-findings (specifically re-verified, hold up)

- G6/G7 (`RecorderHost.h:180-207`, `.cpp:576-596,681-745`): `Status` genuinely has none of the six fields
  S4-A proposes to add, and `load()` genuinely never calls `publishStatus()` today — confirmed by reading,
  not by trusting the grep claim.
- G8/G9 (`RecorderHost.cpp:473-481`, `Program.cpp:162-164,273`, `RecorderClock.cpp:16-27`): the
  Sample-domain position math and the `t`/`sample` semantics are exactly as cited; the plan's
  `positionSeconds` formula (subtract `playFirstSample_`, divide by `playAssetRate_`) is consistent with
  what `player_->position()` actually returns (`Player::position()` returns the raw `pos` passed to the
  last `advanceTo`, and `tick()` computes that `pos` as `playFirstSample_ + transportFrames` for
  WithAudio — `RecorderHost.cpp:476-479`, `Player.h:90`).
- G14/G15/G16/G17 (`MainComponent.cpp:2012-2154`): all six `onPerf*` lambda bodies, the two
  `setSourceMode(File)` call sites (`:2029`, `:2088`), and the "source mode never restored" claim are
  verbatim-accurate; `applyAudioTransport`'s overdub-refusal text (`:5153-5158`) matches G16's cite exactly.
- G19 (`ApiServer.h:95-107`): all six perf callbacks are `void`-returning except `onPerfStatus`
  (`juce::var()`) — confirmed, which means the plan's `perf*` member functions returning `std::string` are
  correctly wired as fire-and-forget from the REST lambdas (`perfRecord(o);` discards the return, matching
  the unchanged `void` ApiServer signature) — this is NOT a defect, the plan's own example code at
  §4.5 line 382 already does this correctly.
- G4/G5 (`RecordPanel.h/.cpp`, `BrowserPanel.h:31-36,47`): the "what exists today" inventory (five
  disabled buttons, empty `refresh()`, zero external callers of `getRecordPanel()`, tab title exactly
  `"Record"`) is accurate line-for-line.
- G22 (`tests/CMakeLists.txt`): confirmed zero `juce_gui_basics`/`juce_gui_extra` links anywhere in the
  file (1126 lines), and `test_analysis_resampler` genuinely is the last target (ends at line 1126), so
  "append at EOF" is accurate.
- R-7 (AX title collision): `ax_press.py` matches by exact `AXTitle` string (`ax_press.py` docstring +
  `press_button` usage), so renaming the panel buttons to "Record Take"/"Play Take" against the tab's
  exact `"Record"` title genuinely removes the ambiguity — not a false claim.

## Not attacked (would need more budget)

S4-C's widget layout math (row A/B/C/D pixel widths) and the full 12-row click-through table's individual
cell correctness (button enabled/disabled per state) were read but not exhaustively cross-checked against
every `Status` predicate combination — that is squarely the Interaction-Logic critic's job the plan already
assigns as MANDATORY in §6, and duplicating it here would not add signal within this pass's scope.

## Verdict

**APPROVE-WITH-AMENDMENTS.** The architecture (host-additive status, GUI-free model, one funnel) is sound
and every load-bearing citation checked out against the actual source — this is an unusually well-grounded
plan. The two MAJOR findings are both about a state the plan itself invented (the REST-only overdub/replay
combos) not being fully honest about what it does — exactly the class of defect the plan's own
Interaction-Logic-critic gate exists to catch, so folding both into that gate's brief (not a rewrite of the
plan) closes them.
