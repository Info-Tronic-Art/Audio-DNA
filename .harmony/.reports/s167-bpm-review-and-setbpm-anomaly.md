# BPM Review + set_bpm Anomaly — s167
STATUS: DONE
VERDICT: JOB2=(a)+(b) gate-is-wrong, no product defect found; COMMIT=SHIP-WITH-FIXES

## JOB 2 — set_bpm anomaly verdict (settled, not a product defect)

**Root cause: `--test-mode` never starts the AnalysisThread, so nothing ever
publishes into the FeatureBus `/api/bpm` reads.** This is answer (a)+(b)
combined; (c) real product defect is RULED OUT by source; (d) n/a.

Chain, with file:line:
- `Main.cpp:14-26` — `--test-mode` sets `testMode=true`, passed into
  `MainComponent`.
- `MainComponent.cpp:1758-1767` — `if (!testMode_) { setFeatureBusWriter(...);
  analysisThread_.startThread(...); }`. In test mode this whole block is
  **skipped** — the analysis thread (which owns `bpmTracker_` and calls
  `process()`/`feedDownbeatFeatures()` every hop) is never started.
- `MainComponent.cpp:1769-1784` — in test mode, `TestServer` claims the
  FeatureBus writer instead (`R4`, single-writer-by-construction: production
  → AnalysisThread, test → TestServer). `FeatureBus.h:8-12` documents this
  explicitly: "ONE writer — the analysis thread in production, the TestServer
  in test mode."
- `TestServer.cpp:43-53` — `injectSnapshot()` is the **only** place TestServer
  publishes, and it only runs when `/api/inject_features` is called. The
  gate's experiment never called that endpoint.
- `ApiServer.cpp:556-577` (`handleSetBpm`) → `MainComponent.cpp:1810-1817`
  (`onSetBpm`) → `tracker->setManualMode(true); tracker->setManualBPM(bpm);`.
  This **is wired correctly** and does mutate the real `BPMTracker` instance
  (`lockedBPM_=120`, `manualMode_=true`, `phase_=0`, `trackerState_=LOCKED`)
  — confirmed by reading `BPMTracker::setManualBPM`/`setManualMode`
  (`BPMTracker.cpp:510-533`). `/api/set_bpm` is **not** the archived
  "no-op stub" described in `docs/archive/feature_audit/slice_10_*.md` /
  `docs/archive/FEATURE_INVENTORY.md` — those are stale; `git log` shows the
  real hookup landed in `45ae7e8` ("...set_bpm... was a no-op stub").
- `ApiServer.cpp:542-554` (`handleGetBpm`) reads `featureBus_.read()` — a
  `FeatureSnapshot` that is **never written** in this run, so it stays at its
  zero-initialized default (`bpm=0, beatInBar=0, barCount=0`) for the entire
  8-second poll regardless of what happened inside `BPMTracker`.
- `[AudioEngine] Switched to mic input mode` (`AudioEngine.cpp:112`) is a
  device-routing log, orthogonal to whether the analysis thread's hop loop
  is running — it does not imply BPMTracker ever got fed audio.

So: the tracker object itself almost certainly did what it was told (locked
to 120, and — per the P24 fix under review in JOB 1 — would have advanced
`beatInBar_`/`barCount_` on its own). None of that reached the HTTP surface
because the publishing thread was never started. **This says nothing about
whether tap tempo / the P24 fix works** — the gate exercised a code path that
structurally cannot reflect BPMTracker state in `--test-mode`.

**Correct headless path:** there is currently no way to observe live
BPMTracker state over HTTP while `--test-mode` is set, short of manually
constructing and POSTing a `FeatureSnapshot` via `/api/inject_features` (which
would bypass BPMTracker/P24 entirely and prove nothing about the fix).

