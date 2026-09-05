<!-- Secondary→Primary idea-ledger (foreign-repo lane). Verbatim IDEA records (spec §4.1). The primary PULLS these at boot via config/repos.yml scan into memory/secondary-ideas-inbox.md. Append-only; never compress `raw`. -->
--- IDEA ---
id:          idea-2026-07-28-RealTimeAudio-1785289328541427496
raw:         I would like cmd x to clear a clip as well
context:     Undo v1 manual e2e sitting 2026-07-28 — asked how to delete clips from a cell; wants Cmd+X (cut) as a clip-clear gesture. Note: Clip menu lists Cut/Copy/Paste — wiring/shortcut state unverified.
why:         
intent:      
target:      
constraints: 
related:     
priority:    HIGH
repo:        RealTimeAudio     session:      date: 2026-07-28
status:      NEW
status-changed: 2026-07-28
status-note:
artifact:
history:     NEW(2026-07-28)
--- /IDEA ---

--- IDEA ---
id:          idea-2026-07-28-RealTimeAudio-17852893285415730694
raw:         I want them all loaded by default
context:     Same sitting — MilkDrop browser showed no presets (preset dir unset; libprojectM absent). Wants bundled resources/projectm_presets loaded by default. Natural bundle with the MilkDropBrowser empty-state null-deref crash fix (.ips 2026-07-28-190701).
why:         
intent:      
target:      
constraints: 
related:     
priority:    HIGH
repo:        RealTimeAudio     session:      date: 2026-07-28
status:      NEW
status-changed: 2026-07-28
status-note:
artifact:
history:     NEW(2026-07-28)
--- /IDEA ---

--- IDEA ---
id:          idea-2026-08-04-RealTimeAudio-17858604595945222831
raw:         If I drop multi images it would be great to have a toggle to select between drop on cell or drop on multi cells. Can this be visible as a selection right after drop?
context:     Boris, session 2026-08-04c, Audio-DNA. PARTIALLY addressed, NOT fully delivered. SHIPPED in 7d3a203: sequence threshold raised to 3+ (2 images now spread across 2 cells, which removes the surprise that prompted this) + a 'SEQ N' badge so sequence cells stop being visually IDENTICAL to video cells (they shared one paint branch — that was the real root cause). NOT SHIPPED: the 'visible as a selection right after drop' escape hatch.
why:         Boris dropped 2 images, they merged into one animated sequence clip, and he did not recognise a feature his own app ships and documents. He wanted both to know it happened AND to be able to opt out per-drop.
intent:      Give the user a per-drop choice between image-sequence and spread-across-cells, discoverable at the moment of the drop.
target:      RECOMMENDED HOME: a persistent 'Spread Sequence to Cells' command in the existing Clip menu (MenuBarModel.cpp:9) or as a TextButton in ClipInspector (already a panel of exactly such actions). No timer, no snapshots, correct undo depth by construction, works ten minutes after the drop, and greys out when inapplicable — which itself teaches.
constraints: An architect designed a transient post-drop chooser pill (~450-500 lines). TWO independently-dispatched blind critics BOTH returned UNSOUND/FAIL and converged against it: cells are 90px with kCellGap=0 so a legible pill is WIDER than its anchor and would occlude the trigger hit box; its dismiss mechanism (UndoManager::onHistoryChanged) is structurally BLIND to autopilot advances and non-user deck switches — the two things most likely to happen mid-set, neither of which creates a command; UndoManager::onHistoryChanged is a single std::function already assigned at MainComponent.cpp:1544 so hooking it would silently break Edit-menu undo text; and a 5s timer makes the escape hatch a race in exactly the window where the clip is most likely to have started playing.
related:     .harmony/decisions-2026-08-04c.md ; commit 7d3a203 ; .harmony/gesture-replay-results.md
priority:    medium — the surprise is already fixed by the badge + threshold; this is the remaining convenience half
repo:        RealTimeAudio     session:      date: 2026-08-04
status:      NEW
status-changed: 2026-08-04
status-note:
artifact:
history:     NEW(2026-08-04)
--- /IDEA ---

## 2026-09-05 — s-rta-0904 (secondary): parked findings from the L2 review

Ruled OUT OF SCOPE for the L2 lane deliberately, not dropped. Both are real and both were
surfaced by the independent reviewer, not by the builder.

- **Solo button has no tooltip** (`src/ui/LayerStrip.cpp:328`). Not introduced by L2, but L2 is
  what first makes the distinction behaviourally real: solo NARROWS the set of layers that
  render, it does not un-hide a hidden layer or un-bypass a bypassed one. A user's likely mental
  model is that solo overrides everything. One line of tooltip while the behaviour is fresh.
  Size: trivial. Belongs with L4 (honesty batch) or any UI pass.

