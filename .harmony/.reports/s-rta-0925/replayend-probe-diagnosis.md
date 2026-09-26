# replayend-probe-diagnosis -- the 5 FAIL rows are a PROBE bug, not a product bug

Builder, branch `lane/0925-replayend-probe` off `main` (6b1c9c2, lane/replayend already merged at
525a6fc). Confidence labels: VERIFIED = read from the cited file:line; INFERRED = derived from cited
code/log; ASSUMED = stated as such.

## VERDICT (first)

**The 5 FAIL rows are a probe bug, not a `RecorderHost` bug.** `RecorderHost`'s end-of-replay hold
(commits 2bb521c/957c4bb/1a9daea/d89756e/d0f2103, merged 525a6fc) is implemented exactly to
`plan-replayend.md` sections 1/3 and correctly holds at the take's REAL end -- for this run's
WithAudio replay that real end is the recorded audio asset's length, **61.44 s**, not the ~20 s the
recorded clip/opacity lane events span. `.harmony/probe-step3.sh`'s replay loop caps itself at a
**fixed 45 s fallback budget** computed from a field (`length`) that is read BEFORE `perf/play` --
at that point `playing_` is false, so the field is unpublished (NA) every time, and the budget
silently falls back to 45 s regardless of how long the take's audio actually is. The loop exits on
that budget before `finished` can ever become true, so every row downstream of "did the replay
finish" (finished, positionSeconds==lengthSeconds, clock stopped, inputSource back to input) reads
as if the hold were broken, when in fact the probe simply stopped watching 16+ seconds too early.
No `RecorderHost`/`MainComponent`/`ApiServer` code change. Fix applied: a probe-only wait phase,
described below and already committed on this branch.

## EVIDENCE

### 1. The numbers pin the root cause exactly

- Full log `/tmp/rta0926-step3.log` line 72: `FAIL end(withAudio): positionSeconds
  45.013395833333334 != lengthSeconds 61.44`. Line 30/32: `audio.segments[0].frames > 0 (2949120)`
  and `audio.wav frame count == take segment frames (2949120)`. `2949120 / 48000 = 61.44` --
  **exact match**, VERIFIED by arithmetic. `lengthSeconds` is not a coincidence or a stale value: it
  IS the recorded audio asset's real duration, in seconds, at the device rate this run recorded at
  (48000, line 29 `audio.segments[0].rate == 48000`).
- Line 73: `FAIL end(withAudio): the clock keeps counting (45.013395833333334 -> 46.0800625 over
  1 s)`. This is NOT the pre-existing D6 "never stops" bug (that bug is what lane/replayend was built
  to fix, and it fixed it -- see part 2 below); it is simply the correct, expected behaviour of a
  replay that has NOT yet reached its real end (45.0 s position < 61.44 s length) -- the clock is
  SUPPOSED to keep counting until 61.44 s. The probe read this INSIDE the still-playing window and
  reported it as a defect.
- Line 70/96: `FAIL end(withAudio): finished == False` / `FAIL end(wallClock): finished == False` --
  downstream of the same root cause: `finished` genuinely never latches within the probe's own
  observation window, because that window (45 s) ends before the take's real end (61.44 s / a
  wall-clock replay's `meta.duration`, the same order of magnitude -- see part 3).
- Line 75: `FAIL end(withAudio): inputSource == file at the end (expected input)` -- also downstream:
  `MainComponent::onReplayFinished()` (the handler that calls `restoreInputAfterReplay()`) only runs
  from `dispatch.replayFinished()`, which `RecorderHost::tick()` fires exactly once, on the tick where
  `finished_` first latches (`RecorderHost.cpp:522-544`, VERIFIED read). If the probe stops watching
  before that tick, the input was never going to be observed switching back -- not because the switch
  doesn't happen, but because the probe gave up first.

### 2. `RecorderHost`'s hold logic matches the merged spec exactly (read, not assumed)

Read in full on this branch (`lane/0925-replayend-probe`, based on `main` after the `lane/replayend`
merge):

- `RecorderHost.h:378-381` -- `playEndPos_`/`finished_` fields, comments match
  `plan-replayend.md` section 1 verbatim.
