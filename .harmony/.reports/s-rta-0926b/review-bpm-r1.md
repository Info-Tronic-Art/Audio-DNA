# Reviewer Verdict — lane bpm, round 1 (s-rta-0926b)
STATUS: DONE
VERDICT: PASS_WITH_NITS

PINNED ARTIFACT: worktree .claude/worktrees/wf_41d6317f-a47-2, branch lane/bpm-thread-0926b,
base bc69fd0d4da0b6ca78ecb3c0e959215fc4f3ca7f, head 990ca27 (3 commits: b0de34c, 8c300d4, 990ca27).
Reviewed via `git show`/`git diff` at that head only.

## Files reviewed
- src/analysis/BPMTracker.h (+28/-1)
- src/analysis/BPMTracker.cpp (+42/-4)
- src/MainComponent.h (+5/-1)
- src/MainComponent.cpp (+13/-4)
- tests/test_bpm_stabilization.cpp (+182/-3)
- .harmony/.reports/s-rta-0926b/bpm.md (builder report, +98)

## Findings

[OK] Correctness (the race + the fix): `runPipeline()` (BPMTracker.cpp:69-76) takes the pending
tempo request FIRST, before `updatePhase`/the manual-mode branch, then `applyTempoRequest`
zeroes `phase_` only `if (realign || tempoChanged)`. This exactly matches the report's "applied
at the START of the next hop" claim. Independently re-derived (not recalled): I extracted the
`tempoRequest_`/`postTempoRequest` scheme (kTempoPending=bit63, kTempoRealign=bit32, low 32 bits
= float bits) into a standalone TSan-compiled repro in $TMPDIR and ran it with 8 threads x 100k
CAS posts concurrently with an exchange-consumer: 0 TSan reports, and the coalescing behaved
exactly as documented — a realign requested by either of two racing posts survives regardless of
post order (verified both orders), the last-posted BPM always wins, no bit overlap between the
float payload and the two flag bits. This independently corroborates the builder's TSan claim
(6 warnings on base, 0 on fix) at the level of the actual bit-packing algorithm, even though I did
not rebuild the full JUCE/Catch2 target myself (a ~25 min cold build; treated as a proposed-not-
executed mutation per the reviewer HARD-GATE — the builder's own RED/GREEN transcripts in
bpm.md are the primary evidence for the full-suite claim, and the scratch logs they cite were
correctly deleted as scratch per rig rules, so they're gone, as expected).

[OK] Enumeration completeness (packet item 2): grepped `setManualBPM|followExternalTempo|
setManualMode|requestResync|getBpmTracker` across all of src/ — every hit outside BPMTracker.*
itself is inside `MainComponent::applyTempoCommand` (MainComponent.cpp:5169). `getBpmTracker()`
has exactly one caller. This confirms the report's "11 writers, all routed through one
function" table is not overclaiming — there is no other path into the tracker.

[OK] Readers-of-published-copies claim: grepped `isManualMode` — only two test files call it;
TopBar/MainComponent/ApiServer all read `displaySnap_`/`snap.*`/`featureBus_.read()`, never
tracker internals. Matches the report.

[OK] Test fidelity (packet item "no mirror copies of production logic"): the new TSan test's
analysis-thread loop calls `feedSilenceDetection` -> `process` -> `feedDownbeatFeatures` in that
exact order; diffed against src/analysis/AnalysisThread.cpp:188-196's real stage-5 sequence and
it matches token-for-token. `manualPhaseInc`/`phaseJumped` are pre-existing helpers (line 857/880),
reused rather than duplicated. The tests call the real `BPMTracker`/`applyTempoCommand` surface,
not a parallel mock.

[OK] Fence respected: `git diff --stat` shows only BPMTracker.{h,cpp}, MainComponent.{h,cpp},
tests/test_bpm_stabilization.cpp, and the report file — no tests/CMakeLists.txt change (correct,
same target), nothing under src/render or src/recording.

[OK] Hygiene: 3 clean commits, nothing stray (no build dirs, no FBO trace code, no .venv symlink)
committed. The worktree's untracked `build-lane/` (git status "?? build-lane/") is pre-existing —
`.gitignore` doesn't cover it at base either (verified `git show <base>:.gitignore` has no
`build-lane` entry, and this lane's diff doesn't touch `.gitignore`) — so this is a pre-existing
gap, not one this lane introduced, and it is correctly NOT part of the reviewed diff.