- **No compositor-level regression test for solo.** `tests/test_compositor.cpp`'s existing "Deck
  layer compositing data model" test only exercises the `Layer` struct's `visible`/`bypassed`
  fields directly and never calls `compositeDeck`. That is consistent with this repo's existing
  pattern (no GL-context test harness), which is exactly why it is worth recording rather than
  silently accepting: the solo skip is now live in three loops with **zero** automated coverage,
  and its only proof is a human looking at pixels. If a GL-context test harness is ever built,
  solo is a good first customer.

- **`compositePersistentLayers` was a plan gap, not a builder miss.** The Fable-authored plan said
  "both layer loops"; there are in fact three sites in the same read class. Worth remembering the
  next time a plan states a count — the plan's count is a claim like any other. The lane fixed it.
## 2026-09-05 — s-rta-0904: L1 review follow-up (OUT OF SCOPE, logged not fixed)

- **Pre-existing, INFO severity, not introduced by L1.** `openVideoForClip` /
  `openImageSequenceForClip` (`src/render/Renderer.cpp:933-954`) assign into the media maps
  directly. That assignment can DESTROY a live VideoPlayer/ImageSequence on the MESSAGE thread,
  outside the retire-list path L1 just built — specifically when reconnect-on-replace fires for a
  clip id whose media is already open. Same hazard class L1 exists to fix (GL resource destroyed
  off the GL thread), different entry point. Found by the independent reviewer while adjudicating
  trap (b). Deliberately excluded from L1 to keep the lane bounded; fixing it while 'already in
  the file' is how a scoped lane becomes an unscoped one. Worth its own small lane, and it should
  reuse L1's retire list rather than inventing a second mechanism.

## 2026-09-05 — s166: the Global Effects lane shipped WITHOUT a behavioral gate, and the reason is structural

