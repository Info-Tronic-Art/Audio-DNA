STATUS: DONE

### POST-DONE CORRECTION (commit a5c9782)
The independent reviewer traced the team-lead's "Mod 2" live-app measurement
to `SignalRegistry.cpp:70` and found it is `EnvelopeSignal("Mod 2", 4.0f)`
(default triangle control points), not `OscillatorSignal(SawUp, 2.0f)` as
I had encoded from the team-lead's label. The measured values
(0.0/0.125/0.25/0.375) were real — they only sampled the envelope's rising
linear segment, where `t = cyclePhase/0.5` numerically coincides with a
duration-2 SawUp's `cyclePhase` in that region. I verified BOTH Mod 1 and
Mod 2 directly against `SignalRegistry.cpp:65-70` (Mod 1 confirmed correct
as-is: `OscillatorSignal("Mod 1", Sine, 1.0f)`), fixed the test to construct
the real `EnvelopeSignal`, renamed/corrected the comments, and added the
assertion that distinguishes the two hypotheses (cyclePhase=0.5 → the real
envelope reads 1.0 at its peak control point; a duration-2 SawUp would read
0.0 at the same snapshot — this single point is what the original test
lacked). Rebuilt RC=0, full ctest 241/241 unchanged; the corrected test case
itself grew from 197 to 235 total assertions (8 cases) with the new
peak-distinguishing check. Committed separately, on its own.

### RESULT
Fixed the multi-bar phase bug in OscillatorSignal/EnvelopeSignal (an oscillator/envelope with beatDuration_ > 4 could never complete a cycle) by folding `4*barCount` into the phase calculation. Made `beatInBar`/`barCount` injectable (clamped) via `/api/inject_features`. Added 8 unit tests (`tests/test_oscillator_bar_fold.cpp`) covering full-cycle completion, unchanged short-duration behavior, and the phrase-reset trade-off. Build RC=0, ctest 241/241 (233 baseline + 8 new). Load-bearing proof done: mutation broke 5/8 test cases exactly as predicted; restore verified byte-identical by md5.

### FACTS (disk-cited)
- Bug confirmed at `src/signal/OscillatorSignal.h` `getValue()`: pre-fix, `totalBeatPhase = beatPhase + beatInBar` is bounded to `[0,4)` (one bar); `EnvelopeSignal.h` `getValue()` had the identical construction.
- `FeatureSnapshot::barCount` (`src/analysis/FeatureSnapshot.h:49`) is `uint16_t`, "bars since last phrase reset".
- `BPMTracker::updatePhrase`/`resetPhrase` (`src/analysis/BPMTracker.cpp:408-457`): `barCount_` increments every bar (`++barCount_` at line 425) and is reset to 0 ONLY on (a) BPM unlock, or (b) a structural transition entering drop or leaving breakdown (lines 430-438) or via explicit `resetPhrase()` (lines 452-457) — never on a fixed period. `phraseBars_` (default 8, configurable 1-32) is used only to compute `phrasePhase` via `barCount_ % phraseBars_` (line 442); the raw `barCount_` itself is not periodic.
- The reset paths touch only `barCount_`/`phrasePhase_`/`prevDownbeatDetected_` — `beatInBar_`/`phase_` (beatPhase) are owned by separate functions and are NOT reset alongside it.
- UI exposure (`src/ui/SignalInspector.cpp`): oscillator's `beatDurationSelector_` offers up to 8 beats (lines 41-47); envelope's `envBeatDurationSelector_` offers up to **16 beats** (lines 87-92) — a worse pre-fix case (capped to 25% of cycle, never reaching its peak control point) than the oscillator's 8-beat case (capped to 50%).
- `src/api/ApiServer.cpp` `handleInjectFeatures` (~line 612): pre-fix, accepted `beatPhase`/`barPhase`/`phrasePhase` but not `beatInBar`/`barCount`.

### METHOD
1. Read work packet + `.harmony/gotchas.md` in full before any edit.
2. Read `OscillatorSignal.h`, `EnvelopeSignal.h`, `FeatureSnapshot.h`, `Signal.h` to confirm the defect and available data.
3. Traced `BPMTracker.cpp` (`src/analysis/`, read-only — outside fence) to verify the packet's own claim about how/when `barCount` resets, rather than trusting its framing.
4. Checked `SignalInspector.cpp` for the real UI-exposed duration ranges (found envelope goes to 16, not just 8).
5. Implemented the fix in both signal headers, added injection fields to `ApiServer.cpp` (fence-only files).
6. Wrote `tests/test_oscillator_bar_fold.cpp` (header-only, no JUCE dependency, matching `test_smoother.cpp`/`test_routing_engine.cpp` house style) + `tests/CMakeLists.txt` entry.
7. Waited for build-slot clearance (team-lead confirmed free, gave live-app regression anchors) — added a dedicated test reproducing those exact measured values before building.
8. Built (`cmake --build build --config Release -j 10`, redirected to file, `$?` read immediately — no pipe/tee), ran `ctest`.
9. Load-bearing proof: md5'd both headers, in-place neutralized (removed the `barCount` term only, kept it as a scoped, clearly-labeled temporary block), rebuilt, ran the test binary directly, confirmed 5/8 cases failed with the mathematically-predicted wrong values, restored the two lines via Edit back to the exact original text, re-verified md5 match, rebuilt, re-ran full ctest.