[OK] Spec/claim fidelity on the two disclosed-but-not-fixed items (packet dimension 7 — I opened
the actual source rather than trusting the prose): (a) `LinkSync.h` confirms `enabled_`, `bpm_`,
`isEnabled()`, `getBPM()` are plain unconditional atomics, NOT gated by `#if AUDIODNA_HAS_LINK` —
so the Link toggle really does force fake-120-BPM manual mode in a default (non-Link) build, as
the report's "Found, not fixed #1" claims. (b) `docs/claude/performance-controls.md:43` really
does say "every LinkSync method compiles to a no-op" — verified false against (a). Both are
pre-existing, outside this lane's fence (docs/, TopBar.cpp, LinkSync.cpp untouched), and honestly
disclosed rather than silently left for someone to discover later.

[NIT] `BPMTracker::applyTempoRequest`'s `tempoChanged = (folded != lockedBPM_)` is an exact
float `!=`. Elsewhere in the codebase (MainComponent.cpp's Link-capture throttle, R3/critic B1)
"the same tempo" is deliberately defined with a 0.01 BPM epsilon, not bit-exact equality. If a
real (built-ON) Ableton Link session ever reports the nominally-same tempo as a double that
isn't bit-identical to the previous tick's double after the `float` narrowing (e.g. two different
internal Link tempo estimates that happen to average to visually-the-same BPM), `followExternal
Tempo` would treat it as "changed" and realign anyway, partially reintroducing the exact symptom
this lane fixes (fewer resets than every tick, but not zero). This is speculative — Ableton
Link's negotiated tempo is a discrete value that should be stable between deliberate changes —
and the builder already disclosed the Link-ON path was compile-checked only, not live-exercised
(no route to enable the real Link path without a synthetic click, which is forbidden). Suggest:
when a future lane builds `-DAUDIODNA_BUILD_LINK=ON` and drives it live, watch for spurious
realigns; if seen, switch `tempoChanged` to the same 0.01 epsilon used for the capture throttle.
Non-blocking: it doesn't affect the default (Link-OFF) build, doesn't affect Tap/set_bpm (which
always realign unconditionally, unaffected by this comparison), and is explicitly named as an
open fork by the builder ("LINK-RAMP").

[NIT] `docs/claude/performance-controls.md:43`'s no-op claim is stale (confirmed above) but
outside this lane's fence to fix. Flagging for Harmony to route to a docs-touching lane —
not a reason to fail this review.

## Semantics-preservation checks (packet item 3 — MUST NOT change)
- Manual BPM never lets a detected beat move the phase: unaffected — this lane touches only the
  `tempoRequest_` path consumed at the top of `runPipeline`; the manual-mode branch that ignores
  `beat` (BPMTracker.cpp:94-98, `updatePhase(false, 0.0f)`) is untouched.
- Resync/Tap/set_bpm realign exactly as today: verified — `setManualBPM` always posts
  `realign=true` (BPMTracker.cpp:563-566), unconditionally, for every one of its 8 call sites
  (Tap x2, manual field, REST/OSC set_bpm, take/routine replay via the generic action dispatch).
  Only the one Link-timer call site was changed to `followExternalTempo`.
- Same-value set_bpm keeps today's (realign) behaviour, undecided product question left alone:
  confirmed — `setManualBPM` has no `tempoChanged` gate at all; only `followExternalTempo` does.
- Latency (~10.7ms, one hop): confirmed by re-reading `runPipeline`'s new top-of-function request
  consumption — happens once per hop, before phase advance, identical timing to the direct write
  it replaces. The report's own live measurement (0.064-0.085s vs 0.064-0.107s baseline, both
  well past one hop) is consistent and not contradicted by anything in the diff.

## Verdict rationale
No MUST-severity findings. The fix is minimal, matches an existing precedent in the same file
(`resyncRequests_`/`requestResync`) rather than inventing a new concurrency primitive, is fully
enumerated and fenced, and its core claim (a real race, fixed with 0 residual TSan reports) is
independently corroborated at the algorithm level by a from-scratch repro I compiled and ran
under ThreadSanitizer myself. The two NITs are non-blocking: one is a disclosed, outside-fence
pre-existing doc/defect pair; the other is a plausible-but-speculative edge case in a dormant,
compile-checked-only code path, already flagged by the builder as an open fork.