**Cheapest discriminating experiment:** repeat the exact same sequence
(`GET /api/bpm` → `POST /api/set_bpm {"bpm":120}` → poll `GET /api/bpm` for
~8s) **without `--test-mode`** (production mode; a silent/quiet room is fine
— manual mode ignores audio content entirely once locked, and the analysis
thread runs its per-hop loop, including in silence, at `AnalysisThread.cpp:
160-163`). Expected if JOB 1's fix is correct: `bpm` reads ≈120 almost
immediately, and `beatInBar` cycles 0..3 with `barCount` advancing by
roughly 4 over the 8s window (120 BPM ÷ 4 beats/bar = 1 bar every 2s). If
`bpm` still reads 0/stale in that configuration, that would indicate a real
defect and should be re-escalated — but I found no code path suggesting that
will happen.

## JOB 1 — review of commit e437872 (BPMTracker P24 fix)

**COMMIT VERDICT: SHIP-WITH-FIXES.** The core anti-freeze mechanism is sound
and I could not find an actual double-counting or freeze defect via manual
trace of every path. The one HIGH item is a **test-coverage / claim-fidelity**
gap (commit message overclaims what was mutation-tested), not a functional bug
in the shipped code — but it should be closed in a fast-follow so a future
regression on that exact line doesn't ship silently.

### 1. Double counting — verified correct by construction (not by hop-timing luck)

`runPipeline()` (`BPMTracker.cpp:68-189`) has exactly 5 exit routes (4 early
returns + 1 fall-through), and **every one of them sets `predictedBeatRegime_`
exactly once immediately before its single call to `updatePhase()`**
(lines 79/80, 92/93, 105/106, 117/118, 187/188). Grep confirms `updatePhase()`
has no other call site, `runPipeline()` has exactly two callers
(`process()` line 60, `processRawBPM()` line 65 — both hit it once), and
`advancePredictedBeat()`/`scoreBeat()` each have exactly one call site
(`BPMTracker.cpp:221`, `:310`), both gated by the *same* `predictedBeatRegime_`
value set earlier in the *same* hop's `runPipeline()` call. There is no path
where the flag is read before being set this hop, and no path that invokes
either increment function twice in one hop. [OK] — genuinely by construction.

Production call order (`AnalysisThread.cpp:160-171`) is
`feedSilenceDetection(rms)` → `process(hopBuffer)` → `feedDownbeatFeatures(...)`,
exactly once each per hop — matches the test helpers' call order
(`tests/test_bpm_stabilization.cpp:37-53`, `:352-359`), so the tests exercise
the real per-hop sequence, not a bypass (T13 gotcha check: clean).

### 2. Regime boundaries — [OK], with one documented/bounded caveat

- silence→audio: `feedSilenceDetection` runs *before* `process()` each hop
  (`AnalysisThread.cpp:162-163`), so `inSilence_` has already flipped by the
  time `runPipeline()` reads it — no one-hop staleness.
- rawBpm<=0 branch (`BPMTracker.cpp:92`) correctly falls back to predicted
  regime only if already locked (`lockedBPM_>0`); if never locked,
  `updatePhase()` is a no-op anyway (`:193-197`). [OK]
- manual↔auto: `manualMode_` is a single atomic bool read once at the top of
  `runPipeline()` (`:77`) — toggling takes effect cleanly next hop, no torn
  state.
- **Caveat (bounded, likely intentional, not a double-count):** during the
  ~100ms silence-*exit* hysteresis window (`silenceExitHops_`,
  `BPMTracker.cpp:227-229`, `561-568`), `inSilence_` can still read `true`
  while aubio has already started flagging real onsets on the resumed audio.
  In that window `predictedBeatRegime_` is `true` (silence branch,
  `:103-108`), so the scoreBeat gate at `:308` suppresses the real onset —
  the beat is *not lost* (the predicted-wrap path still advances the
  counters), just not scored with real spectral data. This is a onsuppression, not a double-count, bounded to ≤100ms, and matches the
  `feedDownbeatFeatures` comment's own reasoning (`:302-307`). LOW severity,
  worth a one-line note in the fix's doc comment but not a defect.

### 3. `downbeatDetected_` shared-channel discipline — [OK], one nit

