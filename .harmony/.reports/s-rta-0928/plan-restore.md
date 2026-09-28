# plan-restore -- remove the message-thread hold at a restoring routine start / loop return, and land a gesture a stall steps over (s-rta-0928)

Authorship: architect plan written by an OPUS architect standing in for Fable. This is the session's recorded tier deviation
(s-rta-0928-work.md 08:45: the Fable quota ran out).
Base: main @ 6db8d67 (code 233eae7). Every file:line below is at HEAD.
Bins:
- VERIFIED: read at the line, or measured in a cited report.
- INFERRED: derived from something verified.
- ASSUMED: not checked; the builder checks it at the named step.

Inputs read in full:
- `.harmony/.reports/s-rta-0928/restore-diag.md` (the diagnosis; below it is called "diag").
- `.harmony/.reports/s-rta-0927/routines-timing.md`.
- `BORIS_DECISIONS.md:320-371` (Playback Behaviour).
- `.harmony/.reports/s-rta-0928/plan-renderleft.md` (the parallel lane, FINAL at 09:29).
- `.harmony/.reports/s-rta-0928/plan-tempo.md` (only its FENCE: shared files).

QUESTION: Remove the message-thread hold at a routine start / loop return that restores; both causes the diag isolated.
- (1) The thumbnail decode on the message thread at every `DeckView::refresh`. The hold must not come back with more image
  cells or 4K images.
- (2) The skipped-gesture write in `Player::advanceTo`.
Also: rule on each diag fix option; name the target; RED-first tests and live rows; what must not change; coordinate with
plan-renderleft's off-GL-thread decoder.

APPROACH (stated first):
1. Cause 1. The deck grid never decodes a file on the message thread.
   - A new header-only `ClipThumbnails` store (`src/ui/ClipThumbnails.h`). `DeckView` owns one, and every `ClipCell` and
     `LayerStrip` pulls from it.
   - It is the codebase's existing FilesBrowser pattern: a `juce::ThreadPool` (2 low-priority threads), a `callAsync`
     completion, a weak guard, and the mtime read beside the decode.
   - It sits over the existing, tested `ThumbnailCache` (an LRU keyed by path + mtime).
   - `get()` never decodes. A miss queues ONE decode per file and returns an invalid Image. The landing re-runs
     `DeckView::refresh()`.
   - Each cell and strip also memoizes WHAT its thumbnail was made from, so refreshing an unchanged cell does no thumbnail
     work.
   - Result: a restore's cost no longer depends on image count or size. Target hold <= 16 ms; expected < 1 ms.
2. Cause 2. `Player::advanceTo` writes a gesture's END value (`curve.eval(x1)`) before its release whenever a tick passes
   x1. This covers two cases: a stall stepped over the whole gesture (the diagnosed anomaly), or over its tail. It is one
   line. A refused end write releases nothing, as in-gesture.
3. Measurement.
   - A permanent, cheap `holdMs` / `holdMsMax` per routine slot on `/api/routine/status`: how long its last start / loop
     return held the message thread inside the engine.
   - probe-routines rows 5h / 8h / 11h hold it to <= 16 ms.
   - Row 7m stalls the app across the routine's first move and requires that the move still lands.
   - The diag's heartbeat method stays a TEMPORARY protocol. It cannot be a permanent row: the app's unexplained idle
     background alone reaches 20-28 ms in any +-250 ms window (diag section 3).
4. Coordination with renderleft: independent. This lane shares no decoder, cache or file with renderleft's `ImageDecode` /
   `ImageTexCache`. The two lanes meet only in append-only / different-function spots. Merge order is free; the rules are
   in FENCE.

## FACTS VERIFIED IN THE SOURCE

F1 (VERIFIED) The decode on every refresh.
- `DeckView::refresh` (`src/ui/DeckView.cpp:258-314`) calls `layerStrips_[..]->refresh()` (:272) and
  `layerCells[col]->setClip(..)` (:279) for EVERY strip and cell.
- `ClipCell::setClip` (`src/ui/ClipCell.cpp:349-368`) calls `updateThumbnail` (:394-412). For an Image clip with no
  `Clip::thumbnail`, that runs `juce::ImageFileFormat::loadFrom` + `rescaled(90, 72, low)` (:406-411).
- `LayerStrip::refresh` (`src/ui/LayerStrip.cpp:723-730`) calls `updateThumbnail` (:938-991). For an Image active clip
  that is `loadFrom` + `rescaled(sz, sz)` (:957-966).
- For a video / sequence it re-runs `clip->thumbnail.rescaled(sz, sz)` on EVERY refresh (:950-955). For a Source / FX-only
  clip it redraws a text placeholder on EVERY refresh (:968-988).
- `Clip::thumbnail` (`src/model/Clip.h:232`) is set only for video and sequences (`MainComponent.cpp:2929, 2939, 4882,
  4951, 6546`).

F2 (VERIFIED, diag section 4) Refreshes on the restore path.
- `handleClipTrigger` refreshes at `MainComponent.cpp:4369-4370`, `applyLayerFlag` at :5887, `applyClipPlaying` at :5946.
- There is one refresh per discrete restore entry: 3 on the probe deck.
- Cost:
  - 8.4-10.4 ms per test-card decode and 28-33 ms per 4K JPEG.
  - 99.6-99.9% of every restoring tick.
  - Restoring tick hold: card 52.8-56.9 ms (medians), big4k 176-179 ms, many 154-155 ms.
  - With no image: `startNow` 0.26-0.51 ms and `DeckView::refresh` 0.11-0.28 ms.
- A routine can never switch decks: `activeDeck` lanes are dropped at save (`docs/claude/recording.md:97-98`). So a restore
  never runs `rebuildGrid`.

F3 (VERIFIED) The in-repo off-thread thumbnail pattern.
- `FilesBrowser::requestThumbnailAsync` / `onThumbnailDecoded` (`src/ui/FilesBrowser.cpp:565-615`):
  - a pool job does only decode work and reads the mtime beside the decode (:574-580);
  - a `callAsync` completion is guarded by a SafePointer (:583-587);
  - the pool is drained FIRST in the destructor (:332-345);
  - 2 threads (`FilesBrowser.h:92-94`).
- `ThumbnailCache` (`src/ui/ThumbnailCache.h:15-115`, header-only, ctest `tests/test_thumbnail_cache.cpp`) is a bounded
  LRU (500) keyed by full path + mtime, with a race-free `put(file, mtimeAtRead, img)` (:54-57).
- `loadFrom` + `rescaled` already run on pool threads in production there.

F4 (VERIFIED) JUCE 8.0 facts the store relies on.
- `JUCE_DECLARE_WEAK_REFERENCEABLE` clears in its own master's destructor (`juce_WeakReference.h:243-246`).
- `ThreadPoolOptions::withDesiredThreadPriority` (`juce_ThreadPool.h:189-191`), `Thread::Priority::low` (`juce_Thread.h:77`).
- `Image::operator==` compares pixel-data identity (`juce_Image.h:145`).
- `JUCE_MODAL_LOOPS_PERMITTED` defaults to 0 (`juce_PlatformDefs.h:316/320`), so `runDispatchLoopUntil` does not exist in
  a ctest. The async tests inject the poster.

F5 (VERIFIED) The skipped gesture.
- `Player::advanceTo`'s continuous loop (`src/recording/Player.cpp:139-180`) touches at :157. At `pos >= g.x1` (:162-170)
  it releases with no `set()`. This happens whether the gesture opened on this call or an earlier one.
- A gesture's `x1` is its last breakpoint (`src/recording/Program.cpp:514-515`).
- `AutomationCurve::eval` clamps to the last point at `x >= xMax` (`src/connect/AutomationCurve.h:96-99`), so
  `g.curve.eval(g.x1)` is the recorded end value.
- A recorded release appends a flat closing point at the last value (`PerformanceRecorder.cpp:128-145`).
- The diag's stall A/B: a 450 ms stall gives the anomaly 5/5; 150 ms gives 0/5.

F6 (VERIFIED) What a refused write means at every sink.
- `SlotSink::set` is refused only when another routine owns the key (`RoutineEngine.cpp:125-139`), and then `release` is
  a no-op for it (:141-148).
- `manualWriteCore` refuses only a higher-rank hand (`src/connect/ManualWrite.h:63-67`), and `manualReleaseCore` never
  lets a lane close a human's grip (:69-71).
- The Player's rule is: a refused set means displaced, which means no release. See Player.cpp:172-177 and
  `firePreambleContinuous` :101-116, pinned by `tests/test_take.cpp:1023-1041`.

F7 (VERIFIED) Two existing ctests pin the old close: G5 and J2.
- G5: `tests/test_routine_engine.cpp:1137-1142`. J2: :1519-1525.
- Both expect `Release` with NO end-value `Set` where a gesture ends exactly on the loop point
  (`gesture(0.5, 0.1f, 4.0, 0.9f)`, length 4).
- These are the only strict sequences at a gesture end found by reading every `fd.on(op0` / `Ev::Set` site. The other
  close-sites count releases only (:343-346, :381, :408, :585-600, :817-818, :1083, :1192, :1798-1806).

