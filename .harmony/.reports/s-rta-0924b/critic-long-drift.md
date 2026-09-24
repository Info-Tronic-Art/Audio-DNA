# Critic: plan-long-drift.md (STEP3_LONG=1 opt-in 10-minute T2 drift proof)

Hostile review against actual source, s-rta-0924b. Every file:line the plan
cites was independently re-read; discrepancies and gaps are below. No claim
in this report is taken from the plan's own text without re-derivation.

## Verified accurate (spot-checked, not assumed)

The plan's factual scaffolding is unusually solid. Independently re-read and
confirmed byte/line-exact:
- `.harmony/probe-step3.sh` is 717 lines (`wc -l` matches F0's header claim).
- Every named insertion anchor (E1 after :94/:95 `set -u`, E2 after :107
  `CLICK_INTERVAL=`, E3 after :165, E4 after :170, E5's replace range
  :349-355 and the python body :351-452, E6 insert between :635 `fi` and
  :637 `# --- 12.`) matches the actual file content at those exact lines.
- P1's insertion point (:354 `interval = int(interval_str)`), P2's (:444
  `pct_matched = ...`, confirmed at 12-space indent, matching the plan's
  snippet indent), and P3's target line (:450, the print's closing
  fragment) are all exact.
- F3/F4/F5 (`AudioEngine::loadFile` calls `stop()` plus position reset,
  `AudioEngine.cpp:32-52`; `onTransportStateChanged` has exactly one
  external setter, `AudioEngine.h:32`, and one internal call site,
  `AudioEngine.cpp:130-131`, zero consumers outside `AudioEngine`; grep
  confirms zero `setLooping`/`isLooping` hits anywhere in `src/audio`) are
  correct -- a too-short WAV really does go silent, not stop the take.
- F6-F9 (onset markers via `onsetCount` delta, `kMaxOnsetMarkersPerTick=8`
  at `RecorderHost.cpp:22`, the marker loop at :428-449, `marker()` at
  exactly :559; the tap self-stop edge detector at :395-415 sets
  `lastError_` only, never `recording_`) all confirmed against current
  source.
- F7 (`kMappingTickHz = 120`, `MainComponent.h:306`, driving
  `recorderHost_.tick(...)` every tick) and F8 (`kCheckpointSeconds = 60.0`,
  `RecorderHost.h:214`) confirmed.
- F11 (`AudioTap.h:132` `kMinFreeBytes = 2 GB`, checked at
  `AudioTap.cpp:109`) confirmed at the exact cited lines.
- F1/F2 (spec D10.3 text, `s167-performance-log-and-routines.md:606-612`
  and the ppm-drift warning at :566-567) confirmed verbatim.
- The OLS stderr arithmetic in section 2 (window-difference stderr about
  0.98 ms independent of take length; slope stderr 1.70/0.76/0.54 ms at
  2/10/20 min; `minutes_for_0p5ms` about 22.8 on the run-8 take) all
  re-derive correctly from sigma=7.60 ms and the stated marker rate.
- The regex non-collision claim in E5's closing paragraph (new keys
  `drift_win_ms=`/`drift_win_stderr_ms=` are not substrings of the existing
  `drift_ms=`/`drift_stderr_ms=` patterns, with or without the leading-space
  anchor E6 actually uses) holds under direct string inspection.
- No C++ source is touched by this plan -- only the bash test harness. There
  is therefore no RT-hot-path, allocation, lock, or thread-safety question
  to raise; the three endpoints reused (`/api/perf/record`, `/status`,
  `/stop`) are the same ones sections 5/7/11 already exercise today.

## MINOR -- early-break claim overstates what the poll loop actually does

Severity: MINOR. Evidence: E6's poll loop (plan lines 268-283) versus F9
(`RecorderHost.cpp:395-415`).

R7 states: "a genuine `recording=false` breaks out immediately with a FAIL
so a dead recorder never costs the full ten minutes." But F9 -- which the
plan itself derives and cites -- establishes that the single most likely
failure mode on an unattended long run (a tap self-stop, e.g. a device
rate/channel change) does not set `recording=false`; it only sets
`lastError_` and leaves `recording_` true (`RecorderHost.cpp:403-415`, the
comment block the plan quotes almost verbatim in F9). The E6 poll loop's
`case` statement only inspects the `recording` field (plan lines 275-279);
it prints `lastError` every 30 s but never inspects it to break early. So
for exactly the failure this long-running, unattended block is most exposed
to, the script will not "never cost the full ten minutes" -- it will
faithfully burn the full `LONG_MINUTES+10s` wall time before the post-stop
`LERR` check (plan line 289-290) finally flags it as FAIL. That is a
correct FAIL, just not an early one, and it contradicts R7's own framing.

Amendment: in the poll loop's python one-liner (plan line 272-273), also
emit `lastError` as a field and add a third `case` arm: a non-empty
`lastError` (not just `recording=false`) should set `LONG_EARLY_STOP=1` and
break, with its own `no` message distinct from the "stopped early" one
(since `recording` may still read `true`). This actually delivers what R7
already claims, on the failure mode F9 says is the one to expect.

