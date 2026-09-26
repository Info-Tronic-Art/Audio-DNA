# Reviewer Verdict — manual-bpm
STATUS: DONE
VERDICT: APPROVE

REVIEW VERDICT: APPROVE

FILE: src/analysis/BPMTracker.cpp (pinned wt a9fbfcf)
  [OK] Spec/claim fidelity: runPipeline's manual branch (lines ~80-85 of the pinned diff) now calls
       `updatePhase(false, 0.0f)` instead of `updatePhase(beat, conf)`. `updatePhase()`'s hard-reset
       guard is `if (beat && conf >= kBeatResetConfidence)` (BPMTracker.cpp:215) — passing `beat=false`
       makes that branch unreachable, matching the claim "a detected beat no longer moves the phase".
       Read the AUTO branches (rawBpm<=0, silence, low-confidence, locked) byte-for-byte on both sides
       of the diff — all still pass the real `beat`/`conf` through; only the `if (manualMode_.load(...))`
       block changed. AUTO-mode behaviour claim VERIFIED.
  [OK] Error handling / correctness: `predictedBeatRegime_ = true` is still set before the call, so
       `advancePredictedBeat()` still fires on the free-running phase wrap — barCount_/beatInBar_/
       downbeatDetected_ bookkeeping (Pitfall 32 level semantics) is untouched, confirmed by reading
       `advancePredictedBeat()` and `updatePhrase()`'s rising-edge logic (unchanged in this diff).
  [OK] Sacred Rule 3 (analysis-thread allocation): the change is a literal swap (`beat`→`false`,
       `conf`→`0.0f`) inside an already-existing call; no new allocation, mutex, or syscall.
  [OK] Realignment paths still live: `requestResync()`→`applyResync()` (phase_=0, barCount_=0,
       downbeatDetected_=true) and `setManualBPM()` (phase_=0) are untouched by this diff and remain
       the only ways to move the manual phase, matching the claim.

FILE: src/analysis/BPMTracker.h (pinned wt a9fbfcf)
  [OK] Comment accuracy: both updated comments ("AUTO mode only", "detected beats never reset it")
       match the code exactly as verified above — no overclaim.

FILE: tests/test_bpm_stabilization.cpp (pinned wt a9fbfcf)
  [OK] Non-vacuous fixture: `ForeignBeatFeeder` really drives aubio-shaped input at a DIFFERENT tempo
       (142 BPM) than the manual BPM (120), and test 1 asserts `foreignBeats >= 40` before checking
       jumps/bars — so a feeder that silently fed nothing would fail on its own non-vacuity check
       first. `phaseJumped()` compares the true per-hop step against the expected free-run increment,
       so it Would have failed under the pre-fix `updatePhase(beat, conf)` (each foreign beat at
       conf=1.0 ≥ kBeatResetConfidence=0.5 hard-resets phase_ to 0, producing a jump every ~14 hops
       out of the ~40 expected free-run increments) — logically confirmed RED pre-fix by re-deriving
       the old code path, matching the builder's reported RED run (47/47 jumps, 0 bars).
  [OK] Test 2 (Resync) and test 3 (Tap / AUTO-restore regression) both isolate a single mechanism per
       assertion and reproduce the claimed realignment/free-run boundary.
  [OK] Staged-test hygiene (dimension 9 / N/A here — no `.new` candidate involved; plain ctest).

FILE: .harmony/probe-manual-bpm.sh (pinned wt a9fbfcf)
  [OK] Not vacuous: row M explicitly checks `onsetCount` rose by at least 60% of the expected click
       count before trusting the jump/bar assertions ("M non-vacuous: the analysis heard the click"),
       satisfying the packet's explicit anti-vacuity requirement. Row A (AUTO sanity) is run first,
       before manual mode is ever entered, working around the documented fact that no REST/OSC route
       leaves manual mode — a real gap, but honestly disclosed rather than papered over with a fake
       assertion.
  [OK] Bar duration measured from real `totalBarCount` edges (`bar_edges`), not from a hard-coded
       expectation — matches "measures bar duration from real edges" in the review check-list.
  [OK] Screen-safety and teardown sections match the pattern of sibling probes (probe-resync.sh,
       probe-downbeat-level.sh) already in the tree; own take folder is deleted at teardown.

FILE: .harmony/.reports/s-rta-0926/manual-bpm.md (pinned wt a9fbfcf)
  [OK] Builder report's RED/GREEN claims (12 PASS/6 FAIL pre-fix, 18 PASS/0 FAIL post-fix x2; ctest
       RED with "47 == 0" jumps pre-fix, 31 cases/168 assertions green post-fix; serial ctest
       576/576) are consistent with what the diff and test file predict; report also candidly flags
       two out-of-fence risks (Link tick would repeatedly zero the phase if AUDIODNA_BUILD_LINK were
       ever turned on; a pre-existing unsynchronized-write race on phase_/lockedBPM_ between message
       and analysis threads for Tap/set_bpm) — correctly scoped as pre-existing/out-of-fence, not
       fixed here, and not swept under the rug.

SLIM CHECK: no new production abstraction, no dead code introduced. The one added field-of-use is a
test helper (`ForeignBeatFeeder`, `manualPhaseInc`, `phaseJumped`) local to the test file — content
inside a test file, not a slimming target regardless.

SUMMARY: 5 files reviewed (BPMTracker.cpp, BPMTracker.h, test_bpm_stabilization.cpp,
probe-manual-bpm.sh, manual-bpm.md report), 0 blocking issues, 0 suggestions. The fix is a minimal,
correctly-scoped one-branch change; AUTO-mode paths are verified byte-identical; the two
realignment paths (Resync, Tap/set_bpm) are verified still live; the new ctest is logically RED on
the pre-fix code (re-derived, not just trusted); the probe is non-vacuous (checks onsetCount rose)
and measures bars from real totalBarCount edges, not synthetic timing. VERIFIED via direct reading
of the pinned worktree at a9fbfcf (git show), not from memory.

METADATA: reviewer=reviewer-manual-bpm, builder_packet=s-rta-0926, date=2026-09-26