F8 (VERIFIED) The legacy image lock.
- On the message thread, `Renderer::loadImage` / `clearImage` take `pendingImageMutex_` (`Renderer.cpp:62-74`). Every
  image trigger reaches them, including a restore's activeClip entry (`MainComponent.cpp:4323-4328`).
- The GL thread holds that same mutex WHILE decoding the pending image (`Renderer.cpp:175-192`).
- So a trigger that lands during a GL-thread decode of the previous one waits for the rest of that decode.
- The diag did not observe it (hand-off 0.02 ms). plan-renderleft R1.3 makes that frame top O(1) under the lock
  (plan-renderleft.md:598-606).

F9 (VERIFIED) renderleft's decoder.
- `ImageDecode::Decoder` (plan-renderleft.md:451-471) is a Renderer member: 3 low-priority threads, delivering full-size,
  GL-ready RGBA byte rows into polled `Mailbox`es (try-drained by the GL thread).
- Its rule R1-c keeps "one prefetch in flight, so a demand decode always finds a free thread" (:206-210).
- Its R1-e / FILED #1 leave thumbnails to this lane, with "reuse ImageDecode::Decoder + a (path, stamp) cache" as a
  suggestion (:233-234, :933-934).

F10 (VERIFIED) Status plumbing.
- `RoutineEngine::Status::Slot` (`RoutineEngine.h:69-96`) is copied in `publishStatus` (`RoutineEngine.cpp:862-916`) and
  serialized in `MainComponent::routineStatusVar` (`MainComponent.cpp:5763-5829`, per-slot fields :5790-5824).
- The stall hook `POST /api/debug/stall_message_thread` is TEST-ONLY (`ApiServer.cpp:283-288, 1582-1597`). It is present
  in main's build (TEST_SERVER ON). probe-routines row 7s already uses it (`probe-routines.sh:731-751`).

F11 (VERIFIED) Budgets and collisions.
- CLAUDE.md is 23,752 of 25,000 bytes. The pitfall index ends at 47 (:229).
- Three lanes each plan one or more new pitfalls: tempo 48 (plan-tempo.md:469-470); renderleft 48 + 49
  (plan-renderleft.md:907-911); this lane one.
- `docs/claude/recording.md` is also edited by the tempo lane, in a new paragraph after :11. This lane edits the Routines
  section (:44-60).

## RULINGS (every design fork)

### Cause 1 -- the diag's options

| option | ruling | why |
|---|---|---|
| A. set `Clip::thumbnail` for image clips at import / load / reconnect | REJECTED as the mechanism. KEPT: "one decode per image, not per refresh" | (a) The decode still runs ON the message thread at load, drop and deck-append (N images x 9-33 ms; a 16-still 4K deck appended mid-show is about 0.5 s). The task forbids that in a performance path. (b) Correctness depends on EVERY image-clip creation path setting it (drop, 2-image spread, mixed drop, Replace Content, load, append, REST / Eyes, old undo snapshots). A missed path silently brings the per-refresh decode back, or shows a blank cell if the fallback is removed. (c) Writes into live clips after the swap are the class `MainComponent.cpp:3008-3013` warns about. |
| B. skip the re-decode when nothing changed | ADOPTED as a per-cell / per-strip MEMO keyed on the thumbnail's SOURCE (type, image path, `Clip::thumbnail` identity, strip size), on top of the shared store | B alone keeps one decode per active-clip change (about 9 / 30 ms) and does nothing for fresh cells (rebuildGrid, deck switch). The store removes both. The memo keeps an unchanged cell's refresh at compares only, so a restore's k refreshes do not scale with cell count. |
| C. one refresh per restore (defer or coalesce) | REJECTED for this lane; FALLBACK NAMED | After A-kept + B + D, a refresh costs about 0.1-0.3 ms on the measured decks (INFERRED from the no-image counterfactual). With the memo it costs O(cells) of compare plus the existing repaint invalidation (INFERRED about 2-4 us per cell). An AsyncUpdater version moves the grid one message behind the model for every caller. FALLBACK: if a measured restore on a deck of >= 100 cells with >= 10 discrete entries holds > 4 ms, add a SYNCHRONOUS `DeckView::RefreshBatch` RAII scope around `routineEngine_.tick/fire` and `recorderHost_.tick` in MainComponent. While any batch is open, `refresh()` only sets a flag; the outermost scope flushes once, in the same tick. |
| D. decode off the message thread, swap in via callAsync | ADOPTED as the core | This is the FilesBrowser pattern (F3). It is the only option that is O(1) in image count AND size on every path (restore, trigger, load, deck switch). |

### Cause 2 -- the diag's options

| option | ruling | why |
|---|---|---|
| A. write the skipped gesture's end value | ADOPTED, generalized: EVERY gesture `advanceTo` closes by reaching `x1` writes `curve.eval(x1)` before its release | A stall can step over a gesture's TAIL as well as the whole gesture: a ramp whose last 0.3 s fall inside the stall is left mid-ramp today. That is the same defect with the same one-line fix. Cost: G5 / J2 change their expected sequences (F7), toward their own stated intent ("where the recording's own hand left op0"). |
| B. fewer long stalls | a side effect only | Cause 1's fix removes the 54-178 ms (up to about 1.6 s at scale, diag 7.3) self-inflicted stalls. Stalls from system load remain (loadpost1). |
| clamp the per-tick clock advance | stays REJECTED | It undoes Pitfall 42. |

FALLBACK for cause 2 if Harmony will not accept the G5 / J2 expectation changes: the NARROW rule. Write the end value only
when the touch and the close happen in the same `advanceTo` call; G5 and J2 stay untouched; the tail-skip case stays open
(filed).

### Other forks

- Share renderleft's `ImageDecode::Decoder` (its suggestion, F9)? NO: independent, no shared file.
  - (1) Different product. It delivers full-size GL-ready byte rows (33 MB per 4K still) into polled mailboxes; a
    thumbnail is a 26 KB `juce::Image` whose landing must NOTIFY the message thread. Reuse means either a second O(pixels)
    downscale pass, or a new thumbnail layout inside renderleft's fenced header.
  - (2) Capacity interference. A deck load queues 16-128 thumbnail decodes. On a shared 3-thread pool they would queue
    ahead of render DEMAND decodes, which breaks renderleft's R1-c guarantee and lengthens its visible layer "hold".
  - (3) Ownership. The Decoder is a Renderer member (render subsystem, context-loss lifecycle). The grid would need a
    Renderer reference.
  - (4) The UI layer already has the matching precedent (F3).
  - Cost of not sharing: 2 low-priority threads and about 100 lines.
- Make the diag's heartbeat a permanent test-gated row? NO.
  - Its window max includes the app's unattributed idle message-thread blocks (17-25 ms at about 15 Hz, diag 7.1), which
    alone fail a 16 ms bar.
  - It would add about 500 messages/s while armed.
  - Instead: an ATTRIBUTABLE in-engine measure (`holdMs`): two `steady_clock` reads per start / loop return. It is
    production-safe, so NOT gated, like `peak_frame_time_ms`.
  - The heartbeat stays the temporary protocol that checks "the same way as the diag".
- Prefetch every deck's thumbnails at composition load? NO (not requested). A never-seen image shows the empty cell look
  for about 10-60 ms (INFERRED, low-priority threads), once per file per session. It never appears at a restore or a
  trigger, because a cell is displayed, and so decoded, before it can be restored to.