## MINOR -- `s_err` scope guard is correct but fragile

Severity: MINOR. Evidence: `probe-step3.sh:429-436` (existing code, not
touched by this plan) versus P2's inserted guard (plan lines 204-206).

`s_err` (used in P2's `minutes_for_0p5ms` computation) is a Python local
assigned only inside the nested `if n_pts greater-than 2: if dof
greater-than 0 and sxx greater-than 0:` branch (:432-434). P2 guards its use
with `if len(matched) greater-than 2 and drift_stderr_ms greater-than 0.0`.
Traced by hand: `len(matched) == n_pts` exactly (both derive from the same
`matched` list, :414 versus :427), and `drift_stderr_ms` is initialized to
`0.0` right before the `n_pts` check and reassigned only inside the
`dof`/`sxx` branch, so the guard does correctly imply `s_err` was assigned
in the overwhelming case (a residual sum of exactly zero being the only
theoretical counterexample, and in that case the guard is merely
over-conservative, not wrong). The plan's own footnote ("builder: keep that
guard") shows this was noticed. This checks out today, but it is an
implicit cross-branch invariant a future edit to the untouched original
code (:429-436) could silently break without touching P2 at all, producing
a `NameError` deep in a 16-minute run's very last step.

Amendment: have the builder add one defensive line at :428 (before `if
n_pts greater-than 2:`): `s_err = None`, and change P2's guard to also
check `s_err is not None` explicitly, rather than relying solely on the
`drift_stderr_ms` proxy. Trivial, and removes a latent trap for the next
person who touches the untouched code around it.

## MINOR -- scope note: sigma projection is labeled, but R5's own mechanism is untested by S1-S4

Section 2's entire threshold sizing (the stderr-scaled bounds, both WARN
thresholds, the ppm framing) is built on sigma=7.60 ms measured from a
65-second take (F16), projected onto a 600-second take by assuming constant
per-marker jitter. The plan properly labels this INFERRED and R5 already
names the specific mechanism most likely to violate that assumption
(periodic 60 s `Take::save` on the message thread, :454-468: "a slow save
could delay one tick and stamp one marker late -- a jitter contribution near
minute boundaries"). That's honest. But none of the cheap self-tests (S1-S4)
actually exercises ten checkpoint boundaries -- S3 is the unmodified default
run (about 65 s, at most one checkpoint boundary) and S4
(`STEP3_LONG_MINUTES=2`) crosses only two. So the one risk the plan itself
flags as most likely to inflate p95 near minute boundaries is not actually
probed until S5, the real 16-minute run -- a reasonable design (a cheaper
proxy for periodic-save jitter isn't obviously available), but the "cheap
self-test" section should say explicitly that R5 is not validated by S1-S4
and is only checked by the real run, so a reader doesn't mistake "S4 passed"
for "R5's risk is retired."

Amendment: add one sentence to section 6 (S4's description) noting S4 only
exercises two of the roughly ten checkpoint boundaries a real run will
cross, so R5 remains unverified until S5.

## Scope / simplicity check

No scope creep found. The plan explicitly rejects three larger
alternatives (replacing the gate's own take, looping the WAV, running the
long take during replay) with concrete, source-grounded reasons (F4's
no-looping-API fact, R5 overdub-exclusivity in section 11, the checkpoint-
sleep rewrite cost) -- all three rejections hold up against the source I
re-read. The additive, opt-in, single-function-hoist design is close to the
minimal change that could deliver the D10.3 measurement without touching
the default run's behavior or renumbering existing sections (a real
concern given how many later sections of this script index into it by
line-anchored sed/sleep offsets).

## Nothing found in: test validity, discrimination power, thread safety

- The tests can fail: a real drift beyond the stated stderr-scaled bound, a
  jitter regression (p95 exceeding 15 ms), or a coverage collapse (matched
  percentage under 90) all produce a `no` row, not a vacuous `ok`. The
  stderr-scaled relaxation is deliberate and defended with real numbers
  (R1), not a silent weakening.
- No RT/hot-path violation is possible because no C++ is touched.
- No missed consumer: independently grepped for `onTransportStateChanged`,
  `setLooping`/`isLooping`, `kMappingTickHz`, `kCheckpointSeconds`,
  `kMinFreeBytes`, and `marker(`/`onsetCountBaseline_` and found nothing the
  plan omitted or mischaracterized.

## VERDICT: APPROVE-WITH-AMENDMENTS

Three MINOR amendments (early-break on `lastError` during the long poll,
defensive `s_err = None`, and an explicit "R5 unverified until S5" note).
No BLOCKER or MAJOR finding -- the plan's disk citations, arithmetic, and
line-anchored edit instructions all independently re-verified against
current source.

REPORT_FILE: /Users/boriskarpman/projects/RealTimeAudio/.harmony/.reports/s-rta-0924b/critic-long-drift.md
STATUS: DONE
