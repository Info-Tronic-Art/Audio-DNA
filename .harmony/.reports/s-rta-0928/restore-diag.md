# s-rta-0928 lane restorediag -- what holds the message thread at a restoring routine start, and the loaded-run stack anomaly

## BUILDER REPORT

STATUS: DONE
RESULT: Both causes are isolated by measurement, and no product code changed. (1) The restore hold is image decoding. Each discrete restore entry calls `DeckView::refresh()`, and every refresh decodes each image thumbnail on the deck from disk, on the message thread. That accounts for 99.6-99.9% of the hold in every restoring event: 50 of 50 test-card events, 20 of 20 on the 4K and many-cell decks. (2) The stack anomaly happens when the routine clock jumps over a whole short gesture in one tick. `Player::advanceTo` then touches and releases that knob but never writes the value. A 450 ms stall reproduces it 5 of 5 times; a 150 ms stall (control) gives 0 of 5. CPU load alone never caused it in 20 runs.
FACTS: `src/ui/DeckView.cpp:258` (refresh: `layerStrips_[..]->refresh()` :272, `layerCells[col]->setClip(..)` :279); `src/ui/ClipCell.cpp:352` (setClip -> updateThumbnail) and `src/ui/ClipCell.cpp:408` (`juce::ImageFileFormat::loadFrom` each call); `src/ui/LayerStrip.cpp:959` (same decode for the active clip; the cached branch at :950 is used only for video/sequence); `src/model/Clip.h:232` (`Clip::thumbnail`, never set for image clips: only `src/MainComponent.cpp:2929` / `:2939` / `:4882` / `:4951` set it, all video/sequence); refresh callers on the restore path `src/MainComponent.cpp:4370` (handleClipTrigger), `src/MainComponent.cpp:5887` (applyLayerFlag), `src/MainComponent.cpp:5946` (applyClipPlaying); `src/recording/RoutineEngine.cpp:413` / `:599` (firePreambleDiscrete at the start / the loop return); `src/recording/Player.cpp:157` + `:162` (touch, then immediately `pos >= g.x1` -> release with no `set`). Evidence: `/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/cfd08720-0a05-463c-84b1-89ae4354a3fb/scratchpad/restore/summary-hold.txt`, `/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/cfd08720-0a05-463c-84b1-89ae4354a3fb/scratchpad/restore/summary-calltree.txt`, `/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/cfd08720-0a05-463c-84b1-89ae4354a3fb/scratchpad/restore/summary-stack.txt`, `/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/cfd08720-0a05-463c-84b1-89ae4354a3fb/scratchpad/restore/instr.diff`
METHOD: I added temporary instrumentation (`instr.diff`) and built it in `build-lane`: steady_clock scopes on every call of the start, restore, glide, loop-return and UI-refresh path, plus a message-thread heartbeat. A background thread keeps one `callAsync` ping in flight, re-posts 2 ms after it is served, and logs any latency of 2 ms or more. I also added `tickGap` and `gestureSkipped` markers. A REST driver (`drive.py`) runs the probe's fixture, take and routine and fires each arm 5 times per launch: Ease start, Jump start, no-restore start, 5 Ease loop returns and 5 Jump loop returns. I ran two quiet launches on the probe fixture, one each on 3 counterfactual fixtures (no image, a 4K JPEG, 5 image cells), a deterministic stall reproduction, and two loaded launches. Each launch held the lock and started after `wait_quiet`, with the load average printed.
CONFIDENCE+VERIFY: High for both mechanisms. Cause 1: the counterfactuals move the hold exactly as the decode count predicts. The no-image fixture drops to the background level (tick < 1 ms; ±250 ms heartbeat max 20-23 ms, the same as with no restore). 4K goes to 178 ms and 5 image cells to 154 ms, both linear in decodes. Cause 2: the deterministic A/B above, with the app's own `gestureSkipped` line on every anomalous rep. To re-prove: `git -C <worktree> apply` `instr.diff`, build `build-lane`, then run `run1.sh cardA ease,jump,norestore,loop,loopjump 5` and `analyze.py runs/cardA`. Expect "attributed %" >= 99.6 and a tick hold of 50-67 ms for restoring events. For the stack, `IDLE=0 run1.sh stallA stackstall450,stackstall150 5` should give anomaly True 5 of 5 vs False 5 of 5.
UNKNOWNS/NOT-DONE: No fix is implemented (diagnosis lane). I could not reproduce the loadpost1 stack anomaly from CPU load alone: 0 of 20 runs at load averages 10-101, and the largest tick gap was 65 ms, which is the restore hold itself. The source of loadpost1's 0.34 s stall is not identified; it had compilers running, not busy loops. About 17-25 ms of message-thread busy time at ~15 Hz is present with no routine at all; I did not attribute it, and it is outside this lane.
NUANCE: The prior report's "9-22 ms for a start without a restore" is not a start cost. A no-restore `startNow` takes 0.03-0.07 ms. What it measured is the app's always-present background: the idle 500 ms-window maximum heartbeat lag has a median of 19.9-23.1 ms. Ease and Jump pay the same hold, because the continuous half costs <= 0.03 ms either way; only the discrete half (clip, pause, resume) refreshes the deck. The glides and the Boris rulings are not involved in the cost. The hold grows with the number of image cells, image size and discrete restore entries, and a real deck will be far above the probe's 54 ms.
HANDOFF-NEEDS: none