- Pool: 2 threads (FilesBrowser's count) at `Thread::Priority::low`. Thumbnails must never compete with the render /
  analysis threads.

### Targets (the hold, measured as the diag did)

- T1 (diag metric, temporary protocol): the enclosing `tickFeaturePipeline` of every restoring start and loop return (Ease
  and Jump) on the card, big4k and many fixtures is <= 16 ms.
  - Expected: < 1 ms, i.e. not logged by the >= 1 ms scope (INFERRED from the no-image counterfactual, F2).
  - Before (diag section 3 medians): card 52.8-56.9, big4k 176.3-179.1, many 153.9-155.1 ms.
- T2 (diag metric): the +-250 ms heartbeat max at those events is <= the same launch's idle p90 + 5 ms. The idle p90 was
  23.2-26.4 ms, so the event stops standing out. Before (medians): card 51.7-58.8, big4k 178.2-180.0,
  many 154.8-156.7 ms.
- T3 (permanent): `bank[].holdMs` <= 16 ms at the Ease start, and `holdMsMax` <= 16 ms over an Ease loop and a Jump start
  + loop (rows 5h / 8h / 11h). Expected <= 1 ms. RED on the step-B lane app: about 50-67 ms (the F2 numbers).
- T4 (structural ctest):
  - 0 image decodes on the calling (message) thread across any number of `DeckView::refresh` calls;
  - <= 1 decode per distinct image file;
  - 0 store lookups per refresh for unchanged cells and strips.
- T5 (cause 2): C1-C3 / E1 go RED -> GREEN. Live: the diag's `stackstall450` gives 0/5 anomalies (was 5/5). Row 7m is RED
  on main and GREEN on the lane.

## DECISION / SPEC

Step order: RED first for each cause. Four product commits, then docs; each commit builds and passes ctest.

### Step 0 -- lane setup (no product change)

- Worktree lane, branch `lane/restore-0928` from 6db8d67.
  - Suggestion: reuse `.claude/worktrees/rta0928-w4` (branch `diag/restore-0928`, no product commits) and its configured
    `build-lane`, which was rebuilt clean at 09:08.
  - Main's build flags: Release, `AUDIODNA_BUILD_TEST_SERVER=ON`, `AUDIODNA_BUILD_SYPHON=ON` (diag PACKET QUALITY).
- Rig rules: `.harmony/HANDOFF.md:36-44`.
  - `df -h /System/Volumes/Data`; the live lock; `open -g` only; osascript quit; wait >= 45 s between lock holds.
  - Never lldb / dtrace / Instruments, never a full-screen capture, never synthetic input, never the Output window.
  - Probe HTTP clients use `Connection: close`.
- Baselines, recorded verbatim:
  - the ctest count on the lane base;
  - on MAIN's app, under the lock: probe-routines summary (98/0 expected), probe-routine-display summary, probe-step3,
    probe-beatclock and probe-deck-tabs summaries.

### Step 1 -- cause 2, RED (tests only)

1. `tests/test_take.cpp`: extend `FakeSink` (:60-86) with `std::string order;` and append 'T' / 'S' / 'R' in
   `touch` / `set` / `release`. No existing assertion reads it. Add, next to the D3/D5 cases (after :592), tag
   `[player][stall]`:
   - C1 "a gesture stepped over in one advanceTo lands its end value, then releases".
     - Setup: lane `layerKey(0,"scalar")` + scalar opacity; gesture `x0 1.173, x1 1.707`,
       pts `{(1.173, 0.5f), (1.707, 0.5f)}` (a one-write REST gesture, F5); `start(0)`; `advanceTo(0.92)`;
       `advanceTo(1.85)`.
     - REQUIRE: touches 1; sets == [0.5]; releases 1; `order == "TSR"`.
     - RED today: sets empty, order "TR".
   - C2 "several gestures stepped over in one call land in order; the last wins".
     - Setup: g1 `[1.0, 1.2]` at 0.3, g2 `[1.4, 1.6]` at 0.7; `advanceTo(0.5)`; `advanceTo(2.0)`.
     - Expect: sets [0.3, 0.7]; order "TSRTSR". RED.
   - C3 "a stall over a gesture's tail lands the recorded end, not the last tick's value".
     - Setup: g `[0, 1]` linear 0 -> 1; `advanceTo(0.5)`; `advanceTo(1.4)`.
     - Expect: last set == 1.0; releases 1. RED: the last set is 0.5.
   - C4 (pin, GREEN before and after) "a refused touch on a stepped-over gesture writes and releases nothing":
     `refuseNextTouch`; C1's jump; sets empty, releases empty.
   - C5 "a refused END write releases nothing".
     - Setup: g `[0, 1]`; `advanceTo(0.5)`; `refuseNextSet = true`; `advanceTo(1.2)`.
     - Expect: `sets.size() == 2`, `sets.back().second == 1.0`, `releases.empty()`.
     - RED today: the old close releases without a set.
2. `tests/test_routine_engine.cpp`, tag `[routine][engine][stall]`, next to :2151-2247. E1 "a tick gap that steps over a
   whole recorded move still lands it":
   - Routine `makeRoutine("m", 8.0, Off, false)`; `lanes[op0] = continuousLane(op0, { gesture(1.173, 0.5f, 1.707, 0.5f) })`.
   - `rig.tick(); rig.runTo(4.0); fire(0)` (Off: starts at 4.0); `rig.runTo(4.9375)`; `mark = log.size()`.
   - `rig.beat = 5.85; rig.tick();` This is a 0.456 s stall, the diag's 450 ms arm.
   - Expect: `fd.on(op0, mark)` == [Touch, Set 0.5, Release]; `lastSet(op0) == 0.5`; `slot(0).yielded == 0`.
   - SECTION control (GREEN on both): `rig.beat = 5.25` (lands INSIDE the move); `runTo(5.75)`; `lastSet(op0) == 0.5`,
     one Release.
3. Build and run both test executables. Record the RED lines verbatim: C1, C2, C3, C5, E1 FAIL; C4 and the E1 control
   PASS.

### Step 2 -- cause 2, fix (commit A)

1. `src/recording/Player.cpp:162-170`. The close branch becomes:
   ```cpp
               if (pos >= g.x1)
               {
                   // s-rta-0928 (restore-diag.md cause 2): the gesture's END lands before it lets go. A tick past x1 -- a
                   // stall that stepped over the whole gesture (touched and closed in this one call) or over its tail --
                   // leaves the knob where the recording did, never where the last tick caught it (or untouched). A refused
                   // write means another hand has the knob: release nothing (the in-gesture rule, below).
                   if (!cur.displaced && sink.set(lane.key, g.curve.eval(g.x1)))
                       sink.release(lane.key);
                   cur.inGesture = false;
                   cur.displaced = false;
                   ++cur.gestureIndex;
                   continue;   // pos may already reach the NEXT gesture too
               }
   ```
2. Comment updates.
   - `Player.cpp:144-147`: "... each one still gets its touch() / end-value set() / release() (D3/D5) -- never silently
     skipped."
   - `Player.h:42-52` (advanceTo doc), add one sentence: "A gesture whose end `pos` has reached lands its end value (the
     curve at x1) before its release -- a stall never drops a recorded move's final state."
3. The G5 / J2 expectation deltas (F7). Exact; nothing else in either test changes.
   - G5 (`tests/test_routine_engine.cpp:1127` and :1137-1142). Keep `const float gestureLast = rig.fd.lastSet(op0);` and
     add `CHECK(gestureLast < 0.9f);   // the last tick inside the gesture caught it short of its recorded end`. Then:
     ```cpp
             const auto ev0 = rig.fd.on(op0, at8);         // op0 held to the end: its end lands, then a quarter-beat spill
             REQUIRE(ev0.size() == 4);
             checkEvent(ev0[0], Ev::Set, op0);             // s-rta-0928: the gesture's recorded end (0.9) first ...
             CHECK(ev0[0].v == Approx(0.9f));
             checkEvent(ev0[1], Ev::Release, op0);         // ... then it lets go
             checkEvent(ev0[2], Ev::Touch, op0);
             checkEvent(ev0[3], Ev::Set, op0);
             CHECK(ev0[3].v == Approx(0.9f));              // the glide starts where the recording left op0: its end
     ```
   - J2 (:1519-1525):
     ```cpp
             const auto ev0 = rig.fd.on(op0, at8);         // held to the end: its end lands and lets go, then set straight to 0.1
             REQUIRE(ev0.size() == 5);                     // s-rta-0928: Set(0.9) (the gesture's end), Release, then the Jump restore
             checkEvent(ev0[0], Ev::Set, op0);
             CHECK(ev0[0].v == Approx(0.9f));
             checkEvent(ev0[1], Ev::Release, op0);
             checkEvent(ev0[2], Ev::Touch, op0);
             checkEvent(ev0[3], Ev::Set, op0);
             CHECK(ev0[3].v == Approx(0.1f));
             checkEvent(ev0[4], Ev::Release, op0);
     ```
4. Run the FULL ctest. Any OTHER failing expectation may be updated only if it has the same shape: a gesture end that now
   has its end-value Set right before its Release at the same tick. List each one in the report with its old and new
   lines. A failure of any other shape: STOP and report. Do not update it.
5. Teeth, run once: revert only the new `sink.set(...)` condition to the old `if (!cur.displaced) sink.release(...)`.
   C1 / C2 / C3 / C5 / E1 must FAIL. Restore, and check the file with `git diff --stat`.
6. Commit A: `fix(player): a gesture whose end a tick passes lands its end value before its release -- a stall no longer
   drops a recorded move (restore-diag cause 2)`.

### Step 3 -- the hold measurement + the live rows (commit B; rows 5h / 8h / 11h are RED at this commit by design)

1. `src/recording/RoutineEngine.h`, `Status::Slot` after `int glides = 0;` (:86):
   ```cpp
            // s-rta-0928 (restore-diag.md): how long this routine's last start / loop return / restart held the message
            // thread inside the engine -- its restore (both halves) and that tick's replay -- and the longest this run, in
            // ms; -1 before its first start. probe-routines rows 5h / 8h / 11h hold it to 16 ms.
            double holdMs = -1.0, holdMsMax = -1.0;
   ```
2. `src/recording/RoutineEngine.cpp`.
   - `#include <chrono>`. In the anonymous namespace (:24-85):
     ```cpp
         // s-rta-0928: Status::Slot::holdMs -- the engine's own steady clock, never the beat clock.
         double msSince(std::chrono::steady_clock::time_point t0)
         {
             return std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - t0).count();
         }
     ```
   - `Running` (after `bool glideScheduled = false;` :186): `double holdMs = -1.0, holdMsMax = -1.0;`.
   - `startNow` (:396).
     - First statement: `const auto holdStart = std::chrono::steady_clock::now();`.
     - LAST statements, after `notify(msg + ".");`:
       `r.holdMs = msSince(holdStart); r.holdMsMax = std::max(r.holdMsMax, r.holdMs);`
     - This covers the fire, bar and restart starts.
   - `tick()` loop end (:566).
     - First statement inside `if (pos >= r.lengthBeats)`: `const auto holdStart = std::chrono::steady_clock::now();`.
     - In the `if (r.loop && r.lengthBeats > 0.0)` branch, after `r.position = pos;` (:616): the same two lines.
     - The once-end branch records nothing.
   - `publishStatus` (after `sl.glides = ...` :903-904): `sl.holdMs = r.holdMs; sl.holdMsMax = r.holdMsMax;`.
