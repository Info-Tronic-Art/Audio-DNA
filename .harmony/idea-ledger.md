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