INBOX-RECHECK: none

### SUMMARY
The 38-86 ms hitch at a restoring start or loop return is thumbnail decoding in `DeckView::refresh()`. The restore's discrete half fires 3 entries here: the active clip, pause C1, resume C0. Each one refreshes the whole deck, and each refresh decodes every image thumbnail on the deck from disk. With 1 image cell plus 1 image layer that is 2 decodes per refresh and 6 per restore, about 8.7 ms each for the 1504x1032 test card. The loaded-run stack anomaly is `Player::advanceTo` jumping over a whole single-write gesture ([1.173, 1.707] beats, about 0.26 s) during a message-thread stall of about 0.3 s or more. The knob is touched and released, but its value is never written.

---

## 1. Instrumentation points (temporary; `instr.diff`, reversed, never committed)

| where | what is timed / logged | log line |
|---|---|---|
| `src/diag/DiagTrace.h` (new, header-only) | `diag::Scope` (steady_clock, depth, min-duration filter); `diag::Heartbeat`: a background `std::thread`, one `MessageManager::callAsync` ping in flight, re-posted 2 ms after it is served; latency (serve - post) logged when >= 2 ms, and a summary every 10 s | `[DG] t0 depth name dur`, `[HB] post lat`, `[HBS]` |
| `MainComponent` ctor/dtor | heartbeat start/stop | |
| `MainComponent::tickFeaturePipeline` | whole (>=1 ms) + signalRegistry, mappingEngine, macroBank, recorderHost, connectionEngine, tickModulation (>=0.5 ms) | |
| `MainComponent::timerCallback` | whole (>=1 ms) + RecordPanel refresh, inspector refresh, routine view (>=0.5 ms) | |
| `recorderHost_.dispatch.fire` lambda (= `routineEngine_.dispatch.fire`) | every call, preamble vs replay, with control/layer/col/v/action | `[EV] dispatch.fire ...` |
| `handleClipTrigger` | whole + triggerClipImmediate, image/source preview hand-off, `deckView_->refresh()` (always) | |
| `manualWrite` / `manualTouch` / `manualRelease` | >= 0.1 ms | |
| `routineEngine_.dispatch.notify`, `RecordPanel::setNotice` | always | |
| `RoutineEngine::fire` | whole + `compileRoutine` (always) | `[EV] fire` |
| `RoutineEngine::startNow` | whole + firePreambleDiscrete, stepGlides, firePreambleContinuous, advanceTo(0), notify (always) | `[EV] startNow slot restore jump glideScheduled` |
| `RoutineEngine::tick` | whole (>=0.5 ms); loop end: advanceTo(end), player stop, firePreambleDiscrete/Continuous, advanceTo, stepGlides (always); running/pending stepGlides, refreshBank, publishStatus (>=0.25 ms); tick gap > 40 ms | `[EV] loopEnd ...`, `[EV] tickGap wallMs beat a -> b` |
| `Player::advanceTo` | a gesture touched AND closed in one call | `[EV] gestureSkipped lane x0 x1 from to accepted y1` |
| `DeckView::refresh` (>=0.1 ms) + its LayerStrip refresh; `DeckView::setRoutineView`; `DeckView::paint`, `LayerStrip::paint`, `ClipCell::paint` (>=0.25 ms); `InspectorPanel::refresh` (>=0.5 ms) | | |
| `ClipCell::updateThumbnail`, `LayerStrip::updateThumbnail` | the image-decode branch, always | `cc.` / `ls.updateThumbnail.decodeImage` |