3. `src/MainComponent.cpp` `routineStatusVar`, after `p->setProperty("glides", sl.glides);` (:5814):
   ```cpp
        p->setProperty("holdMs", sl.holdMs);         // s-rta-0928: the last start / loop return's message-thread hold (ms; -1 none)
        p->setProperty("holdMsMax", sl.holdMsMax);   // ... and the longest this run
   ```
4. ctest H1, `[routine][engine][hold]`, in `test_routine_engine.cpp`: "a start and every loop return publish how long they
   held the engine".
   - `FakeDispatch` (:113-183) gains `int preambleFireSleepMs = 0;`. Its `fire` lambda sleeps that long
     (`juce::Thread::sleep`) when `f.p.origin == Origin::Preamble`.
   - Routine: `makeRoutine("h", 4.0, Bar, true)` with `restoreStyle = Jump` (no glides) and a discrete preamble entry
     `{layerKey(0,"activeClip"), v 2}`; sleep 20.
   - Pending: `holdMs == -1`.
   - After `runTo(4.0)`: `holdMs >= 20 && holdMs < 2000`; `holdMsMax == holdMs`.
   - After `runTo(8.0)` (loop return): `holdMs >= 20`; `holdMsMax >= holdMs`.
   - SECTION `restoreState = false`: after the start, `0 <= holdMs < 20`.
   - RED is by construction (a new field). Teeth: record only around the continuous half; H1 must FAIL (holdMs < 20).
5. `.harmony/probe-routines.sh`, four rows. Document them in the header block (:13-19 style), each with its RED.
   - Row 5h. After the `for f in preambleUnresolved preambleRefused unresolved` loop (:708-711):
     ```bash
     HOLD5="$(rstat "'%.3f' % d['bank'][0]['holdMs'] if isinstance(d['bank'][0].get('holdMs'), (int, float)) else 'absent'")"
     num_leq "$HOLD5" 16 \
         && ok "5h: the Ease start's restore held the message thread $HOLD5 ms <= 16 ms (bank[0].holdMs)" \
         || no "5h: the Ease start held the message thread $HOLD5 ms (bank[0].holdMs; expected 0..16 -- RED: every deck refresh decoded each image thumbnail on the message thread, restore-diag.md)"
     ```
     `num_leq` (:582) rejects "absent" and a negative -1.000, so "no restore measured" FAILs.
   - Row 8h. After `rows python3 "$RT" loop ...` (:783) and BEFORE `P /api/routine/stop '{"slot":0}'`:
     - read `holdMsMax` the same way, and `cycle`;
     - PASS iff `cycle >= 2` AND `holdMsMax` <= 16;
     - text: `8h: the Ease start and every loop return (cycle N) held <= 16 ms (bank[0].holdMsMax X)`.
   - Row 11h. After `rows python3 "$RT" jumploop ...` (:923) and BEFORE the stop-all: the same test on `holdMsMax`, text
     `11h: the Jump start and its loop return (cycle N) held <= 16 ms (holdMsMax X)`.
   - Row 7m. Directly after the 7s block (:752), inside the same `if echo "$STALL_HOOK" | grep -q '"ok": true'` guard;
     else `echo "SKIP: 7m -- no TEST-ONLY stall hook"`.
     - Fire Probe Routine (slot 0: once, restore on, Ease; loop is still false here); `T7m="$(python3 "$RT" wait 0 running
       2.6)"`.
     - New rt.py command `stallat LO MS SECS OUT`:
       - poll `/api/routine/status` every 10 ms until `bank[0].position >= LO` (timeout 3 s);
       - record `p_before` and `t_stall`;
       - POST the stall `{"ms": MS}`;
       - then sample `{t, pos (rpos), op0 (/api/composition L0 opacity)}` every ~40 ms for SECS;
       - write JSON.
       - Call: `stallat 0.80 600 3.5 "$OUT/movestall.json"`.
       - 600 ms is 1.2 beats: from `p_before` in [0.80, 0.95] the resume lands at about 2.0-2.2, safely past the move's
         end (1.70-1.77; take stamps vary 1.173-1.237 + about 0.53) and before the next recorded event (L1 at beat 3.2).
     - New rt.py command `movestall JSON TAKE_FOLDER`:
       - `x0` = `first_move_beat` (:205-216). `x1` = the last curve point of that same gesture (a sibling helper,
         `first_move_span`, reading the same lane).
       - `p_after` = the first sample position > `p_before + 0.5`.
       - Precondition: `p_before < x0` AND `p_after > x1`. Else print
         `no 7m: inconclusive -- the stall did not span the first move (p_before P, p_after Q, move [x0, x1])`. It FAILS,
         never passes. Retry the row ONCE on inconclusive.
       - Then every sample with position in `[p_after, 5.0]` (>= 3 samples) reads L0 opacity 0.5 +- 0.05.
       - Row text: `7m: a 600 ms stall across the first move (positions P -> Q over [x0, x1]) still lands it: L0 opacity 0.5 at N samples in Q..5.0`.
       - RED text on main: `... every sample 1.0 (the move was touched and released unwritten -- restore-diag cause 2)`.
     - Then `P /api/routine/stop '{"all":true}'; sleep 0.3`.
     - If Harmony wants a smaller lane, cut this row first: E1 covers the engine, and the temporary protocol's stall arm
       covers the app.
6. Build the lane app at this commit and run probe-routines. Expected:
   - 5h / 8h / 11h RED with about 50-67 ms (the behavioural RED; record the lines);
   - 7m GREEN (commit A is in);
   - every old row GREEN.
   Run the new rows on MAIN's app too. Expected: 5h / 8h / 11h FAIL "absent", and 7m FAIL with every sample 1.0 (the
   diag's 5/5).
7. Commit B: `perf(routine): /api/routine/status holdMs / holdMsMax -- how long a start or loop return held the message
   thread; probe-routines 5h/8h/11h (hold) + 7m (a stall across a move)`.

### Step 4 -- the store (part of commit C)

1. NEW `src/ui/ClipThumbnails.h`, header-only, exactly this shape:
   ```cpp
   #pragma once
   #include "ui/ThumbnailCache.h"
   #include <juce_core/juce_core.h>
   #include <juce_events/juce_events.h>
   #include <juce_graphics/juce_graphics.h>
   #include <functional>
   #include <set>

   // ClipThumbnails (s-rta-0928, restore-diag.md): the deck grid's IMAGE-clip thumbnails, decoded OFF the message thread
   // once per file and shared by every ClipCell and LayerStrip (DeckView owns one). get() never decodes: a hit is a
   // ThumbnailCache lookup (path + mtime); a miss queues ONE decode per file on a low-priority pool thread and returns an
   // invalid Image; the result lands on the message thread and onLanded fires so the owner re-pulls. It replaces a decode
   // from disk on EVERY DeckView::refresh -- once per discrete routine-restore entry and on every clip trigger -- which
   // was 99.6-99.9% of a 54-178 ms message-thread hold. FilesBrowser's pattern (FilesBrowser.cpp:565-615), not a new one.
   class ClipThumbnails
   {
   public:
       static constexpr int kWidth = 90, kHeight = 72;   // the video / sequence Clip::thumbnail size (MainComponent.cpp:2929)

       using Decoder = std::function<juce::Image(const juce::File&)>;   // a pool thread: no Component, no model
       using Poster  = std::function<void(std::function<void()>)>;      // hands a completion to the message thread

       explicit ClipThumbnails(int maxEntries = ThumbnailCache::kDefaultMaxEntries)
           : cache_(maxEntries),
             decode_(&ClipThumbnails::decodeThumbnail),
             post_([](std::function<void()> fn) { juce::MessageManager::callAsync(std::move(fn)); })
       {
       }

       ~ClipThumbnails()
       {
           masterReference.clear();                        // a completion still queued finds nobody home
           pool_.removeAllJobs(true, kShutdownTimeoutMs);  // FIRST (FilesBrowser.cpp:332-345); jobs capture no `this`
       }

       // Message thread. The file's thumbnail; invalid while its decode is queued / running, or when it cannot be decoded
       // (a failure is retried only after the file's mtime changes). NEVER decodes on this thread.
       juce::Image get(const juce::File& imageFile)
       {
           jassert(juce::MessageManager::getInstanceWithoutCreating() == nullptr
                   || juce::MessageManager::existsAndIsCurrentThread());
           ++lookups_;
           if (auto hit = cache_.get(imageFile); hit.isValid())
               return hit;
           const auto key = imageFile.getFullPathName() + "|"
                          + juce::String(imageFile.getLastModificationTime().toMilliseconds());
           if (inFlight_.count(key) != 0 || failed_.count(key) != 0)
               return {};
           inFlight_.insert(key);
           ++queued_;
           juce::WeakReference<ClipThumbnails> self(this);
           pool_.addJob([self, imageFile, key, decode = decode_, post = post_] {
               const auto mtime = imageFile.getLastModificationTime();   // beside the decode (FilesBrowser.cpp:574-580)
               juce::Image image;
               try { image = decode(imageFile); } catch (...) {}
               post([self, imageFile, key, mtime, image] {
                   if (auto* store = self.get())
                       store->landed(key, imageFile, mtime, image);
               });
           });
           return {};
       }

       // Message thread: a queued decode finished, valid or not (DeckView re-runs refresh(); waiting cells re-pull).
       std::function<void()> onLanded;

       // Tests only, before the first get(): a counting decoder and a manual poster (a ctest runs no message loop:
       // JUCE_MODAL_LOOPS_PERMITTED is 0).
       void setBackendsForTests(Decoder d, Poster p) { jassert(queued_ == 0); decode_ = std::move(d); post_ = std::move(p); }
       int decodesQueued() const noexcept { return queued_; }
       int lookups() const noexcept { return lookups_; }

       // A pool thread: today's ClipCell decode (ClipCell.cpp:408-410), moved off the message thread. Invalid on a missing
       // or undecodable file.
       static juce::Image decodeThumbnail(const juce::File& f)
       {
           auto img = juce::ImageFileFormat::loadFrom(f);
           return img.isValid() ? img.rescaled(kWidth, kHeight, juce::Graphics::lowResamplingQuality) : juce::Image();
       }

   private:
       void landed(const juce::String& key, const juce::File& file, juce::Time mtimeAtDecode, const juce::Image& image)
       {
           inFlight_.erase(key);
           if (image.isValid())
               cache_.put(file, mtimeAtDecode, image);   // keyed by what was decoded (ThumbnailCache.h:50-57)
           else
               failed_.insert(key);
           if (onLanded)
               onLanded();
       }

       static constexpr int kShutdownTimeoutMs = 5000;
       ThumbnailCache cache_;
       std::set<juce::String> inFlight_, failed_;   // "path|mtimeMs": never queued twice; a failure waits for a new mtime
       Decoder decode_;
       Poster post_;
       int queued_ = 0, lookups_ = 0;
       juce::ThreadPool pool_ { juce::ThreadPoolOptions{}.withNumberOfThreads(2).withThreadName("ClipThumbs")
                                                        .withDesiredThreadPriority(juce::Thread::Priority::low) };
       JUCE_DECLARE_WEAK_REFERENCEABLE(ClipThumbnails)
       JUCE_DECLARE_NON_COPYABLE(ClipThumbnails)
   };
   ```
   `inFlight_` is erased on EVERY landing. So an entry the LRU later evicts is simply decoded once more; it is never stuck
   invalid (see S3).
