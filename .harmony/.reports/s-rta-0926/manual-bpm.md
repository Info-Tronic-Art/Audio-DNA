# s-rta-0926 lane manual-bpm: Builder report

STATUS: DONE
RESULT: Manual BPM mode no longer lets a detected beat reset the beat phase. The phase runs freely from the manual BPM. Resync, Tap and set_bpm still realign it, and AUTO mode is unchanged. The fix is one line in `BPMTracker::runPipeline`'s manual branch: `updatePhase(beat, conf)` became `updatePhase(false, 0.0f)`, the same call the held-silence branch already makes.
FACTS:
- RED on main's build (2bf1d56, pre-fix), `.harmony/probe-manual-bpm.sh`: 12 PASS / 6 FAIL. Over 20 s of manual 120 BPM with a 142 BPM click there were 39 phase jumps. The phase ran at 2.371 beats/s, which is the click's 142/60 and not 120/60. **0 bars.** After Resync: 19 jumps and 0 bars. (verified, `scratchpad/manual-bpm/red1.txt`)
- GREEN on build-lane: run 1 was 18 PASS / 0 FAIL and run 2 was 18 PASS / 0 FAIL. Both had 0 jumps and ran at 2.000 beats/s. Bars were 1.973 to 2.017 s in run 1 and 1.985 to 2.012 s in run 2. The first bar after Resync landed at 2.022 s both times, and bars after it were 1.982 to 2.015 s. (verified)
- ctest RED before the fix (build-lane, test target only): the new free-run case failed with `47 == 0` jumps: "foreign beats fed 47, phase jumps 47 (first at hop 39), bars 0". The new Resync case failed at `REQUIRE(found)` because the bars stall. After the fix test_bpm_stabilization passed 31 cases / 168 assertions. (verified)
- Serial full ctest on build-lane (`ctest -j1`): 576/576 passed (main is 573, plus the 3 new cases). (verified)
- `.harmony/probe-resync.sh` on the build-lane app: 16 PASS / 0 FAIL. `.harmony/probe-downbeat-level.sh` on the build-lane app: 14 PASS / 0 FAIL (duty 0.253, true-run median 504 ms, 15 edges == 15 bars). Both already have app-path overrides (`RESYNC_BUILD_DIR` and `DOWNBEAT_BUILD_DIR`, default `build-lane`), so neither was edited. (verified)
METHOD: I read BPMTracker in full (manual branch, updatePhase, advancePredictedBeat, feedDownbeatFeatures, applyResync, setManualBPM, silence) and MainComponent::applyTempoCommand (tap/manual/auto/resync/link). I then wrote the ctest and probe, ran both RED, applied the fix, rebuilt, ran serial ctest, and ran the probe GREEN twice plus the two sibling probes. Every live run held `/tmp/audiodna-live.lock`.
CONFIDENCE: high that the fix hits the root cause: the RED phase rate matched the click tempo exactly, and every GREEN run matched the manual tempo. VERIFY: Harmony's gate should re-run `bash .harmony/probe-manual-bpm.sh` on a merged build (set `MANUALBPM_APP` or `MANUALBPM_BUILD_DIR`) plus serial ctest.
UNKNOWNS / NOT DONE:
- The "leave manual mode, AUTO still locks" live row could not be driven after leaving manual mode, because no REST or OSC route leaves manual mode. `set_bpm` and OSC bpm only enter it, and "auto" exists only as the TopBar toggle, which the probe must not click. Instead, the probe runs its AUTO sanity row first, in a fresh launch that has never been in manual mode (it locks at 143.86 on the 142 click after about 11 s, on both builds). The manual-to-AUTO transition is pinned by a ctest.
NUANCE:
- The change is confined to `if (manualMode_)`. The AUTO paths are byte-identical, including the locked `rawBpm <= 0` branch, which still passes `beat` through (that is AUTO mode, so it is out of scope).
- `beatDetected_` still stores aubio's raw flag in manual mode. It is diagnostic only: nothing reads `beatDetected()` (grep of src and tests), and `scoreBeat()` was already gated off by `predictedBeatRegime_`.
- barCount, totalBarCount and downbeatDetected keep their semantics. They advance only through the predicted wrap (advancePredictedBeat) and the rising edge in updatePhrase. This is live-verified by probe-downbeat-level and pinned by a ctest (bars at 187 to 188 hops).
HANDOFF-NEEDS: none. There are two pre-existing issues outside the fence (see Risks).

INBOX-RECHECK: none

## Files changed
- `src/analysis/BPMTracker.cpp`: manual branch calls `updatePhase(false, 0.0f)`, with a comment explaining why.
- `src/analysis/BPMTracker.h`: two comments now say that the beat-reset rule applies in AUTO mode only.
- `tests/test_bpm_stabilization.cpp`: 3 new cases tagged `[s-rta-0926]`, plus a small `ForeignBeatFeeder` helper that feeds a 142 BPM beat stream at full confidence:
  1. "Manual mode: detected beats at another tempo never move the beat phase". Over 1900 hops (about 48 foreign beats) it asserts 0 phase jumps, bpm 120, and exactly 10 bars each 187 to 188 hops long. This case was RED before the fix.
  2. "Manual mode with detected beats at another tempo: Resync realigns once, then free-runs". This case was RED before the fix.
  3. "Tap still realigns in manual mode; leaving manual mode restores the AUTO beat reset". This is a guard that passes both before and after the fix.
- `.harmony/probe-manual-bpm.sh` (new, force-added because `.harmony/*` is gitignored). It generates its own click with `gen-click-wav.py --interval 20282` (142 BPM) and feeds it through file mode: `perf/record` with `audio:false` and `audioFile`, then an immediate `perf/stop` (the transport keeps playing). It deletes its own take folder at teardown.
  - Rows: A (AUTO sanity), then M (manual: bpm, onsetCount, jumps, rate, bar durations), then R (Resync: new-downbeat shape, a single origin change, jumps, first bar about 2.0 s, bar durations).
  - Overrides: `MANUALBPM_APP`, `MANUALBPM_BUILD_DIR`, `MANUALBPM_PY`, `MANUALBPM_POLL_S`, `MANUALBPM_CLICK_BPM`.
  - Screen safety: launches with `open -g`, never opens the Output window, quits with osascript, and checks the Quartz window list for Output windows.
- `.harmony/.reports/s-rta-0926/manual-bpm.md`: this report.

## Risks / open concerns (outside the fence, not changed)
1. **Link ticks will pin the phase near 0 (inferred, dormant).** `MainComponent.cpp:3537-3542` calls `applyTempoCommand("link", bpm)` on every UI timer tick while Link is enabled. Each call runs `setManualBPM`, which sets `phase_ = 0`. With `AUDIODNA_BUILD_LINK=ON` the phase would therefore be reset about 30 times a second. It is dormant only because Link is OFF by default. A fix belongs in MainComponent (only call on a tempo change) or in `setManualBPM` semantics. That is a product decision: should re-sending the same BPM realign the phase?
2. **Data race on `phase_` (inferred, pre-existing).** `setManualBPM` writes `phase_`, `lockedBPM_` and other fields from the message thread while the analysis thread reads and writes them, with no atomics. Resync was already moved onto the analysis thread through `requestResync`; Tap and set_bpm have not been.
3. The probe plays an audible 142 BPM click for about a minute on the default output device, the same as probe-step3.sh.

## Packet quality
- Clarity: CLEAR.
- Missing context: the packet didn't mention that /api/features has no trackerState field or that no REST/OSC route leaves manual mode. I found both by reading the source.
- Self-brief files: the notebook routine-grid section was useful.