- **The feature is source-reviewed SHIP and has no way to be exercised headlessly.** `694f8f3`
  makes `Composition::globalEffects` composite for the first time. The independent reviewer
  traced the temp-Clip lifetime, the `0xFFFFFFFF` sentinel against all three per-layer caches
  (bounded insert, no per-frame allocation, nothing enumerates layer ids), and the pipeline
  order against `updateFeedbackBuffer`'s actual read — verdict SHIP. But **no REST endpoint
  reaches composition-level effects.** The full endpoint list is `bpm, composition, effects,
  features, health, inject_features, load_image, load_source, render_frame, reset,
  set_effect_chain, set_effect, set_layer_opacity, set_param, set_syphon, snapshot, sources,
  state, status, switch_deck, syphon, trigger_clip, trigger_column` — `set_effect_chain` and
  `set_effect` address the v1 `effectChain_`, not `composition_->globalEffects`. And no ctest
  target links `CompositorEngine.cpp` or `Renderer.cpp` at all (headless GL is unavailable in
  this repo's test rig), so there is no unit surface either.
- **So the honest status is: source-verified, not behaviour-verified.** It was NOT gated in the
  running app and must not be described as if it were. The falsifier a human can run: add an
  effect to the Composition Inspector's Global Effects stack and confirm the output visibly
  changes, then bypass it and confirm the change reverses.
- **The small lane that fixes this class permanently:** add a test-server/API endpoint that adds,
  reorders and bypasses an entry in `composition_->globalEffects`, addressed the way
  `ApiServer`'s `set_param` addresses effects. Then `render_frame` at two states gives a real
  headless oracle for every future composition-tier render change — this lane, the four dead
  render fields, and the master-effects consolidation the s166 architecture calls L7. **Cheap,
  and it converts a whole tier of the app from ungateable to gateable.** Recommend doing it
  before, not after, the connection engine lands.

## 2026-09-05 — s166: FIELD EVIDENCE — every tempo-locked oscillator FREEZES when no beat is detected

- **Observed on the live app, not inferred.** With `beatPhase` held static and audio injected,
  both registry oscillators (Mod 1, Mod 2) sat perfectly still across 2 s of wall time. Sweeping
  injected `beatPhase` 0.0 → 0.25 → 0.5 → 0.75 moved them exactly as their shapes predict —
  Mod 1 (sine) 0.5000 → 1.0000 → 0.5000 → 0.0000; Mod 2 (ramp) 0.0000 → 0.1250 → 0.2500 →
  0.3750. So they are healthy and beat-phase-driven; they freeze because `beatPhase` freezes.
- **Why this matters for a live set:** silence between tracks, a quiet intro, or a failed beat
  lock stops EVERY tempo-locked modulation dead, mid-performance — not gracefully, just frozen
  at whatever phase it held. This is the app's core promise ("audio controls the video") failing
  in exactly the moment a VJ is most exposed.
- **This is the architecture pass's open question D3c, now with evidence:** should beat-locked
  sources freeze when no beat is detected, or keep running from the last/tapped BPM? The design
  recommends keeping them running from the tapped/last BPM, and the app already has a manual BPM
  mode and tap tempo to run from. This observation supports that recommendation strongly.
  **Boris's call — it is a feel question about his instrument, not a technical one.**
- Method note worth keeping: the first gate run reported "oscillators frozen — the tick is not
  running" and would have been read as a regression in the lane that had just landed. A single
  discriminating probe (sweep the phase instead of holding it) separated "the clock stopped"
  from "the clock has no input", in about two minutes. **An anomaly attributed without a
  discriminating test is a false lead with a commit message attached.**
- Rig note: `/api/signals` (test server, port 8080, `--test-mode`) reports every signal's live
  cached value, and `/api/inject_features` accepts `beatPhase`, `rms` and `bandEnergies`.
  Together they are a real headless oracle for anything signal-driven — the first one this repo
  has had for the modulation layer. Use it instead of asking for a human.

## 2026-09-05 — s166: BLOCKING PREREQUISITE for the connection engine — fence the bypass toggle FIRST

- **Independently found by the L0 builder and re-verified by the L0 reviewer, from opposite
  directions.** `EffectStackView.cpp`'s bypass button handler does `slot.bypassed =
  !slot.bypassed;` directly on the live `EffectSlot` with **no `runFenced(...)` wrapper**, while
  the erase and push_back handlers in the same file DO route through the fence. Confirmed by
  grep: `runFenced` appears around the erase and add sites and NOT around the bypass site.
- **Today it is harmless and that is exactly why it is dangerous.** A `bool` flip racing a
  GL-thread read is a torn write of a single byte — no heap object moves, nothing corrupts, so
  nothing has ever gone wrong and the omission reads as an accepted convention (POD flips
  tolerated, reallocating ops fenced).
- **It becomes heap corruption the moment `EffectSlot` grows a connection struct** — a
  `std::string` signal name and an envelope vector, which is precisely what the s166
  architecture's Lane 2 adds. Writing a string or vector while the GL thread reads it hands the
  GL thread a torn pointer and length. The same code pattern that is benign today becomes a
  crash-or-worse then, and it will not announce itself: it will look like an intermittent,
  unreproducible graphics glitch under load.
- **So: fence the bypass toggle BEFORE Lane 2 adds any heap-allocated field to `EffectSlot`.**
  Small lane, one call site, follows a convention already established two functions away in the
  same file. Doing it after Lane 2 means shipping a window in which the bug is live.
- Related and already closed: lane L0 (`f53a8f1`) removed the two per-frame GL-thread deep copies
  of these same vectors. The reviewer separately verified that iterating them LIVE by reference —
  a longer exposure window than the old copy — is safe today, because all three effect-vector
  scopes (clip, layer, global) already fence reallocating edits through the same
  `makeDeckFence()` hook propagated by `InspectorPanel::setEffectFenceHook`. Reallocation is
  covered; the bypass FLIP is the hole.

## 2026-09-05 — s166: the composition tier now HAS an oracle, and the first thing it proved was this morning's ungated lane

- **`694f8f3` (Global Effects compositing) is now BEHAVIOURALLY VERIFIED.** With a procedural
  source loaded, adding `Invert` to the composition's Global Effects stack via
  `POST http://[::1]:8080/api/add_global_effect` changed the rendered frame, and removing it
  restored the baseline byte-for-byte (identical md5). That lane shipped this morning
  source-reviewed and un-exercised because no surface could reach it; the oracle built this
  afternoon closed its own gap the same session.
- **The GL fence held under real contention.** 25 back-to-back add/remove cycles against a live
  render loop, all 25 adds returning `ok:true`, no crash, no assertion, health still answering,
  graceful quit. That was the builder's own stated open concern and it is now closed empirically.

### RIG FACTS — the two-server layout, which cost me three probe runs to establish
- **There are TWO HTTP servers and they serve DIFFERENT routes.** `TestServer` answers on
  **`http://[::1]:8080`** (IPv6 loopback) and owns `health`, `signals`, `inject_features`, and
  all the new composition-tier routes. `ApiServer` answers on **`http://127.0.0.1:7070`** (IPv4 —
  it does NOT answer on `[::1]`) and owns `effects`, `sources`, `load_source`, `render_frame`.
  Hitting the wrong one returns 404 (right host, wrong server) or a bare connection failure
  (wrong address family), and neither looks like "you used the wrong port".
- A gate that drives this app therefore needs BOTH base URLs. Write them down rather than
  rediscovering them.

### TWO ANOMALIES OBSERVED, both filed for follow-up
- **`load_source` returns `ok:false` and yet works.** `POST /api/load_source {"name":"Gravity
  Well"}` reported failure, and the very next rendered frame had changed. An endpoint that
  reports failure while having an effect is exactly the lying oracle this lane exists to
  prevent — pre-existing production endpoint, not this lane's doing. Under review.
- **A global effect can be a silent no-op at default parameters.** `Kaleidoscope` added cleanly
  and changed nothing on screen; `Invert` changed it immediately. Any future gate written
  against this oracle must probe with an effect that is non-neutral at defaults, or it will
  report "the tier is broken" when the tier is fine.

### METHOD NOTE — my own gate was wrong before the app was
The first run of this gate reported 7 failures. Five were bugs in the GATE, not the app:
pretty-printed JSON defeated `grep '"ok":false'` (the real text is `"ok": false`), float
formatting defeated a `grep '0.42'` against `0.419999986886978`, and an empty effect-name
variable made `grep "$EFF"` match every line — which produced a false PASS on the readback check
AND a stress test where all 25 "adds" were silently rejected, so it stressed nothing while
reporting success. **Assert on parsed JSON, never on the text of a JSON response, and never
interpolate a possibly-empty variable into a grep pattern.** A gate that cannot fail is not a
gate, and this one could neither fail correctly nor pass correctly.

## 2026-09-05 — s166 RETRACTION: `render_frame` is NOT a reliable pixel oracle, and my "PROVEN" claim above is withdrawn

**Correcting my own entry earlier in this file.** I wrote that `694f8f3` (Global Effects
compositing) was BEHAVIOURALLY VERIFIED because adding `Invert` changed the rendered frame and
removing it restored the baseline. **That claim does not hold and I am withdrawing it.**

**What I found on a third run.** `POST /api/render_frame` returns a BLANK image most of the
time. Measured directly:
- With nothing loaded: `a49e72c11f5655dbdadf61256a88c69d`.
- After `load_source "Gravity Well"`, one frame came back as `8323700d0a510a251f57b54cc5ac8a97`
  (a real render) — and the very next consecutive frame, same source, no changes in between,
  came back as `a49e72c1...` again, i.e. the blank hash.
So the endpoint alternates between a genuine capture and a blank one. **That means my earlier
"the frame changed when I added Invert" is fully explained by the flicker** — I sampled a
non-blank frame at that moment and a blank one before it. The effect may well work; my evidence
does not show it. **A test whose baseline oscillates between two values cannot establish
causation, and I treated a coincidence as a proof.**

**What IS still verified** (model layer, unaffected by the render flakiness, and each one
observed directly): all 7 endpoints round-trip; `add_global_effect` returns a real slot index
and the readback lists the effect; `remove_global_effect` empties the stack; empty bodies,
unknown effect names and out-of-range clips are all REJECTED rather than silently accepted;
25 consecutive add/remove cycles against a live render loop with 25/25 genuine successes, no
crash, no assertion, graceful quit; and after the review fix, the detached-context guard does
NOT over-fire on an attached context (real index 0, not the `-1` sentinel).

**THE BLOCKING FOLLOW-UP, and it now outranks the rest of this arc's tooling work:**
find out why `render_frame` returns a blank image intermittently. Until that is fixed there is
NO pixel oracle for this app, and every composition-tier and connection-engine change will keep
shipping on source review alone — which is the exact hole L8 was built to close. Candidate
causes worth checking first: the capture races the draw and grabs an unpainted buffer; the
offline render path runs on a context that is not the one compositing the deck; or it captures
before `load_source` has actually taken effect (note `load_source` also returns `ok:false` while
apparently working — see the anomaly above; the two may share a root cause).
**Recommended shape of the fix:** make `render_frame` synchronous against a real completed
composite — render, wait for the frame it rendered, then write — and have it return a
distinguishing marker (e.g. frame counter or a non-blank assertion) so a caller can tell a
capture from a miss. An oracle that silently returns blank is worse than no oracle, which is the
lesson this whole session keeps re-teaching.