2. NEW `tests/test_clip_thumbnails.cpp`, tag `[thumbnails]`.
   - Harness: `ScopedJuceInitialiser_GUI`; a per-process `ScratchDir` (copy `test_thumbnail_cache.cpp:20-51`); real files.
   - Manual poster: a `juce::CriticalSection`-guarded queue + an atomic `posted` + `waitFor(n, 5000 ms)` + `drain()` on the
     test thread.
   - Counting decoder: records `juce::Thread::getCurrentThreadId()` and returns a 90x72 image, or invalid when told.
   - S1 "get() never decodes on the calling thread; a miss queues one decode per file":
     - `get(a)` x10 and `get(b)` x10 before any landing: all invalid.
     - `waitFor(2)`; decoder calls == 2, none on the test thread; `decodesQueued() == 2`.
     - `drain()`: `onLanded` fires 2x; `get(a)` and `get(b)` are valid 90x72; `decodesQueued()` stays 2.
   - S2 "a failed decode is not retried at the same mtime; a new mtime is a new decode":
     - the decoder returns invalid for a; land; `get(a)` x5 invalid; `decodesQueued() == 1`;
     - `a.setLastModificationTime(+2 s)`; `get(a)` -> `decodesQueued() == 2`.
   - S3 "an entry the LRU evicted comes back through one new decode": `ClipThumbnails store(1)`; land a; land b (evicts a);
     `get(a)` -> `decodesQueued() == 3`; land; valid.
   - S4 "a completion that lands after the store is gone does nothing":
     - the poster pushes into a queue OUTSIDE the store's scope; `get(a)`; `waitFor(1)`; destroy the store; `drain()`;
     - no crash under the target's sanitizers; an `onLanded` counter (a shared_ptr<int> captured by value) stays 0.
   - S5 "decodeThumbnail: a real PNG gives 90x72; a text file or a missing file gives invalid". Write a 300x200 PNG with
     `juce::PNGImageFormat` (the `test_png_write.cpp:29` idiom).
   - Teeth, run once, recorded:
     - t1: `get()` returning `decode_(imageFile)` on a miss -> S1 FAILs;
     - t2: `landed()` not erasing `inFlight_` -> S3 FAILs;
     - t3: the completion calling through a raw captured `this` instead of `self` -> S4 FAILs under ASan.
3. `tests/CMakeLists.txt`: APPEND a `test_clip_thumbnails` block at EOF.
   - Shape: `test_thumbnail_cache`'s (:599-622), but linking `Catch2::Catch2WithMain` + `juce::juce_gui_basics` (for
     `ScopedJuceInitialiser_GUI` / `callAsync`), with `apply_sanitizers` + `catch_discover_tests`.

### Step 5 -- the grid RED (accessors + one test that compiles against the OLD grid)

1. Test seams only, no behaviour change:
   - `ClipCell.h`: `bool hasThumbnail() const { return thumbnail_.isValid(); }   // tests`.
   - `LayerStrip.h` (public): the same.
2. NEW `tests/test_deck_thumbnails.cpp`, tag `[thumbnails][deck]`. B0 "a refresh never decodes a fresh image on the
   calling thread":
   - Setup:
     - real PNGs in a ScratchDir;
     - `Composition comp; comp.initDefault();` (3 layers x 12 columns);
     - image clips (`mediaType Image`, distinct `id`s) at L0 C0 (active: `layers[0].activeClipColumn = 0`), L0 C2 and
       L2 C1, via `Deck::setClip` (`src/model/Deck.h:129`);
     - `DeckView dv; dv.setSize(1400, 600); dv.setComposition(&comp); dv.refresh()` x3.
   - Walk `dv`'s children recursively. Every `ClipCell` whose clip is Image, and the `LayerStrip` with `getLayerIndex() == 0`,
     must have `CHECK_FALSE(hasThumbnail())`.
   - RED today: true, because it was decoded synchronously on the calling thread (ClipCell.cpp:408, LayerStrip.cpp:959).