### CONFIDENCE + VERIFY
High confidence in the fix's correctness (verified algebraically and empirically): for beatDuration_ in {0.25,0.5,1,2,4}, `4*barCount_/beatDuration_` is always an integer, so `fmod` strips it exactly — bit-for-bit unaffected, confirmed by ctest (`OscillatorSignal short beat durations unchanged across bars`, `EnvelopeSignal default duration (4.0) unchanged`, and the team-lead's live-app anchor test all green). Cross-bar completion verified both by direct value assertions (peak/trough reachability) and by reproducing the team-lead's own live `/api/signals` measurements exactly. Independent verification still needed: (1) Reviewer read of the two header diffs and the ApiServer.cpp diff; (2) team-lead's own `/api/signals` sweep against the running app with the new `beatInBar`/`barCount` injection fields, which is the actual designed oracle for this lane (I did not launch the app — out of scope per packet: "I gate it").

### UNKNOWNS / NOT DONE
- App not launched/gated by me (explicitly the team-lead's job per packet: "Do NOT launch the app. I gate it").
- Did not add `kBeatsPerBar`-style constant sharing between `src/analysis/BPMTracker.h` and the signal headers — used the literal `4.0f`, matching the existing (pre-fix) code's own implicit 4/4 hardcode in the same two files; considered importing `kBeatsPerBar` but rejected it as an unnecessary new `signal/` → `analysis/` header coupling for a private constant, and analysis/ is explicitly off-fence.
- Nothing was found or changed outside the fence.

### NUANCE
**The judgment call, named as instructed:** `barCount` is NOT a fixed-period counter (this refines the work packet's own framing, which implied a periodic 8-bar reset). It grows monotonically and resets to 0 only on rare structural-transition events (drop hit / breakdown exit). Given that, I used the raw `barCount` (not `phrasePhase`, which the packet already flagged as unreconstructable — `FeatureSnapshot` doesn't publish the phrase length in bars, so there's no way to convert `phrasePhase` back to beat units for an arbitrary `beatDuration_`, and it would ALSO introduce a truly periodic jump every phrase where the raw counter does not). Net trade-off, pinned by the "Phrase-reset trade-off" test: the oscillator/envelope phase **never jumps merely because N bars elapsed** (proved: crossing an 8-bar boundary with no reset event produces a continuous, non-decreasing phase); it **does jump** at an actual structural-transition reset, because `beatInBar`/`beatPhase` continue from wherever the beat clock is while `barCount` snaps to 0 (proved: a simulated reset produces a >0.3 discontinuity). This is strictly better than the pre-fix behavior (never completing a cycle at all, 100% of the time) traded for a rare, real jump only at drop/breakdown transitions — which is also musically the moment a visible transition is already happening, so a visual jump there is far less jarring than a perpetual half-cycle stutter.

Two things I verified rather than assumed, per this repo's own gotchas: (1) `EnvelopeSignal` genuinely needed the identical fix, and worse — its UI selector goes to 16 beats, not just 8, so it was even more broken pre-fix (capped at 25% of its cycle, never reaching its peak control point at position 0.5). Added a dedicated test for this. (2) The team-lead's live-app sweep (Mod 1 sine dur=1, Mod 2 ramp dur=2, both at `beatInBar=0`/`barCount=0`) is now a pinned regression-anchor test with the exact measured values, not just my own derived math.

### HANDOFF-NEEDS
None — build slot returned clean, no blockers. Independent reviewer + team-lead's live `/api/signals` sweep against the running app are the remaining verification steps (by design, not mine to run).

---

## PHASE EXPRESSION AND WHY
`totalBeatPhase = beatPhase + beatInBar + 4.0f * barCount` (was missing the last term). `4.0f` = beats/bar (4/4 time, matching the existing `beatInBar` range 0-3 and the pre-existing "totalBeatPhase is in [0,4) over one bar" comment already in this code). `cyclePhase = fmod(totalBeatPhase / beatDuration_, 1.0f)` unchanged. Applied identically in `OscillatorSignal.h` and `EnvelopeSignal.h`.

## PHRASE-RESET TRADE-OFF AND CHOICE
See NUANCE above. Chose: raw monotonic `barCount` (accept rare jumps only at real structural-transition phrase resets). Rejected: deriving from `phrasePhase` (data doesn't exist to convert back to beat units for arbitrary `beatDuration_`, and it resets on a truly fixed period which is worse, not better). Pinned with a dedicated test asserting both halves.

## INJECTION FIELDS ADDED
`src/api/ApiServer.cpp` `handleInjectFeatures`: `beatInBar` (clamped 0-3, `uint8_t`) and `barCount` (clamped 0-65535, `uint16_t`), placed immediately after the existing `phrasePhase` line, same style/precedent as the `structuralState`/`detectedGenre` R6 clamp comment.

## BUILD / TEST NUMBERS
- Build (pre-mutation, i.e. the actual fix): exit code **0**. `cmake --build build --config Release -j 10`, output redirected to a file, `$?` read immediately (no pipe/tee, per this repo's documented trap).
- ctest (fix in place): **241/241** passed (233 baseline + 8 new test cases from `test_oscillator_bar_fold.cpp`). Exact accounting: 233 + 8 = 241, matches.
- Mutation build (barCount term removed): exit code **0** (compiles fine — this is a value bug, not a type error).
- Mutation test run (`./tests/test_oscillator_bar_fold` direct): **3 passed, 5 failed** (192/197 assertions passed, 5 failed) — failures exactly at the cases that require cross-bar behavior: "8-beat cycle completes across two bars", "Sine reaches both extremes", "EnvelopeSignal completes ... 8 beats", "EnvelopeSignal 16-beat option", "Phrase-reset trade-off". The 3 that passed under the bug are the ones NOT meant to discriminate this bug (short-duration-unchanged, live-app-anchor at barCount=0, envelope-default-unchanged) — expected and correct.
- Restore: `md5` before mutation and after restore identical for both files:
  - `OscillatorSignal.h`: `af6aaa8b7801863252ea339270316aa9` (both readings)
  - `EnvelopeSignal.h`: `398e0fb3141df6fbcf3d6a5b746130a4` (both readings)
- Post-restore rebuild: exit code **0**. Post-restore ctest: **241/241** passed again.

## FILES CHANGED
- `src/signal/OscillatorSignal.h` — folded `4*barCount` into `totalBeatPhase`, documented rationale/trade-off inline.
- `src/signal/EnvelopeSignal.h` — identical fix, comment references OscillatorSignal.h for the full rationale.
- `src/api/ApiServer.cpp` — `handleInjectFeatures`: added clamped `beatInBar`/`barCount` injection.
- `tests/CMakeLists.txt` — new `test_oscillator_bar_fold` executable entry (header-only, no JUCE link needed).
- `tests/test_oscillator_bar_fold.cpp` — new, 8 TEST_CASEs (see TESTS below).

## TESTS (all in tests/test_oscillator_bar_fold.cpp)
1. "OscillatorSignal 8-beat cycle completes across two bars" — start/mid/near-end values + full monotonic 32-point sweep across 2 bars.
2. "OscillatorSignal Sine 8-beat cycle reaches both extremes" — peak (cyclePhase 0.25) AND trough (cyclePhase 0.75, provably unreachable pre-fix).
3. "OscillatorSignal short beat durations unchanged across bars" — 5 durations x 5 shapes x 7 barCounts, all within 0.0005 of the barCount=0 baseline.
4. "Live-app regression anchors: Mod 1 / Mod 2" — reproduces the team-lead's exact live `/api/signals` measurements.
5. "EnvelopeSignal completes a full cycle over 8 beats".
6. "EnvelopeSignal 16-beat option completes across 4 bars" — the worse, UI-real case discovered while verifying.
7. "EnvelopeSignal default duration (4.0) unchanged across bars".
8. "Phrase-reset trade-off is pinned" — no jump crossing a fixed 8-bar boundary absent a reset event; a real >0.3 jump on a simulated structural-transition reset.

## ISSUES
None outside the fence. Two informational corrections to the work packet's own framing, both stated above and pinned by tests: (1) `barCount` does not reset on a fixed period, only on structural-transition events — refines but doesn't invalidate the packet's judgment-call framing. (2) `EnvelopeSignal`'s UI exposes up to 16 beats, not just the oscillator's 8 — makes the envelope fix more consequential than the packet's framing suggested, not less.

## SKILL_PROPOSALS
None — this was a straightforward bug-fix + test lane, well covered by existing skills/conventions.

## RISKS
None identified beyond what's already named in NUANCE (the accepted rare phase-jump at structural-transition phrase resets, which is the explicitly sanctioned trade-off).

## PACKET QUALITY
- Clarity: CLEAR. The packet named the defect precisely (with file/anchor-text pointers, not line numbers), specified the fence tightly, and explicitly asked for the judgment call to be surfaced rather than decided silently.
- Missing context: the packet characterized `barCount` as resetting "on a fixed 8-bar phrase" — verification against `BPMTracker.cpp` showed the reset is actually structural-transition-triggered, not periodic. This is a correction, not a gap, and didn't change the chosen fix (raw barCount was still correct) but did change the accuracy of the trade-off's likely-frequency (rarer than "every phrase implied).
- Unused context: none — every section of the packet was directly load-bearing for this task.
- Self-brief files: `.harmony/gotchas.md` — existed, useful (confirmed the build-exit-code-through-a-pipe trap and the app-launch/single-instance rules, though I never launched the app this lane).

## STATUS
DONE

## NEXT ACTION
Team-lead: sweep `/api/signals` against the running app with the new `beatInBar`/`barCount` injection fields (the designed oracle for this lane) and dispatch the independent source reviewer. No action needed from me.

INBOX-RECHECK: 2 addenda folded (team-lead's "slot is free" message with live-app regression anchors, and the repeat/recap message) — both incorporated (regression-anchor test added before building; build/test/proof steps completed in the order recapped).