- `RecorderHost.cpp:708-713` (`play()`) -- `playEndPos_` computed ONCE per `play()`:
  `WithAudio: playFirstSample_ + max(max(0, program_->length - playFirstSample_), asset.frames)`;
  `else (Wall): max(program_->length, loadedTake_->meta.duration)`. Byte-for-byte the plan's section 3
  formula (`plan-replayend.md:176-180`). This is the F2/E2 "the end is the take's length, not its
  last event" behaviour Boris ruled on (`binding-decisions.md:486-494`) -- the real end for a
  WithAudio replay is `max(lane length, audio asset length)`, and for this run the audio asset
  (61.44 s) is the larger of the two (the lane's own clip/opacity events all land by t~16-20 s per
  the log's `replay(withAudio): t~16s activeClipColumn=2` line).
- `RecorderHost.cpp:503-545` (`tick()`) -- `pos` for WithAudio is derived from `transportFrames`
  (JUCE's `AudioTransportSource::getNextReadPosition()`, device-domain samples) converted to asset
  frames via the same 5.2 ratio formula the overdub path already used (`:508-517`); `finished_`
  latches the FIRST tick where `pos >= playEndPos_`, `player_->stop()` is called (nothing released --
  every gesture already closed at its own `x1`), and `dispatch.replayFinished()` fires exactly once,
  AFTER `publishStatus()` (`:537-544`) -- matching plan section 3's ordering rationale verbatim.
- `RecorderHost.cpp:820-826` (`publishStatus()`) -- while `finished_`, `position` is PINNED at
  `playEndPos_` rather than re-read from the (now-stopped) `Player`; `lengthSeconds` is derived FROM
  `playEndPos_` (one source of truth, matching plan section 3's equivalence note), not a second
  formula.
- `MainComponent.cpp:3391-3398` -- `transportFrames` is sourced from
  `audioEngine_.getTransportSource().getNextReadPosition()` every tick while
  `recorderHost_.needsTransportFrames()` is true, which is true for the entire WithAudio replay
  (`RecorderHost.cpp:117-120`: `overdub_ || (playing_ && playMode_ == WithAudio)`) -- the transport
  keeps being read all the way to the file's real end; nothing freezes it early.
- `MainComponent.cpp:5228-5251` (`perfPlay`) -- for a WithAudio replay, the ACTUAL 61.44 s WAV is
  loaded into `audioEngine_` and played via `applyAudioTransport("play", ...)`; the replay is a real
  61+ second transport play, not a synthetic/short one.

None of this needed a build to confirm -- it is all readable, deterministic C++ against numbers the
log already prints. Given the risk/effort tradeoff (a full CMake configure+build in a disposable
scratch dir, per the rig rules, for a question source-reading already answers with an exact
arithmetic match), a rebuild was not run; if Harmony wants an independent behavioral re-proof, the
new probe wait phase (below) is exactly that proof, and it runs on her own gate dir already.

### 3. Where the probe went wrong, cited to line numbers on `main` (pre-fix)

- `.harmony/probe-step3.sh:687` (pre-fix; now 687 unchanged by this branch):
  `TAKE_LEN="$(perf_field "d.get('length','NA')")"` -- this READS `/api/perf/status` at a point in the
  script AFTER `perf/load` but BEFORE the section's first `perf/play` call (the call is two lines
  later, at old `:691`/current `:723` after this branch's insert). `RecorderHost::publishStatus()`
  only populates `length` (and every other playback-only field) inside `if (playing_ && player_)`
  (`RecorderHost.cpp:820`) -- before ANY `play()` this run, `playing_` is false, so the JSON key is
  simply ABSENT, and `d.get('length','NA')` returns the literal string `'NA'`.
- `.harmony/probe-step3.sh:688`:
  `REPLAY_BUDGET="$(awk -v l="$TAKE_LEN" 'BEGIN{ if (l=="NA" || l+0<=0) print 45; else print l+2 }')"`
  -- with `TAKE_LEN == "NA"`, this ALWAYS evaluates to the fixed fallback, `45`. This is not new to
  this lane -- `plan-replayend.md`'s own ground-truth section (item under "0. Ground truth", the
  bullet on probe section 10) already named this exact defect BEFORE the fix landed: *"TAKE_LEN is
  read after perf/load and BEFORE perf/play (:636), but length is only published while playing ...
  it reads 0 -> REPLAY_BUDGET=45 and the position break (l+0>0) can never fire: both replay loops run
  the full 45 s today."* The plan's own prediction for the FIX was that this would stop mattering
  because "Breaking on `finished` ... SHORTENS the gate ... the fixed binary finishes each loop ~13 s
  SOONER than today's 45 s budget" (same ground-truth section, last bullet) -- i.e. the plan's author
  assumed the take's real length (F2's `lengthSeconds`) would land UNDER the 45 s fallback. For
  THIS run's fixture (a ~61 s recording -- the click-track/T2-alignment section that arms and records
  before section 10 runs for roughly a minute of wall-clock, per the log's own `framesWritten` growth
  from 92160 at arm+2s to a final 2,949,120-frame asset) that assumption does not hold: the real end
  is 61.44 s, well OVER the 45 s fallback, so the new `finished`-early-break the lane added
  (`.harmony/probe-step3.sh` old `:718`, now unchanged) never gets a chance to fire before the SAME
  loop's pre-existing `ELAPSED >= REPLAY_BUDGET` break fires first, at t~45 s.