3. `tests/CMakeLists.txt`: APPEND a `test_deck_thumbnails` block.
   - Sources: `test_deck_thumbnails.cpp`, `${SRC_DIR}/ui/DeckView.cpp`, `${SRC_DIR}/ui/ClipCell.cpp`,
     `${SRC_DIR}/ui/RoutinePad.cpp`, plus `test_layer_strip_follows_model`'s whole source list (:1515-1532).
   - Links and definitions: the same as that block.
   - Add only the ONE .cpp per symbol the linker names (that block's rule). ASSUMED: this converges under about 30
     sources. The model accessors DeckView uses are inline (`Composition.h:190-200`, `Deck.h:44-51, 122-135`,
     `Layer.h:193-221`).
   - FALLBACK if it does not converge: test `ClipCell` + `LayerStrip` directly. Replicate `DeckView::refresh`'s calls
     (`setClip` + `setActive` per cell, `strip->refresh()`), and pass the store with `setThumbnails`. Say so in the
     report; the Boris check below then carries the "thumbnails still show" proof.
4. Build and run. Record B0's RED lines verbatim.

### Step 6 -- wire the grid (commit C = Steps 4-6)

1. `ClipCell.h`:
   - `class ClipThumbnails;` (forward);
   - public `void setThumbnails(ClipThumbnails* store) { thumbs_ = store; }`;
   - `updateThumbnail`'s comment (:69) becomes "never decodes -- s-rta-0928";
   - private: `ClipThumbnails* thumbs_ = nullptr; Clip::MediaType shownType_ = Clip::MediaType::None; juce::String shownPath_;`.
2. `ClipCell.cpp`: `#include "ui/ClipThumbnails.h"`. `updateThumbnail` (:394-412) becomes:
   ```cpp
   void ClipCell::updateThumbnail()
   {
       // s-rta-0928 (restore-diag.md): NEVER decodes. An Image clip's thumbnail comes from DeckView's ClipThumbnails --
       // decoded once per file off the message thread, invalid until it lands; a video / sequence's is its Clip::thumbnail.
       // Re-derived only when its source changed: every DeckView::refresh (once per discrete restore entry, every clip
       // trigger) used to decode each image cell from disk right here.
       const auto type = (clip_ != nullptr && clip_->hasMedia()) ? clip_->mediaType : Clip::MediaType::None;
       const bool cached = clip_ != nullptr && clip_->thumbnail.isValid();
       const juce::String path = (type == Clip::MediaType::Image && !cached) ? clip_->mediaFile.getFullPathName()
                                                                             : juce::String();
       // Same source: a cached picture is the same image; an image path has landed; anything else derives no picture
       // (so a video whose Clip::thumbnail was cleared never keeps the old one).
       if (type == shownType_ && path == shownPath_
           && (cached ? thumbnail_ == clip_->thumbnail
                      : (path.isEmpty() ? !thumbnail_.isValid() : thumbnail_.isValid())))
           return;
       shownType_ = type;
       shownPath_ = path;
       thumbnail_ = juce::Image();
       if (type == Clip::MediaType::None)
           return;
       if (cached)
           thumbnail_ = clip_->thumbnail;
       else if (path.isNotEmpty() && thumbs_ != nullptr)
           thumbnail_ = thumbs_->get(clip_->mediaFile);   // invalid while its decode is under way
   }
   ```
   - `setClip` (:349-368) is unchanged: the pointer is always updated, and the tooltip and `repaint()` run as today.
   - `paint` (:17-214) is unchanged. It still reads the clip live (name, badges, lock, missing-file marker), so only the
     thumbnail is memoized.
3. `LayerStrip.h`:
   - forward `class ClipThumbnails;`;
   - public `void setThumbnails(ClipThumbnails* store) { thumbs_ = store; }` (next to `refresh()` :40);
   - private (next to `thumbnail_` :129):
     ```cpp
         // s-rta-0928: what thumbnail_ was made from -- re-derived (rescaled / placeholder drawn) only when this changes.
         ClipThumbnails* thumbs_ = nullptr;
         Clip::MediaType shownType_ = Clip::MediaType::None;
         bool shownFxOnly_ = false;
         int shownSize_ = 0;
         juce::String shownPath_;
         juce::Image shownCached_;   // the Clip::thumbnail it rescaled (identity; holding it rules out a reused address)
     ```
4. `LayerStrip.cpp`: `#include "ui/ClipThumbnails.h"`. `updateThumbnail` (:938-991) becomes:
   ```cpp
   void LayerStrip::updateThumbnail()
   {
       // s-rta-0928 (restore-diag.md): NEVER decodes -- the ClipCell rule (an Image clip's picture from DeckView's
       // ClipThumbnails, a video / sequence's from Clip::thumbnail); rescaled / placeholder drawn only when the active
       // clip's source or the square's size changed (it used to re-decode or re-rescale on every refresh).
       const Clip* clip = layer_ != nullptr ? layer_->getActiveClip() : nullptr;
       int sz = thumbnailBounds_.getHeight();
       if (sz < 1) sz = 64;
       const auto type = clip != nullptr ? clip->mediaType : Clip::MediaType::None;
       const bool fxOnly = clip != nullptr && clip->hasEffects() && !clip->hasMedia();
       const juce::Image cached = clip != nullptr ? clip->thumbnail : juce::Image();
       const juce::String path = (type == Clip::MediaType::Image && !cached.isValid()) ? clip->mediaFile.getFullPathName()
                                                                                       : juce::String();
       if (type == shownType_ && fxOnly == shownFxOnly_ && sz == shownSize_ && path == shownPath_ && cached == shownCached_
           && (path.isEmpty() || thumbnail_.isValid()))
           return;
       shownType_ = type; shownFxOnly_ = fxOnly; shownSize_ = sz; shownPath_ = path; shownCached_ = cached;
       thumbnail_ = juce::Image();
       const juce::Image source = cached.isValid() ? cached
                                : (path.isNotEmpty() && thumbs_ != nullptr) ? thumbs_->get(clip->mediaFile) : juce::Image();
       if (source.isValid())
           thumbnail_ = source.rescaled(sz, sz, juce::Graphics::lowResamplingQuality);
       else if (type == Clip::MediaType::Source || fxOnly)
       {
           // ... the existing placeholder block, VERBATIM (LayerStrip.cpp:971-987) ...
       }
       repaint(thumbnailBounds_);
   }
   ```
   `setLayer` (:690-721) and `refresh` (:723-730) are unchanged.
   - Before this change, an image's strip square was made from the full picture. Now it is made from the store's 90x72,
     exactly as a video's already is (:952).
   - Same stretch-to-square and resampling quality. INFERRED: no visible difference at 76 px.
5. `DeckView.h`:
   - `#include "ui/ClipThumbnails.h"` (after :6);
   - public, after `void refresh();` (:35):
     ```cpp
         // s-rta-0928: the grid's image thumbnails, decoded off the message thread; every ClipCell / LayerStrip pulls from
         // it. Public for tests (setBackendsForTests before setComposition).
         ClipThumbnails& getThumbnails() { return thumbnails_; }
     ```
   - private, directly after `Composition* composition_ = nullptr;` (:105):
     `ClipThumbnails thumbnails_;   // declared before the strips / cells: destroyed after them`.
6. `DeckView.cpp`:
   - Constructor, first statement (:4): `thumbnails_.onLanded = [this] { refresh(); };   // s-rta-0928: a thumbnail landed -- waiting cells re-pull`.
   - `rebuildGrid`: `strip->setThumbnails(&thumbnails_);` between :147 and :148 (BEFORE `setLayer`), and
     `cell->setThumbnails(&thumbnails_);` between :184 and :185 (BEFORE `setClip`).
   - `refresh()` itself does not change.
7. `tests/test_deck_thumbnails.cpp` adds these cases.
   - B1 "a refresh storm decodes nothing on the calling thread; each image file decodes once, off-thread":
     - `dv.getThumbnails().setBackendsForTests(countingDecoder, manualPoster)` BEFORE `setComposition`.
     - `initDefault` + `deck.addLayer()` x5 = 8 layers. Image clips in columns 0-3 of every layer (32 image cells),
       4 distinct files. Every layer's active column is 0.
     - `setComposition`; `refresh()` x30; `waitFor(4)`.
     - Expect: decoder calls == 4, none on the test thread; no image cell and no strip `hasThumbnail()`.
     - `drain()`: every image cell and every strip `hasThumbnail()`.
     - `refresh()` x30 more: decoder calls still 4.
   - B2 "an unchanged cell re-derives nothing":
     - after B1's landing: `L = lookups()`; `refresh()` x30; `lookups() == L`;
     - then `comp.decks[0].layers[0].activeClipColumn = 1; dv.refresh();`: `lookups() == L + 1` (only L0's strip);
       decoder calls still 4 (that file is already decoded).
   - B0 stays: it runs on the REAL backends, and no message loop runs, so nothing ever lands.
   - Teeth, run once, recorded:
     - t4: the old synchronous decode re-inserted in `ClipCell::updateThumbnail` -> B0 FAILs;
     - t5: the memo removed (always re-derive) -> B2 FAILs (lookups grow by 32 + 8 per refresh).
8. Full ctest. The count is the baseline + S1-S5 + B0-B2 + C1-C5 + E1 + H1 (Catch sections are not separate ctests).
9. Commit C: `perf(ui): the deck grid never decodes an image on the message thread -- ClipThumbnails (one off-thread
   decode per file, shared by every cell and strip); a refresh re-derives only what changed (restore-diag cause 1)`.

### Step 7 -- live gates (lane app, Release + TEST_SERVER + SYPHON, lock held, quiet: no clang, load printed)

1. probe-routines x3 quiet: every row GREEN, including 5h / 8h / 11h (holdMs values printed, expected <= 1 ms) and 7m.
   New total = the old 98 + 4.
2. probe-routine-display x2: GREEN at its baseline count.
   - Also check that no window shot is taken within 0.5 s of a composition load. The shots compare teal / cue / cyan pixel
     counts between shots, and a shot taken before the test card's thumbnail landed would differ.
   - ASSUMED fine: the shots are seconds after load. If one is not, add a 0.5 s wait after its load, and only there.
3. probe-step3 (take replay: the Player change reaches replay), probe-beatclock (7s / stall rows), probe-deck-tabs
   (rebuildGrid paths): GREEN at their baselines.
4. TEMPORARY protocol, the diag's method (never committed):
   - Rig. Copy `restore/{drive.py,analyze.py,run1.sh,big4k.jpg}` from the diag evidence root
     (`/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/cfd08720-0a05-463c-84b1-89ae4354a3fb/scratchpad/restore/`)
     to your own scratchpad, and point `run1.sh`'s `W` at the lane worktree.
   - Instrumentation. `git apply --3way` the diag's `instr.diff` onto the lane head.
     - KEEP: `src/diag/DiagTrace.h`, the heartbeat start/stop in the MainComponent ctor/dtor, the `tickFeaturePipeline`
       scope, the `startNow` / `loopEnd` / `tickGap` markers, the `gestureSkipped` marker and the `DeckView::refresh` scope.
     - DROP the `updateThumbnail` decode-branch hunks: that branch no longer exists.
     - Build `build-lane`.
   - Runs:
     - `FIXVAR=card`, `big`, `many`: arms `ease,jump,loop,loopjump`, 5 each;
     - `IDLE=0 run1.sh stallA stackstall450,stackstall150 5`.
   - GREEN bars: T1 and T2 (above) per fixture; stall arm 0/5 anomalies on both arms (was 5/5 and 0/5). Report a table in
     the diag's section-3 shape, before (the diag's rows) vs after.
   - Remove the instrumentation with `git apply -R` of your own recorded diff, then rebuild. The stash-guard hook blocks
     a tree-wide checkout (restore-diag ISSUES).
     Required: `git status` clean except `build-lane`, and
     `strings <app binary> | grep -c '\[DG\]\|\[HB\]\|\[EV\]'` == 0.
5. Boris check. This is the visible change; present it per CLAUDE.md "Non-Technical User":
   - Load a show whose grid holds several big photos, including one 4K still.
   - The thumbnails appear within a blink.
   - Fire a looping routine that switches a layer back to a photo, with Start: Ease and then Jump.
   - At every start and loop point the app keeps moving: no freeze of the grid, the meters or the faders.

### Step 8 -- docs (commit D; rebase first -- see FENCE for the pitfall number)

1. `docs/claude/recording.md`. After the Restore-style paragraph (:56-60), inside the "Fire / restore / replay" bullet:
   > **Restore cost (s-rta-0928).** A start / loop return / restart restores synchronously in its tick. Its discrete half
   > refreshes the deck grid once per entry, and a refresh never decodes a file: image thumbnails come from `DeckView`'s
   > `ClipThumbnails` (`src/ui/ClipThumbnails.h`), decoded once per file off the message thread, and a cell or strip
   > re-derives its thumbnail only when its source changed (Pitfall NN). Before this, one refresh per entry decoded every
   > image thumbnail on the deck: 54 ms on the probe deck, 178 ms with one 4K still, k x (cells + layers) x decode in
   > general. `/api/routine/status` `bank[].holdMs` / `holdMsMax` = how long the last start / loop return held the message
   > thread inside the engine, and the longest this run (ms, -1 before the first); probe-routines rows 5h / 8h / 11h hold it
   > to 16 ms. **A move always lands its end:** when a tick passes a recorded gesture's end -- including a stall that
   > stepped over the whole gesture or its tail -- `Player::advanceTo` writes the gesture's final value before it lets go
   > (routines and take replay alike; a refused write releases nothing); probe-routines row 7m.
2. `docs/claude/pitfalls.md`.
   - New entry NN, after the last entry at rebase time:
     > NN. **A deck-grid refresh never decodes a file -- an image clip's thumbnail comes from DeckView's `ClipThumbnails`**:
     > `ClipCell::updateThumbnail` / `LayerStrip::updateThumbnail` decoded the image from disk (`ImageFileFormat::loadFrom`)
     > on EVERY `DeckView::refresh()`, and a refresh runs once per discrete routine-restore entry and on every clip trigger,
     > pause or layer flag from any source (human, REST, MIDI, OSC, replay, routine). A restoring start or loop return held
     > the message thread 54 ms on the probe deck and 178 ms with one 4K still (restore-diag.md: 99.6-99.9% of the hold).
     > `ClipThumbnails` (`src/ui/ClipThumbnails.h`; FilesBrowser's pool + callAsync pattern over `ThumbnailCache`, path +
     > mtime) decodes each file ONCE on a low-priority pool thread; `get()` never decodes (a miss is an invalid Image, and the
     > landing re-runs `refresh()`); cells and strips re-derive only when the thumbnail's source (type, path,
     > `Clip::thumbnail` identity, strip size) changed. A file edited in place keeps its old thumbnail until the next rebuild
     > (deck switch / load), like the compositor's path-keyed texture. Guards: `tests/test_clip_thumbnails.cpp`,
     > `tests/test_deck_thumbnails.cpp`; live: `/api/routine/status` `bank[].holdMs` (probe-routines 5h / 8h / 11h).
   - Pitfall 42 (:93). In item (1), after "-- the events were never dropped before either, only late" and before
     "; (2)", insert:
     > ; a continuous gesture whose END fell inside the gap (the whole gesture, or its tail) lands its end value before its
     > release (s-rta-0928, restore-diag cause 2: a 0.26 s one-write move stepped over by a >= 0.3 s stall was touched and
     > released unwritten; `tests/test_take.cpp [player][stall]`, probe-routines row 7m)