`advancePredictedBeat()` (`:225-242`) uses the identical formula scoreBeat()'s
locked branch uses (`:332-337`): `beatCounter_=(beatCounter_+1)%4;
beatInBar_=beatCounter_; downbeatDetected_=(beatCounter_==0)`. Since
`updatePhrase()`'s rising-edge detector (`:471-472`) runs every hop
unconditionally and compares against `prevDownbeatDetected_` (also updated
every hop), the two writers preserve the exact same latch/clear discipline —
no risk of a permanently-latched-high flag.

**[ISSUE] LOW/observation** — `advancePredictedBeat()` unconditionally treats
`beatCounter_==0` as a downbeat regardless of `downbeatLocked_`, whereas
`scoreBeat()` has two branches: the *locked* branch does this, but the
*unlocked* branch (`:345-356`) drives `beatInBar_` off `totalBeatsScored_` and
always sets `downbeatDetected_=false` (no downbeat is claimed until a real
position has been statistically established). The doc comment
(`BPMTracker.cpp:227-228`) scopes its claim correctly ("mirrors... scoreBeat()'s
**locked** branch"), so it isn't a misrepresentation, but it means: in cold-start
manual/tap-tempo (exactly the commit's own new test,
`downbeatLocked()==false` throughout), the predicted path still emits
`downbeatDetected_=true` every 4th beat as if locked. I checked for
consumers of `downbeatLocked()` outside `BPMTracker` itself — there are
**none** (`grep -rn "downbeatLocked\b" src/` outside BPMTracker.{h,cpp} is
empty) — so this has zero observable effect today. Flag it only because a
future consumer that gates bar-sync effects on `downbeatLocked()` would get
phantom downbeat/bar events from tap tempo. → Suggest a one-line doc-comment
addendum on `advancePredictedBeat()` noting this deliberately assumes 4/4
alignment from beat 0 when no real downbeat has ever been established.

### 4. P23 silence-hold regression — untouched, [OK]

The silence branch (`:103-108`) still calls `updatePhase(false, 0.0f)` with
the same hardcoded args as before the diff; the only change inside
`updatePhase()` is the new trailing `if (wrapped && predictedBeatRegime_)`
block (`:214-222`), which never mutates `phase_`/`lockedBPM_`. `bpm()` and
`beatPhase()` outputs during silence are unaffected — confirmed by re-reading
the diff context (`git show e437872` hunk 3) and the unchanged phase math
above the new block.

### 5. Tests — [ISSUE] HIGH: "anti-double-count" test doesn't test what it claims

Traced by hand (build was out of scope for this review per the task packet,
so this is a **deterministic arithmetic trace of the diff's own formulas**,
not an executed repro — flagging as such):

`feedRealOnsets()` (`tests/test_bpm_stabilization.cpp:30-53`) never calls
`feedSilenceDetection()` and never sets manual mode, and always feeds
`rawBpm=120>0`, `conf=1.0≥kConfidenceThreshold`. Tracing `runPipeline()`'s
branches against those inputs: **`predictedBeatRegime_` is `false` on every
single hop of the "Anti-double-count" test** (`:407-425`), because none of
the three conditions that set it `true` (manual mode / `inSilence_` /
`rawBpm<=0`) is ever reached. Consequence: the scoreBeat gate at
`feedDownbeatFeatures`, `BPMTracker.cpp:308`
(`if (beatDetected_ && lockedBPM_ > 0.0f && !predictedBeatRegime_)`) has
`!predictedBeatRegime_` **vacuously true** for the whole test — removing that
clause entirely would not change the test's result at all. **This is the
specific gate the commit's double-counting-hazard paragraph is about**
(aubio's -70dB gate vs. our RMS hysteresis disagreeing), and it is
**completely unexercised** by this test.

What the test *does* verify: with `hopsPerBeat = round(24000/512) = 47`
(exact rounding: true period is 46.875 hops) and the pre-existing hard phase
reset on every high-confidence onset (`beat && conf>=0.5 → phase_=0`,
`:208-212`, unchanged by this diff), the phase's free-running accumulation
from 0 reaches `46 × 0.021333 ≈ 0.9813` by the hop *before* the next onset,
then crosses 1.0 exactly on the onset hop itself (`0.9813+0.0213=1.0027`).
So `wrapped` is `true` on *every* onset hop in this synthetic feed —
meaning if the **other** guard, `if (wrapped && predictedBeatRegime_)` in
`updatePhase()` (`:219`), were loosened to unconditional `if (wrapped)`,
`advancePredictedBeat()` would fire on every onset hop *alongside*
`scoreBeat()` (since `predictedBeatRegime_` is false, that guard's own
condition is unaffected), producing exactly the claimed 8-bars-instead-of-4
failure. So the test protects `updatePhase()`'s guard, but gives **zero**
protection to `feedDownbeatFeatures()`'s guard — the commit message's
"mutation-tested... verified this fails at 8 if the gate is broken" claim is
therefore accurate for one specific line and, by this trace, not
demonstrable for the other.

**Fix recommended (fast-follow, not blocking):** add a test that forces
`predictedBeatRegime_=true` via `inSilence_` (call `feedSilenceDetection`
with low RMS enough times to set `inSilence_=true`) and then, on the same
hop, feeds a real high-confidence onset (`beatDetected_=true`) via
`processRawBPM`/`feedDownbeatFeatures` directly (bypassing the `inSilence_`
gate's early return isn't needed — just call `feedDownbeatFeatures` with a
manually-flipped `beatDetected_` state is not exposed publicly, so the
practical route is: hold RMS low, then call `processRawBPM(bpm, highConf,
beat=true)` while `inSilence_` is still `true`, and assert `barCount`
doesn't advance by 2 for that hop). This is the only way to make
`predictedBeatRegime_` and a real onset coincide, which is the scenario the
commit's own hazard description is about.

The other three new tests (`silence`, `manual-mode-cold`, `regression`) do
assert what they claim — traced separately and match expected mod-4 cycling
and bar counts within the stated tolerance windows.

### 6. Other observations

- **Thread safety [OK]:** `predictedBeatRegime_` is a plain `bool`, not
  `std::atomic` like `manualMode_` (`:175`). Verified this is correct, not an
  oversight: it's written in `runPipeline()` and read in `updatePhase()`/
  `feedDownbeatFeatures()`, all of which are called synchronously,
  same-thread, once per hop from `AnalysisThread`'s single run loop
  (`AnalysisThread.cpp:160-171`) or from single-threaded test code — no
  cross-thread access, so atomicity is unnecessary. `manualMode_` needs it
  because it's set from the UI/message thread and read from the analysis
  thread (comment at `:174`); `predictedBeatRegime_` never crosses threads.
- **Naming/dead code [OK, pre-existing]:** `isSilent()`/`silenceDuration()`
  zero-consumer status predates this commit (P23) and is explicitly
  acknowledged in the commit message as out of scope; not a new SLIM issue
  introduced here. `isSilent()` now has one consumer (the new silence test),
  `silenceDuration()` still has none anywhere.
- No new dead members, no naming complaints on `predictedBeatRegime_` /
  `advancePredictedBeat()` — clear names, comments accurately scoped.

## Files reviewed
- /Users/boriskarpman/projects/RealTimeAudio/src/analysis/BPMTracker.h
- /Users/boriskarpman/projects/RealTimeAudio/src/analysis/BPMTracker.cpp
- /Users/boriskarpman/projects/RealTimeAudio/tests/test_bpm_stabilization.cpp
- /Users/boriskarpman/projects/RealTimeAudio/src/analysis/AnalysisThread.cpp (+.h)
- /Users/boriskarpman/projects/RealTimeAudio/src/api/ApiServer.cpp (+.h)
- /Users/boriskarpman/projects/RealTimeAudio/src/MainComponent.cpp
- /Users/boriskarpman/projects/RealTimeAudio/src/features/FeatureBus.h
- /Users/boriskarpman/projects/RealTimeAudio/src/test/TestServer.cpp (+.h)
- /Users/boriskarpman/projects/RealTimeAudio/src/Main.cpp
- /Users/boriskarpman/projects/RealTimeAudio/src/audio/AudioEngine.cpp (log line only)