- Net effect observed in the log: `replay(withAudio)` prints column changes up to `t~16s`, the loop
  then silently keeps polling until `t~45s` (no more column changes to print), exits on the budget,
  and the 10E block reads a snapshot mid-replay (`positionSeconds` 45.01, `finished` still false) and
  reports it as 4 failures for that mode. The wallClock loop repeats the same story one field lower
  (a wall-clock replay's real end is `meta.duration`, the same order of magnitude as the WithAudio
  asset's length since both derive from the same ~61 s recording session) -- `finished == False` there
  too (line 96), but its OTHER assertions (`playing` stays true, `inputSource` unchanged, sequence,
  last-look-held) still pass because they don't depend on the take actually having reached its end.

### 4. Why this reads as "probe", not "code", against the task's own decision framework

The task asked to determine, from the log + probe source + `RecorderHost`/status code, "is
`lengthSeconds` for a with-audio take the audio length or the lane length, and is that what Boris's
ruling implies." Per part 2 above: `playEndPos_` for WithAudio is `max(lane length, audio asset
length)`, and Boris's own ruling (`binding-decisions.md:486-494`, "It should keep playing ... with
the current parameters at the end. If there is no audio input, then it should play according to the
parameters") together with `plan-replayend.md`'s own E2 test contract ("a with-audio replay finishes
when the transport reaches the asset's end") settles this: the audio genuinely IS the clock, and the
replay legitimately runs until the audio's real end, however long that is. A probe that assumes a
fixed, short budget regardless of the actual recording's length is checking an assumption Boris's own
ruling rejects (a replay must NOT stop or truncate at some arbitrary short cutoff before the audio's
real end) -- fixing the CODE to finish "sooner" to fit the probe's budget would contradict the ruling
this exact lane was built to implement. The fix belongs in the probe.

## FIX APPLIED (probe-only, this branch)

`.harmony/probe-step3.sh`: added `wait_for_replay_finish()` (defined once, next to the existing
`TAKE_LEN`/`REPLAY_BUDGET` derivation) and one call site after EACH of the two shape loops (WithAudio,
wallClock), before their respective "10E" assertion blocks. The function:

1. Returns immediately if `finished` is already true (no-op on a build/fixture combination where the
   shape loop's own early-break already caught it).
2. Otherwise reads `lengthSeconds` FRESH from `/api/perf/status` (now available, since the app is
   playing) and computes a wait bound of `lengthSeconds + 5s margin - elapsed-so-far` (elapsed
   measured from the SAME `START_T` the shape loop above already uses for that mode) -- i.e. derived
   from the take's real, just-observed length, never a small fixed number.
3. Falls back to a fixed 10 s bound ONLY if `lengthSeconds` itself is unavailable/non-positive (a
   genuinely broken build where even the pre-existing F2 formula regressed) -- a safety net against
   an infinite hang, not the primary policy.
4. Polls `finished` every 0.25 s (same cadence as the shape loop) until true or the deadline passes.

**Fail-first preserved on a pre-fix binary -- INFERRED from source, NOT verified by the pre-fix RED
log (correction, fix round 1 -- `review-replayend-probe-r1.md` finding 2 is correct: the original
wording here overclaimed).** The reasoning: pre-fix code never sets `Status::finished` at all (the
field did not exist before commit `2bb521c`), so on a pre-fix binary the new wait phase's primary
branch (bounded by `lengthSeconds`, which F2 already publishes pre-fix -- `RecorderHost.cpp:799-821`)
waits out its bound and then reads `finished` as permanently `NA`/false, so the 10E assertions that
depend on it still correctly FAIL. That is a sound INFERENCE from reading the pre-fix source, not a
verification -- `/tmp/rta0926-end-red.log` does NOT establish it: that log was produced by the OLD
(unfixed) probe script, contains none of `wait_for_replay_finish`'s own echo lines (the function did
not exist yet when that log was captured), and reflects an even earlier product build state (9
FAILs, `finished == NA` throughout, pre-`lane/replayend`-merge). Citing it as verification of the
NEW function's behavior on a pre-fix binary was citing an artifact that never exercised that
function. No fail-first re-proof run (new probe vs. an actual pre-`lane/replayend` build) has been
made; this paragraph is corrected to an honest INFERRED label, not upgraded to VERIFIED.

**No RecorderHost/MainComponent/ApiServer/test change.** `ctest` count is unaffected by this branch
(probe-only); the 9 tests this lane's REAL fix added (`E1-E4`, `R13-R15`, `R12b`, per
`plan-replayend.md` section 8) already exist on `main` from the `lane/replayend` merge and are not
touched here.

## FIX ROUND 1 (`review-replayend-probe-r1.md`, REQUEST_CHANGES) -- disposition

Two findings, both correct:

1. **No live run of the committed probe fix exists postdating `ed6c669`** -- both cited logs
   (`/tmp/rta0926-step3.log` 11:21:47, `/tmp/rta0926-end-red.log` 11:16:24) predate the fix commit
   (11:30:27) and contain none of `wait_for_replay_finish`'s own diagnostic echo lines. Correct;
   the original "Fail-first preserved, verified by..." wording overclaimed.
2. **The "verified by re-reading the pre-fix RED log" claim misattributed verification.** Correct;
   fixed above -- that paragraph is now labeled INFERRED, not VERIFIED, and states plainly that the
   RED log does not exercise the new function.

**What this fix round adds:** `.harmony/test-wait-for-replay-finish.sh`, a headless unit test that
extracts `wait_for_replay_finish`/`perf_field`/`perf_status`/`is_true` VERBATIM from
`.harmony/probe-step3.sh` by line range (never retyped, so it tracks the live source) and exercises
them against a synthetic local HTTP responder standing in for `/api/perf/status` -- a real child
process (`python3 http.server`) and a real `curl` round-trip over `127.0.0.1`, not a shell-function
stub. Run 2026-09-26 (log: `/tmp/rta0926-wait-unit-test.log`), postdating `ed6c669`: 6/6 PASS --
the diagnostic echo (`wait_for_replay_finish(unit-test): lengthSeconds=3 elapsed=0s -- waiting up to
8s more`) fires with correct numbers (the exact line class the reviewer found absent from both
pre-fix logs), the function returns in 1s once the synthetic endpoint flips `finished` to true
(well before its own 8s bound -- proves the poll loop, not just the timeout), the no-op path fires
when `finished` is already true at entry, and the fixed-10s-fallback branch fires when
`lengthSeconds` is unavailable.

**What this does NOT close:** this proves `wait_for_replay_finish`'s OWN mechanism against synthetic
data, not `RecorderHost`'s real end-of-replay behavior against a live rebuilt app. Producing that
run requires launching `Audio-DNA.app`; this fix round's rig rules forbid launching the app, and per
this project's own convention (every `probe-*.sh` header in this directory: "the party that builds
never verifies") that live gate run is Harmony's to execute against `build-gate/`, not the
Builder's. Finding 1's core ask -- a log postdating `ed6c669` showing the 5 previously-FAIL rows now
PASS on the real app -- remains outstanding and is not claimed as done here.

## WHAT HARMONY SHOULD DO

1. **Still required, per fix round 1's disposition above -- not yet done.** Rebuild/relaunch is NOT
   required for this fix -- it is a shell-script-only change to `.harmony/probe-step3.sh`. Run
   `.harmony/probe-step3.sh` again against the SAME already-built binary (main HEAD 6b1c9c2, or this
   branch merged in) and confirm a log postdating `ed6c669` shows: the previously-FAIL rows now
   PASS (`end(withAudio): finished == true`, `positionSeconds == lengthSeconds`, `the clock
   stopped`, `inputSource back to input`, `end(wallClock): finished == true`), and
   `wait_for_replay_finish`'s own echo lines present in the log. Expect the run to take roughly
   16-45 s longer per mode (the new wait phase actually watches the replay through to its real end)
   -- not a flake, the intended behavior change. The headless unit test added in fix round 1
   (`.harmony/test-wait-for-replay-finish.sh`) proves the function's own mechanism against synthetic
   data; it is not a substitute for this live run against the real app.
2. If Harmony wants the fail-first protocol re-proven end-to-end (not just inferred from source
   above), run this updated probe once against a pre-`lane/replayend` build
   (`STEP3_BUILD_DIR` pointed at a build of `main` before commit `525a6fc`) and confirm the same 5
   rows are still RED there (expected, per the reasoning above), then against the current
   binary/HEAD and confirm all rows are GREEN.
3. No code review of `RecorderHost.cpp`/`.h`, `MainComponent.cpp`, or `ApiServer.cpp` is needed for
   THIS lane's change (none were touched) -- `lane/replayend`'s own review already covers that code
   (`.harmony/.reports/s-rta-0925/review-replayend-r1.md`).

STATUS: DIAGNOSIS + FIX ROUND 1 COMPLETE -- probe fix written and syntax-checked (`bash -n`); its own
polling/echo mechanism proven via a headless unit test against a synthetic endpoint
(`.harmony/test-wait-for-replay-finish.sh`, 6/6 PASS, log `/tmp/rta0926-wait-unit-test.log`,
2026-09-26, postdating `ed6c669`); no C++/build changes made or required. UNVERIFIED and
outstanding: a live run of `.harmony/probe-step3.sh` against the real rebuilt app showing the 5
previously-FAIL rows now PASS -- blocked in this fix round by rig rules forbidding launching the
app, and per this project's convention that live gate run is Harmony's to execute, not the
Builder's.