3. `CLAUDE.md`, index line NN after the last index line (<= 170 bytes):
   `NN. A deck-grid refresh never decodes a file (image thumbnails come from DeckView's ClipThumbnails) -- before touching ClipCell/LayerStrip thumbnails.`
   Then `wc -c CLAUDE.md` must be <= 25,000 after rebase. If over, pay per plan-tempo A8 (move text into docs/claude).
4. `.harmony/APP-INVENTORY.md:190`: "... lanes/preamble/stacking counters, holdMs/holdMsMax (s-rta-0928),
   lastSaved/lastError".
5. NOT edited by the lane:
   - `.harmony/HANDOFF.md`: Harmony closes item 4 at merge. Renderleft edits the neighbouring item 3, so an in-lane edit
     would make adjacent hunks.
   - `.harmony/notebook.md`: an optional append. The method worth keeping is the diag's "callAsync heartbeat + scope
     attribution + counterfactual fixtures". Harmony may add it.

## MUST NOT CHANGE (and how each is kept)

- Routine timing rows: every existing probe-routines row, including the position-window rows 8 / 11 / 11j and grids 7 / 7s,
  and every probe-routine-display row.
  - Kept: no timing code changes. The only new work in a tick is two clock reads, plus the end-value `set` at a gesture
    close. Step 7 runs them.
- Glide shape: Ease glides a straight line from what the control shows over the last beat ending ON the boundary (start,
  loop return, restart), with the quarter-beat spill; Jump cuts ON the bar.
  - Kept: `scheduleGlides` / `stepGlides` are untouched.
  - The one effect: at a loop point where a gesture ends, the glide's `from` is now the recorded end value (G5: 0.9) rather
    than the last tick's value (0.886). Pinned by G5 / J2's new lines, G1-G12, J1-J4, and rows 5g / 8g / 9g / 11j.
- Restore set: only the routine's own knobs (Boris). No change to `sliceRoutine` / `compileRoutine` / the preamble.
- "The later routine takes a knob over" (D9): `SlotSink` is untouched. The end-value `set` passes its owner check like any
  in-gesture set, and a refused end write releases nothing. Pinned by the stacking tests :604-668, G6 and row 11.
- The discrete restore still fires ON the boundary, synchronously, in the same tick. No deferral was introduced (option C
  rejected).
- The grid shows what it showed, with the same sizes (90x72 cells, a square strip) and the same placeholders. The only
  differences:
  - a never-seen image shows the empty-cell look for about 10-60 ms on first display (INFERRED);
  - a file edited in place updates at the next rebuild.
- "Routine pads and bands" (CLAUDE.md UI patterns) and Pitfall 41 (faders follow the model from the timer): RoutinePad,
  bands and `syncFromModel` are untouched.
- Sacred rules: no audio / analysis / render thread code changes; no new mutex (the store uses the MessageManager queue, as
  FilesBrowser does); no GL.

## FENCE and MERGE ORDER

This lane's files:
- `src/ui/ClipThumbnails.h` (NEW), `src/ui/ClipCell.{h,cpp}`, `src/ui/LayerStrip.{h,cpp}`, `src/ui/DeckView.{h,cpp}`.
- `src/recording/Player.{h,cpp}`, `src/recording/RoutineEngine.{h,cpp}`.
- `src/MainComponent.cpp`: `routineStatusVar` only, +2 lines.
- `tests/test_clip_thumbnails.cpp` (NEW), `tests/test_deck_thumbnails.cpp` (NEW), `tests/test_take.cpp`,
  `tests/test_routine_engine.cpp`, `tests/CMakeLists.txt` (2 appended blocks).
- `.harmony/probe-routines.sh`.
- `docs/claude/recording.md` (Routines section), `docs/claude/pitfalls.md`, `CLAUDE.md` (1 index line),
  `.harmony/APP-INVENTORY.md:190`.

NOT touched: `src/render/*`, `src/media/*`, `src/core/CompositionLoad.h`, `ApiServer.cpp`, `TestServer.cpp`, `Clip.h`, and
renderleft's new headers.

Where lanes meet:
- renderleft (FENCE plan-renderleft.md:826-846):
  - `MainComponent.cpp`, in different functions (theirs: `swapCompositionModel`, `openImageFolder`, `advanceSlideshow`);
  - `tests/CMakeLists.txt`, appends only (resolve per `notebook.md:1987-1989`: HEAD's file + the other lane's appended
    block);
  - `pitfalls.md` + the CLAUDE.md index.
- tempo (plan-tempo.md:966-968): `MainComponent.cpp` `perfRecord` only; `recording.md` after :11 (a different section);
  `pitfalls.md` + the CLAUDE.md index; `tests/CMakeLists.txt` appends. `test_tempo_start` links `Player.cpp`, but its
  cases do not observe gesture ends (ASSUMED; the rebased ctest run proves it).

Merge order: free. Whichever lane merges later rebases onto main first. Pitfall numbers are assigned AT REBASE:
- all three lanes plan "48";
- this lane writes "NN" in its docs until then;
- code comments cite `restore-diag.md`, never a pitfall number, so a renumber touches only the three doc files.

Renderleft R1.3 (legacy image lazy, O(1) under `pendingImageMutex_`) is the fix for F8's residual lock. Neither lane
waits on the other.

## RISKS (strongest counterargument first)