Analysis: `analyze.py`. Per routine event it reports the enclosing top-level scope (the tick), the heartbeat lag covering it, the maximum heartbeat lag within ±250 ms (the prior report's window), a flat per-call attribution, the next 150 ms of heartbeat and paint, and the idle background window.

## 2. Runs (all `open -g` via `start_app`, the live lock held per launch, no Output window: every `outwins` read 0)

| tag | fixture | arms x reps | load avg at start -> end | evidence |
|---|---|---|---|---|
| cardA | probe fixture (test_card.png 1504x1032 PNG, 205 KB) | ease, jump, norestore x5; loop, loopjump x5 cycles | 3.62 -> 3.25 | `runs/cardA/` |
| cardB | same | same | 3.44 -> 3.55 | `runs/cardB/` |
| sourceA | L0 C0 = solid_color source (no image anywhere) | ease, jump, norestore x5; loop x5 | 3.29 -> 2.93 | `runs/sourceA/` |
| bigA | L0 C0 = 3840x2160 JPEG (1.6 MB, `big4k.jpg`) | same | 2.61 -> 3.42 | `runs/bigA/` |
| manyA | 4 columns; 5 image cells (L0 C0,C2,C3; L1 C2,C3) | ease, jump, norestore x5 | 50.13 -> 10.01 (decaying from loadB; no compiler, `wait_quiet` passed; per-decode 8.5 ms = cardA/B's 8.7 ms) | `runs/manyA/` |
| stallA | probe fixture, stack, TEST-ONLY `/api/debug/stall_message_thread` at Probe Routine position ~0.92-0.96 | stackstall450 x5, stackstall150 x5 | 2.95 -> 3.07 | `runs/stallA/` |
| loadA | probe fixture, stack, 14 synthetic busy loops | stack x10 | 10.39 -> 86.65 | `runs/loadA/` |
| loadB | same, 30 busy loops | stack x10 | 45.70 -> 101.53 | `runs/loadB/` |

The busy loops were killed before every lock release (`busy loops left: 0`). The Takes the driver recorded were moved out of `~/Documents/Audio-DNA/Takes` into `restore/takes/`.

## 3. The hold: measured per arm (`summary-hold.txt`)

"tick hold" = the enclosing `tickFeaturePipeline` scope. A tick under 1 ms is not logged; "not logged" means < 1 ms. "±250 ms HB max" = the largest heartbeat lag within ±250 ms of the event. "attributed" = `DeckView::refresh` time / tick time.

| fixture | event | n | tick hold ms, med [min-max] | ±250 ms HB max ms, med [min-max] | image decodes ms, med/max | attributed % |
|---|---|---|---|---|---|---|
| card | start Ease | 10 | 52.8 [51.8-55.1] | 51.7 [50.1-56.1] | 52.4 / 54.5 | 99.7 (min 99.7) |
| card | start Jump | 10 | 55.7 [52.2-66.9] | 58.8 [50.1-65.5] | 55.3 / 66.3 | 99.7 (min 99.6) |
| card | loop return, Ease | 10 | 56.1 [52.4-62.1] | 54.7 [49.7-61.8] | 55.6 / 61.6 | 99.7 (min 99.6) |
| card | loop return, Jump | 10 | 56.9 [52.6-65.2] | 56.4 [50.5-62.7] | 56.4 / 64.7 | 99.7 (min 99.6) |
| card | start of a looping routine (Ease / Jump) | 2 / 2 | 60.4 / 54.5 | 59.7 / 53.4 | 59.9 / 54.0 | 99.7 |
| card | **start, no restore** | 10 | **< 1** (`startNow` 0.03-0.07 ms) | 21.1 [18.2-28.5] | -- | -- |
| source (no image) | start Ease / Jump / loop return | 5 / 5 / 5 | **< 1** (`startNow` 0.26-0.51 ms; `DeckView::refresh` 0.11-0.28 ms) | 21.0 / 21.3 / 23.2 | -- | -- |
| source | start, no restore | 5 | < 1 | 20.2 [12.4-29.0] | -- | -- |
| big4k | start Ease | 5 | 179.1 [175.7-182.4] | 178.2 [175.5-182.5] | 178.5 / 181.8 | 99.9 |
| big4k | start Jump | 5 | 176.3 [174.2-183.7] | 178.6 [172.6-181.3] | 175.8 / 183.0 | 99.9 |
| big4k | loop return | 5 | 178.1 [177.7-181.1] | 180.0 [175.3-181.1] | 177.6 / 180.5 | 99.9 |
| many (5 image cells) | start Ease | 5 | 153.9 [152.1-155.8] | 154.8 [152.1-167.0] | 152.9 / 154.7 | 99.9 |
| many | start Jump | 5 | 155.1 [153.8-156.5] | 156.7 [153.9-172.6] | 154.0 / 155.4 | 99.9 |
| any | `RoutineEngine::fire` incl. `compileRoutine` | 81 | 0.02-0.07 ms | -- | -- | -- |

**Background (no routine; idle 2.5-5.5 s per launch):** heartbeat lags >= 2 ms have a median of 6.3-8.1 ms and a p90 of 14.3-19.0 ms. The per-500 ms-window maximum has a median of 19.9-23.1 ms, a p90 of 23.2-26.4 ms and a maximum of 23.2-28.5 ms. The prior report's "no-restore start 9-22 ms" falls inside this band, so it is background. **After the restoring tick:** the next 150 ms holds a heartbeat maximum of 17.1-17.5 ms (median; max 22.4) and paint scopes summing to 1.77 ms (median; max 3.07), which is also background. The repaint that the refreshes trigger costs <= 3 ms.

## 4. Call tree, per call (`summary-calltree.txt`; card = 40 restoring events: 10 Ease starts, 10 Jump starts, 20 loop returns)

| call | card med / max ms | big4k med / max ms (n=15) | many med / max ms (n=10) |
|---|---|---|---|
| `MainComponent::tickFeaturePipeline` (the whole tick) | 54.14 / 66.86 | 178.14 / 183.67 | 154.63 / 156.52 |
| `RoutineEngine::startNow` or the loop end in `tick()` | 54.12 / 66.85 | 178.13 / 183.65 | 154.61 / 156.51 |
| &nbsp;&nbsp;`Player::firePreambleDiscrete` -> dispatch 1: `activeClip` L0 -> C0 (`handleClipTrigger`, immediate) | 20.04 / 29.61 | 63.88 / 68.51 | 52.98 / 53.77 |
| &nbsp;&nbsp;&nbsp;&nbsp;`DeckView::refresh()` | 19.98 / 29.52 | 63.81 / 68.40 | 52.91 / 53.70 |
| &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;`LayerStrip::updateThumbnail` image decode (L0 active clip) | 10.42 / 16.05 | 33.35 / 37.02 | 8.88 / 9.47 |
| &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;`ClipCell::updateThumbnail` image decode(s) (1 cell; many: 5) | 9.45 / 13.29 | 31.61 / 32.63 | 43.79 / 44.30 |
| &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;rest of the refresh | 0.13 / 0.18 | 0.15 / 0.17 | 0.33 / 0.36 |
| &nbsp;&nbsp;&nbsp;&nbsp;rest of the handler (`triggerClipImmediate` 0.00, `loadImage` hand-off to the GL thread 0.02) | 0.07 / 0.11 | 0.07 / 0.13 | 0.06 / 0.08 |
| &nbsp;&nbsp;dispatch 2: `playing` pause L0 C1 (`applyClipPlaying` -> `DeckView::refresh()`) | 17.52 / 20.30 | 56.80 / 58.39 | 50.69 / 51.18 |
| &nbsp;&nbsp;&nbsp;&nbsp;LayerStrip decode / ClipCell decode(s) / rest | 9.02 / 8.56 / 0.10 | 28.21 / 28.49 / 0.13 | 8.45 / 41.82 / 0.28 |
| &nbsp;&nbsp;dispatch 3: `playing` resume L0 C0 (`applyClipPlaying` -> `DeckView::refresh()`) | 16.86 / 19.34 | 56.82 / 59.40 | 50.58 / 52.50 |
| &nbsp;&nbsp;&nbsp;&nbsp;LayerStrip decode / ClipCell decode(s) / rest | 8.38 / 8.41 / 0.09 | 28.27 / 28.33 / 0.12 | 8.41 / 41.93 / 0.28 |
| &nbsp;&nbsp;continuous half (Ease: `stepGlides`; Jump: `firePreambleContinuous`) + `advanceTo(0)` + start notice + `RecordPanel::setNotice` | 0.02 / 0.03 | 0.02 / 0.03 | 0.03 / 0.03 |

- **Decode unit cost:** test card 8.4-10.4 ms, 4K JPEG 28-33 ms.
- **Decodes per `DeckView::refresh()`:** one per image `ClipCell` on the active deck, plus one per layer whose active clip is an image.
- **Decodes per restore:** that count times the number of discrete restore entries: 6 here, 18 on "many".

Nothing else on the path costs more than 0.2 ms. These were ruled out by measurement: take/program load or parse, file I/O other than the thumbnail decodes, per-routine allocation, locks, waits on the render thread, glide scheduling, notices and paints.

Side measurement: the routine's replayed recorded trigger (L0 -> C1 at routine beat ~5.18) goes through the same refresh path. It costs 8.8-14.8 ms (card, n=40) and 27.9-33.5 ms (4K, n=10) against 0.03-0.29 ms without images. Every clip trigger, pause/resume and layer flag on the active deck pays this, whether it comes from a human, REST, MIDI, OSC, a take replay or a routine.

## 5. Loaded-run stack anomaly (found_not_fixed #3)

**Reproduced deterministically; not reproduced from CPU load alone.** Data: `summary-stack.txt`.

| arm | n | anomaly (no L0 0.5 in Probe Routine positions 1.8-5.0) | position jump across the stall | app log |
|---|---|---|---|---|
| 450 ms message-thread stall at position 0.92-0.96 (quiet, load 2.95) | 5 | **5/5** (every sample 1.0) | 0.917-0.960 -> 1.813-1.877 | `tickGap wallMs=453.9-463.6` + `gestureSkipped lane=1 x0=1.173 x1=1.707 ... accepted=1 y1=0.500` on every rep |
| 150 ms stall (control) | 5 | 0/5 (every sample 0.5) | 0.917-0.939 -> 1.237-1.280 (lands INSIDE the gesture) | `tickGap 151.4-174.0`, no `gestureSkipped` |
| no stall, 14 busy loops (load 10 -> 87) | 10 | 0/10 | none > 0.3 beats | max `tickGap` 65.2 ms (a restore hold), no `gestureSkipped` |
| no stall, 30 busy loops (load 46 -> 102) | 10 | 0/10 | none | max `tickGap` 65.3 ms, no `gestureSkipped` |

**Mechanism (verified by the runs above; code at `src/recording/Player.cpp:150-178`):**
1. A plain REST/MIDI write is recorded as a one-write gesture of about 0.26 s: here x0 = 1.173 and x1 = 1.707 beats, the take's L0 opacity 0.5 (`restore/takes/*/take.json`).
2. The routine clock keeps every beat across a message-thread stall (the s-rta-0927 beat-clock fix). So the first tick after a stall of ~0.3 s or more calls `advanceTo(pos)` with pos already past x1.
3. `advanceTo` touches the lane (`:157`), finds `pos >= g.x1` (`:162`) and releases with no `set()`. The gesture's value is never written, and the knob keeps its restored value until the lane's next gesture, at beat 13.18 here.

Discrete points are not affected: the discrete loop fires every point up to pos.

**loadpost1 (s-rta-0927) matches the signature:** Probe Routine position froze at 1.07 for 0.34 s, then jumped to 1.79 across [~1.2, ~1.7]; L0 opacity stayed 1.0 and `yielded` was already 1 (`anomalies/loadpost1-stack.json` in the s-rta-0927 evidence). INFERRED for that run: it had no gestureSkipped log. The 0.34 s stall's own source is unknown. In my runs pure CPU load never stalled the message thread by more than 65 ms (the app's main thread outranks default-QoS busy loops); loadpost1 ran with compilers (memory/I/O pressure).

## 6. Fix options (none implemented)

### Cause 1 -- thumbnail decodes on every deck refresh

| option | what moves where | expected restore hold after (this fixture / 4K / many) | risks vs the rulings and the code |
|---|---|---|---|
| **A. cache image thumbnails on the Clip** (recommended) | Set `Clip::thumbnail` (`Clip.h:232`) for image clips once, at import / composition load / media reconnect. It is already done for video and sequences (`MainComponent.cpp:2929`, `:2939`, `:4882`, `:4951`). `ClipCell.cpp:400` and `LayerStrip.cpp:950` then take the existing cached branch | ~0.3-0.5 ms everywhere (INFERRED from the no-image fixture: `startNow` 0.26-0.51 ms, refresh 0.11-0.28 ms; plus a 90x72 rescale per LayerStrip) | No ruling touched: model writes stay ON the boundary and the glide is unchanged. The one-time decode moves to load, where N images = N decodes; it can be async (option D). A stale thumbnail if the file changes on disk: the reconnect path must re-make it. Small memory cost (90x72 per clip) |
| B. skip re-decode when nothing changed | ClipCell/LayerStrip keep the last thumbnail when clip id + file + mtime are unchanged | The ClipCell decodes vanish. The LayerStrip whose active clip CHANGES still decodes once: ~9 ms card / ~30 ms 4K (INFERRED) | Keying on `Clip*` alone is unsafe (vector reallocation); key on id + path. The smallest patch, but it leaves a per-trigger decode |
| C. one refresh per restore | Defer `deckView_->refresh()` during `firePreambleDiscrete`, or make DeckView refresh coalesce via an AsyncUpdater | card ~18-20 ms, 4K ~57-64 ms, many ~51-53 ms (1 refresh; INFERRED from the per-dispatch rows) | The UI follows the model up to one message-loop pass later; the model is unchanged. Not enough alone; good with A |
| D. decode off the message thread | A thread-pool thumbnail cache, swapped in via `callAsync` | ~0.3-0.5 ms (the hold); the thumbnail appears a few ms later | Never touch a Component from the worker; cancellation on clip delete/reload. More code than A; pairs with A for load-time cost |

All four also remove the same 9-33 ms from every human, REST, MIDI, OSC and replayed clip trigger or flag toggle on the active deck.

### Cause 2 -- a stall jumps over a whole gesture

| option | change | expected | risks |
|---|---|---|---|
| **A. land the skipped gesture's end value** (recommended) | In `Player::advanceTo`, when an accepted touch finds `pos >= g.x1`, call `sink.set(key, g.curve.eval(g.x1))` before the release | The 450 ms-stall arm shows 0.5 after the jump (the value the recording held at x1). Take replay (`RecorderHost` uses the same Player) is fixed too | D9 arbitration still applies (`SlotSink::set` owner check). A human hand still wins: a refused touch means displaced, so no set. Several gestures skipped in one call land in order, and the last one wins. Needs a new ctest next to the D3/D5 touch/release-pair tests. No Boris ruling touched: it is a knob the routine moves |
| B. fewer long stalls | Fix cause 1 (a restore holds 54-178 ms on these decks, and 18 decodes x 30 ms would be about 0.5 s) | Lowers the odds; does not remove load stalls | -- |
| (rejected) clamp the clock's per-tick advance | -- | -- | Undoes the s-rta-0927 beat clock: events would land late again |

## 7. found_not_fixed
1. **Idle background** (unattributed): the message thread is busy 17-25 ms about every 70 ms with no routine running. The instrumented paint scopes inside it sum to <= 1 ms, so the rest sits outside every instrumented scope. It sets a ~20-28 ms floor under any ±250 ms hitch measurement. Out of scope.
2. **Early-in-bar poll vs the Bar edge (low confidence):** the driver fires once `/api/bpm` reads beatInBar 0 and beatPhase < 0.5, the same rule as probe-routines' `edge` helper. Even so, 17 of 81 fires started less than 100 ms later, i.e. the engine's `totalBarCount` edge came right after the poll. Cause not isolated (INFERRED: `/api/bpm`'s beatInBar and the tracker's bar counter are published from different reads). It can shorten or remove the Ease glide window in probe rows 5g/11j.
3. **Scale (INFERRED from the linear fits above):** a restore with k discrete entries on a deck with c image cells and l image layers costs about k x (c + l) x (per-image decode). For example, 3 x (16 + 2) x 30 ms = 1.6 s for 16 4K cells. At that size cause 1 can itself trigger cause 2 for a gesture recorded in the routine's first beats.

### FILES CHANGED
- none committed. Temporary: 8 source files + `src/diag/DiagTrace.h` (saved in `restore/instr.diff`; the tracked part in `restore/instr-tracked.diff`). I reversed them with `git apply -R` of my own diff, not a tree-wide checkout, which the stash-guard hook blocks, and deleted the new header. `git status`: only `?? build-lane/`. `build-lane` was rebuilt from the clean tree (09:08:53-09:09:07); the binary has 0 instrumentation strings (`strings | grep` count 0).
- `.harmony/.reports/s-rta-0928/restore-diag.md` -- this report.

### TESTS
- No product tests (diagnosis). The live A/B runs are in section 2. Every launch's `outwins` read `audio-dna windows 0, Output-named 0`.

### SLIM CHECK
nothing to cut -- no product diff.

### ISSUES
- The stash-guard hook blocked `git checkout -- .` in my own worktree. I reversed with `git apply -R` of my own recorded diff instead; same end state.
- The first baseline build overlapped my first RoutineEngine edit. It was superseded by the instrumented rebuild; every measured binary is the instrumented one (the tickGap/gestureSkipped build was used for stallA/loadA/loadB/manyA; cardA/cardB/sourceA/bigA ran the build before that marker was added. Both builds have identical hold scopes).
- The macOS load average reached 86-101 with only 14-30 busy loops; these are the printed values.

### SKILL_PROPOSALS
- none (the heartbeat + scope method is recorded in `instr.diff`; a notebook note was not added because this lane must leave the tree clean).

### RISKS
- High: the hold is linear in image cells x image size x restore entries, so a real VJ deck (many 4K images) will freeze the UI for seconds at every restoring start and loop return, and clip triggers pay the same per refresh. Mitigation: option A (+C).
- Medium: cause 2 silently drops recorded single writes after any stall of 0.26 s or more, in both routines and take replay. Mitigation: option A.

### METRICS
- Self-check: instrumented builds `cmake --build build-lane --target AudioDNA -j3` exit 0 (x3), clean rebuild exit 0; 9 live launches (cardA, cardB, sourceA, bigA, manyA, stallA, loadA, loadB, plus 1 smoke).
- Tool calls: ~55. Files read: ~20.

### KNOWLEDGE CONTEXT
- Tools used: grep. Impact authority: none (read-only diagnosis). Risk level: NORMAL.

### PACKET QUALITY
- Clarity: CLEAR.
- Missing context: The main build has `AUDIODNA_BUILD_SYPHON=ON` (from `build/CMakeCache.txt`), which I matched. Catch2's FetchContent is in `tests/CMakeLists.txt`, not the root file. The stall hook `/api/debug/stall_message_thread` exists in TEST_SERVER builds on 7070, which made the deterministic reproduction possible.
- Unused context: the `.venv` symlink step. I ran the driver with the main `.venv` python directly and never ran probe-routines in the worktree, so no symlink was created.
- Self-assembly: LEGACY (no DEPARTMENT field).
- Self-brief files: `.harmony/.reports/s-rta-0927/routines-timing.md` (useful: the symptom, the staleness method, loadpost1 evidence); `.harmony/probe-routines.sh` + `.harmony/probe-routines.json` (useful: the scenario and fixture I reused).

### STATUS
DONE. Both causes are attributed with numbers (>= 99.6% of every restoring hold). The stack anomaly is reproduced 5/5 with a control 0/5. Natural CPU-load reproduction was attempted in 20 runs with no hit. The rig is clean: lock released, no app running, 0 Output windows, busy loops killed, worktree clean except `build-lane` (kept, rebuilt clean).

### NEXT ACTION
A fix lane for cause 1 (option A, optionally with C). The live check: rerun `drive.py` on card/big/many with the instrumentation re-applied, and expect restoring tick holds below 1 ms. A fix lane for cause 2 (option A) plus a ctest where one `advanceTo` spans a whole gesture and must `set` its end value. The live check is `stackstall450`, which should show 0/5 anomaly.

Evidence root: `/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/cfd08720-0a05-463c-84b1-89ae4354a3fb/scratchpad/restore/` (`runs/<tag>/{app-err.log,drive.out,drive-log.json,analysis.txt,analysis.json,fixture.json,stack-*.json}`, `summary-hold.txt`, `summary-calltree.txt`, `summary-stack.txt`, `instr.diff`, `instr-tracked.diff`, `drive.py`, `analyze.py`, `run1.sh`, `runload.sh`, `instr_engine.py`, `instr_mc.py`, `big4k.jpg`, `takes/`, `build-lane-w4-cfg.log`).