- R-1. "An async store adds a subsystem and a placeholder; option A (+C) is simpler and synchronous." It loses:
  - A still decodes on the message thread at load, drop and deck-append (a performance path when a deck is appended
    mid-show).
  - A is only as correct as the list of clip-creation paths that remember to set it.
  - The store is the existing FilesBrowser pattern, over the existing LRU, in ~100 header-only lines, with 5 unit cases
    and teeth.
  - The placeholder shows only at a never-seen image's first display, never at a restore or trigger.
- R-2. Memo staleness: a key that misses an input would show a stale thumbnail.
  - The key IS the derivation's input set (type, path, `Clip::thumbnail` identity, size). Everything else paint reads
    live, and `setClip` still repaints every refresh. B2 and t5 pin the memo.
  - Accepted staleness: a file edited in place, until the next rebuild. The compositor keys by path too (F1's cache; after
    renderleft R1-d it is re-validated at swap), and a swap rebuilds the grid, so both pick the new file up together.
- R-3. The cause-2 generalization changes G5 / J2 and adds one write per gesture close.
  - The added write is the value the knob is normally already at (a flat recorded tail, F5).
  - Status counters (`yielded` / `refusedByHand`) can count one refusal at a gesture's end that used to go unseen, but only
    when another hand took the knob exactly at that tick.
  - Fallback: the narrow rule (RULINGS).
- R-4. Residual message-thread holds this lane does not remove:
  - (a) F8: a trigger landing during a GL-thread legacy decode waits for it. Removed by renderleft R1.3; unmeasured until
    then.
  - (b) The idle 17-25 ms blocks (diag 7.1), which set T2's floor.
  - (c) `ClipCell::paint` stats every image / video file on every paint (`ClipCell.cpp:203-205`). This is message-thread
    I/O, not a decode. Filed.
  - (d) The per-refresh cost at scale, about 2-4 us per cell for compare + repaint invalidation (INFERRED). A 128-cell
    deck with 10 discrete entries is about 3-5 ms. The synchronous batch fallback is named if a measurement exceeds 4 ms.
- R-5. First display of a big never-seen deck: 2 low-priority threads decode about 16-33 images/s at 4K (INFERRED, E-cores
  about 2x slower). The grid fills in progressively, never blocking.
- R-6. `holdMs` measures inside the engine only, not the whole tick. The temporary protocol measures the whole tick
  (T1 / T2). Both are reported.
- R-7. Row 7m's timing. The stall must span the move. The precondition makes a miss "inconclusive" (a FAIL), never a false
  PASS. One retry. Cut 7m first if it flakes: E1 and the stall arm keep the proof.
- R-8. `test_deck_thumbnails` link-set growth (ASSUMED manageable). The fallback is named (Step 5.3).
- R-9. Probe-routine-display shots taken before a thumbnail lands (checked in Step 7.2).
- R-10. The unknown source of loadpost1's 0.34 s stall stays unknown. After cause 2's fix, such a stall no longer drops a
  recorded move.

## COMMIT SEQUENCE (each builds, passes ctest and reverts alone)

1. A: `fix(player): a gesture whose end a tick passes lands its end value before its release` (Steps 1-2; G5 / J2 deltas).
2. B: `perf(routine): /api/routine/status holdMs / holdMsMax; probe-routines 5h/8h/11h + 7m` (Step 3). Rows 5h / 8h / 11h
   are RED at this commit by design; the RED is recorded.
3. C: `perf(ui): the deck grid never decodes an image on the message thread -- ClipThumbnails + per-cell memo` (Steps 4-6).
4. D: `docs: recording.md restore cost + gesture end; pitfall NN; Pitfall 42 clause; CLAUDE.md index; APP-INVENTORY`
   (Step 8, after rebase).
5. The report commit: RED/GREEN lines verbatim, the temporary-protocol table, teeth t1-t5, and every expectation change.

## DONE LOOKS LIKE

- ctest green: baseline + S1-S5 + B0-B2 + C1-C5 + E1 + H1. The G5 / J2 deltas are exactly as specified; any other change
  is listed with its reason. Teeth t1-t5 and the Player / H1 teeth are each recorded once.
- probe-routines GREEN x3 quiet, with 5h / 8h / 11h / 7m. Their RED is recorded two ways: main's app (absent / 1.0-stuck)
  and the step-B lane app (about 50-67 ms).
- probe-routine-display x2, probe-step3, probe-beatclock and probe-deck-tabs GREEN at their baselines.
- The temporary protocol table: T1 <= 16 ms (expected not logged) and T2 within the idle band for card / big4k / many ×
  Ease / Jump / loop returns; stall arm 0/5 and 0/5. Instrumentation removed, `strings` == 0.
- The Boris check text is in the report.
- No file outside the FENCE; `wc -c CLAUDE.md` <= 25,000.

## FILED (found, not in this lane)

1. Sequence thumbnails. An ImageSequence clip's first-frame thumbnail decodes on the message thread at load, drop and
   deck-append (`MainComponent.cpp:2937-2939`, `:4949-4951`; renderleft FILED #6 lists `ImageSequence::open` too).
   `ClipThumbnails` can serve it: key on `sequenceFiles[0]` in the two memo functions, and drop both sync decodes.
2. `ClipCell::paint`'s per-paint `existsAsFile()` stat (`ClipCell.cpp:203-205`). A missing-file flag could be refreshed on
   the memo path instead.
3. The coalescing fallback (option C, synchronous `DeckView::RefreshBatch`), if a large-deck measurement ever exceeds 4 ms.
4. The diag's found_not_fixed 1 (idle 17-25 ms blocks) and 2 (the early-in-bar poll vs the Bar edge): untouched.
5. The narrow-vs-general cause-2 rule, if Harmony takes the fallback: the tail-skip case stays open.

REPORT_FILE: .harmony/.reports/s-rta-0928/plan-restore.md
STATUS: FINAL.
- Cause 1: `ClipThumbnails` (off-thread, once per file, FilesBrowser pattern over ThumbnailCache) + a per-cell memo. The
  grid never decodes on the message thread.
- Cause 2: `Player::advanceTo` lands every gesture's end value before its release (G5 / J2 deltas; narrow-rule fallback).
- Measurement: permanent `holdMs`, rows 5h / 8h / 11h / 7m; the diag's heartbeat protocol stays temporary.
- Coordination: independent of renderleft's decoder, no shared file, merge order free, pitfall numbers assigned at rebase.

----------------------------------------------------------------------------------------------------------------------
## HARMONY ADOPTION (s-rta-0928, 10:02) — OVERRIDES THE BODY WHERE THEY DIFFER
Blind council: attack-restore-gates.md (SOUND_WITH_FIXES, no MUST), attack-restore-threads.md (SOUND_WITH_FIXES, no
MUST). Both verified the threading (message-thread-only engine/Player; pool jobs touch only a copied juce::File) and the
F5/F6/F7 citations. ADOPTED as written: cause 1 = ClipThumbnails store (own 2-thread low-priority pool, NOT shared with
renderleft) + per-cell/strip memo; cause 2 = the GENERALIZED end-value write (every gesture advanceTo closes by reaching
x1 writes curve.eval(x1) before its release) — accepted knowingly: it also changes a normal loop-boundary Ease
glide-from value to the recorded end (0.9 vs the tick-sampled 0.886), which is the recording's own hand; goes on the
Boris list. Rulings:
- D1 Teeth: Step 2.5's revert check also requires G5 and J2 to flip RED.
- D2 Citation: the Ease-glide rows that must not move are 5g / 8g / 9g / 11g (not 11j).
- D3 Row text for 5h / 8h / 11h / 7m says "holdMs covers the two diagnosed causes (thumbnail decode, skipped gesture)
  only" — a GREEN is not "no restore hold exists". Also re-run the diag's TEMPORARY heartbeat protocol (T1/T2) once on
  the lane app (instr.diff re-applied in a scratch copy, never committed) and report T1/T2 numbers.
- D4 No stat() per refresh while a decode is in flight: ClipThumbnails::get() checks inFlight/failed BEFORE any
  ThumbnailCache lookup that stats the file; ctest asserts a pending path is not re-stat'ed per refresh (count via a
  test hook or a counting FileStat seam — smallest mechanism).
- D5 Stacking test (threads SHOULD 2): routine A holds a gesture ending at beat T; routine B takes the same key just
  before T; assert A's end-value set is REFUSED (no write, no release of B's grip) and yielded / refusedByHand counts
  match the existing stacking tests' expectations (the "once per gesture" comment at RoutineEngine.cpp:~125-131 is
  updated to the new truth).
- D6 Re-grep every wiring site before inserting (citations drifted 1-3 lines: DeckView.cpp:186 setClip in rebuildGrid,
  ClipCell.cpp:393 updateThumbnail).
- D7 FENCE unchanged (renderleft owns src/render/*, ImageDecode/ImageTexCache; tempo owns FeatureSnapshot/BPMTracker/
  RecorderHost/MainComponent perfRecord). Pitfall number "NN" until rebase, as planned.
- D8 Boris list: the loop-boundary glide-from change (D-cause-2 above); a never-seen image cell shows the empty-cell
  look for ~10-60 ms once per file per session.
