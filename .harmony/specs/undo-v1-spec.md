# Undo/Redo v1 — Implementation Plan (Audio-DNA / RealTimeAudio)

<!-- Authored by Plan agent (plan-undo), 2026-07-17. Persisted verbatim by Harmony.
     Status: SPEC — Wave 2 build item (Boris-ratified: implement real undo, structural
     edits only). Lines are pre-Wave-0-cleanup; symbols authoritative if drifted. -->

All file:line claims below were verified by reading source in `/Users/boriskarpman/projects/RealTimeAudio` this session. Lines are pre-Wave-0-cleanup; symbols are authoritative if lines drift.

## 1. Recommendation: keep the custom scaffold, use value-copy snapshot commands — NOT toVar, NOT juce::UndoManager

**Keep the existing `UndoManager`/`Command` scaffold** (`src/core/UndoManager.h`, `src/core/Command.h`). It is small, correct, already has merge support (`Command::canMergeWith`, Command.h:23), a history cap (`UndoManager.h:44`, kMaxHistory=500), a change callback for menu updates (`UndoManager.h:38`, `onHistoryChanged`), and is already unit-tested (`tests/test_composition.cpp:180-249`, Catch2).

**Against juce::UndoManager:** its main leverage is automatic undo via `juce::ValueTree`, and this model is plain C++ structs (`Clip`, `Layer`, `Deck`, `Composition` — all value types, verified). Without ValueTree you'd write the same UndoableAction subclasses you'd write Commands for, plus a migration, for zero gain. The one feature it adds (named transactions) is a ~40-line `CompositeCommand` on the existing scaffold.

**Snapshot style: direct C++ value copies, not toVar.** Every model type is cheaply copyable by value (verified: `Clip` is a flat struct of scalars/strings/vectors; `juce::Image thumbnail` is refcounted copy-on-write; `Layer::clips` is `vector<optional<Clip>>`). A command stores `std::optional<Clip> before, after` plus coordinates; undo is plain assignment. This is strictly better than toVar snapshots because **`Clip::toVar` is verified LOSSY today** (`src/model/Clip.cpp:3-80` omits `presetPlaylist` and all playlist fields, the entire Transform block, `clipOpacity`, `clipWidth/Height`, `blendOverride`, `alphaType`, and channel R/G/B/A flags — a toVar-based undo would silently destroy MilkDrop playlists and transforms). `Composition::toVar` (Composition.h:162-198) is similarly lossy at its level.

**Dependency note (toVar-fidelity wave):** by using value copies, v1 undo has NO correctness dependency on the concurrent toVar round-trip-fidelity wave. The only place that wave would matter is if tests used "serialize → compare" equality; the test strategy below uses hand-written `operator==` instead, so the dependency disappears entirely. If a future version wants toVar-based snapshots (e.g., undo persistence across sessions), that DOES require the fidelity wave landed first — state this explicitly in any follow-up spec.

**Key architectural rule — commands store coordinates, never pointers.** Commands address their target as `(deckIndex, layerIndex, column)` or an effect-stack scope descriptor and re-resolve through `Composition&` at execute/undo time. Raw `Clip*`/`Layer*` pointers dangle: `Layer::ensureColumns` and `deck->layers` mutations reallocate vectors (`src/model/Deck.h:80-104`), and inspectors already hold raw pointers (`inspectClip(clip)` at `src/MainComponent.cpp:571`).

**Threading model (verified):** the GL render thread runs `renderOpenGL()` continuously (`src/render/Renderer.cpp:23-30` — `setContinuousRepainting(true)`, `setComponentPaintingEnabled(false)`) and reads the deck model lock-free through `std::atomic<Deck*> activeDeck_` (`src/render/Renderer.h:85-86, 269`); `CompositorEngine::compositeDeck` iterates `deck.layers` directly (`src/render/CompositorEngine.cpp:648, 678, 852`). There is NO mutex on the model. Additionally, **autopilot triggers clips from the GL thread** (`Renderer.cpp:212-215` → `src/model/Autopilot.cpp:283, 305, 367`). Consequences:

- All commands execute/undo on the message thread only (add `jassert(MessageManager::existsAndIsCurrentThread())` in `UndoManager::perform/undo/redo`). REST already marshals via `MessageManager::callAsync` (`src/api/ApiServer.cpp`, `handleTriggerClip`), so REST paths are safe.
- Undo restores are the same kind of unsynchronized write the app already performs everywhere (`deck->setClip`; `effects_->erase` at `src/ui/EffectStackView.cpp:280`; `layers.erase` at `MainComponent.cpp:2899`). v1 does not make the race worse in kind, but whole-`Layer`/`Deck` restores widen the torn-read window. Mitigation: an optional `withDeckDetached()` guard for structure-changing commands — store `nullptr` into `activeDeck_`, call `glContext_.executeOnGLThread(noop, /*block*/true)` to fence out any in-flight frame, mutate, restore the pointer. Deadlock risk is low because component painting is disabled so the GL thread never takes the message lock (inferred from JUCE semantics — verify in build step 1). Recommended for layer-add/remove/move and deck-add/remove; plain cell assignments keep today's risk profile.
- Command wrapping happens ONLY at user-initiated entry points (`handleClipTrigger`, `handleColumnTrigger`, drop lambdas, menu handler, button callbacks) — never inside `Layer::triggerClip`, which the GL thread also calls.

## 2. Command inventory (one row per in-scope op)

| # | Operation | Mutation site (symbol, file:line) | Command | Snapshot payload |
|---|---|---|---|---|
| 1 | Clip trigger (cell click) | `MainComponent::handleClipTrigger`, MainComponent.cpp:2461-2469 | `TriggerClipCmd` | per-layer runtime: `activeClipColumn`, `previousClipColumn`, `crossfadeProgress`, `pendingTriggerColumn`, target clip `playing` |
| 2 | Column trigger | `MainComponent::handleColumnTrigger`, MainComponent.cpp:2562-2567 | `TriggerColumnCmd` (composite of #1 across layers) | same, all non-ignoring layers |
| 3 | Clip move/swap (drag name bar) | `onClipMoved` lambda, MainComponent.cpp:753-789 | `SwapClipsCmd` | two cells' `optional<Clip>` before/after + `numColumns` |
| 4 | Clip clear/delete (menu, multi-select) | `kClipClear`, MainComponent.cpp:3022-3034 | `CompositeCommand` of `SetClipCmd` | per-cell `optional<Clip>` before / `Clip{}` after |
| 5 | File drop (image/video) | `MainComponent::handleFileDrop`, MainComponent.cpp:2617-2666 | `SetClipCmd` | cell before/after (after includes id, thumbnail) |
| 6 | Multi-image drop (sequence) | `MainComponent::handleMultiFileDrop`, MainComponent.cpp:2668-2705 | `SetClipCmd` | as above (sequenceFiles list in payload) |
| 7 | Multi-video drop (sequential cells) | `onMultiVideoDropped` lambda, MainComponent.cpp:607-626 | `CompositeCommand` of `SetClipCmd` + `numColumns` before/after | N cells |
| 8 | FX drop on cell (new FX clip or append to chain) | `onEffectDropped` lambda, MainComponent.cpp:627-702 | `SetClipCmd` (covers both branches) | cell before/after |
| 9 | Source drop (multi) | `onSourceDropped` lambda, MainComponent.cpp:703-752 | `CompositeCommand` of `SetClipCmd` | N cells |
| 10 | MilkDrop single / playlist drop | `onMilkDropDropped` MainComponent.cpp:791-836, `onMilkDropPlaylistDropped` 839-889 | `SetClipCmd` | cell before/after (presetPlaylist survives — value copy, not toVar) |
| 11 | Replace content (keep FX) | `kClipReplaceContent`, MainComponent.cpp:3036-3092 (`Clip::replaceContent`, Clip.h:170) | `SetClipCmd` | cell before/after |
| 12 | Clip content-lock toggle | `kClipLockContent`, MainComponent.cpp:3094-3112 | `ToggleClipLockCmd` (or SetClipCmd) | bool per cell |
| 13 | Layer clear (X button) | `LayerStrip::clearBtn_` → `onClearClip` → direct mutation in `DeckView::rebuildGrid` lambda, DeckView.cpp:105-114 (`clearActiveClip`) | `ClearActiveClipCmd` — reroute callback through MainComponent | layer runtime fields (as #1) |
| 14 | Layer bypass toggle | `LayerStrip` ctor, LayerStrip.cpp:333-338 (mutates `layer_->bypassed` directly) | `ToggleLayerFlagCmd` — needs undo hook into LayerStrip | bool + layer index |
| 15 | Layer solo toggle | LayerStrip.cpp:339-344 | `ToggleLayerFlagCmd` | bool + layer index |
| 16 | Layer new/insert | `kLayerNew/InsertAbove/InsertBelow`, MainComponent.cpp:2921-2929 (`Deck::addLayer`, Deck.h:50) | `AddLayerCmd` | layer index (undo = erase); GL fence |
| 17 | Layer remove | `kLayerRemove`, MainComponent.cpp:2930-2939 | `RemoveLayerCmd` | full `Layer` copy; GL fence |
| 18 | Layer move up/down | `kLayerMoveUp/Down`, MainComponent.cpp:2971-2998 (`Deck::moveLayer`, Deck.h:71) | `MoveLayerCmd` | from/to indices; GL fence |
| 19 | Layer clear-clips menu | `kLayerClearClips`, MainComponent.cpp:2940-2952 — NOTE: body identical to deck clear, clears ALL layers (looks like a bug; see risks) | `ClearDeckClipsCmd` | whole-`Deck` copy |
| 20 | Layer fold toggle | `kLayerFold`, MainComponent.cpp:2954-2970 | `ToggleLayerFlagCmd` | bool |
| 21 | Deck new | `kDeckNew`, MainComponent.cpp:2887-2895 | `AddDeckCmd` | deck index + prior `activeDeckIndex`; GL fence + `setActiveDeck` in apply hook |
| 22 | Deck remove | `kDeckRemove`, MainComponent.cpp:2896-2905 | `RemoveDeckCmd` | full `Deck` copy + index; GL fence |
| 23 | Deck clear clips | `kDeckClearClips`, MainComponent.cpp:2906-2918 | `ClearDeckClipsCmd` | whole-`Deck` copy |
| 24 | Deck switch (tab) | `MainComponent::handleDeckSwitch`, MainComponent.cpp:2707-2719 | `SwitchDeckCmd` | old/new `activeDeckIndex`; apply hook calls `renderer.setActiveDeck` |
| 25 | Column add | `kColumnNew/InsertBefore/InsertAfter`, MainComponent.cpp:3001-3009 (`Deck::addColumn`) | `AddColumnCmd` | column count |
| 26 | Column remove (last) | `kColumnRemove`, MainComponent.cpp:3010-3019 (`Deck::removeColumn`) | `RemoveColumnCmd` | per-layer removed `optional<Clip>` at that column |
| 27 | Effect add (drop on stack) | `EffectStackView::itemDropped`, EffectStackView.cpp:409-450 (`effects_->push_back`) | `EffectStackCmd` (whole-vector snapshot) | `vector<EffectSlot>` before/after + stack scope |
| 28 | Effect remove | delete-button lambda, EffectStackView.cpp:278-286 (`effects_->erase`) | `EffectStackCmd` | as above |
| 29 | Effect bypass toggle (optional, cheap) | bypass lambda, EffectStackView.cpp:266-272 | `EffectStackCmd` | as above |

Effect-stack scope addressing: the three hosts are `ClipInspector.cpp:751` (`&clip->effects`), `LayerInspector.cpp:742` (`&layer->layerEffects`), `CompositionInspector.cpp:380` (`&comp->globalEffects`). Verified that `onEffectAdded`/`onEffectRemoved`/`onBypassChanged` are wired NOWHERE — EffectStackView mutates in isolation. It needs a new `performEdit` hook plus a scope descriptor (`Global | Layer(idx) | Clip(layer,col)`) set by each host, so commands re-resolve the vector by coordinates, never via the stored `effects_` pointer.

Out of v1 (per Boris-ratified scope): all sliders (`LayerStrip.cpp:389-461`; effect param `UniversalParamControl::onValueChanged` at `EffectStackView.cpp:337-343`), blend/keying/transition dropdowns (`LayerStrip.cpp:433-475` — discrete, good v1.1 candidates), transport buttons, playhead scrub, mapping values, macros, autopilot-driven triggers (GL thread — must never create commands), the legacy v1 toolbar `saveDeck`/`loadDeck` path. `kCompNew`/`loadPreset` are not undoable → they call `undoManager_.clear()`.

Existing wiring that starts working immediately: Cmd+Z/Cmd+Shift+Z at `MainComponent::keyPressed` (MainComponent.cpp:1639-1646), menu Undo/Redo at `kCompUndo/kCompRedo` (MainComponent.cpp:2753-2758), `UndoManager undoManager_` member at MainComponent.h:205.

## 3. Coalescing / transaction rules

- **One drop = one command.** Drop callbacks fire once per gesture; multi-cell drops (#7, #9), multi-select clear (#4), and column trigger (#2) wrap in a new `CompositeCommand : Command` (executes children in order, undoes in reverse, description from a supplied name). The existing scaffold has no composite — add it.
- **Clip move drag = one `SwapClipsCmd`** (the gesture only commits on `itemDropped`, ClipCell.cpp:412-423 — nothing to coalesce mid-drag).
- **Retrigger of the already-active cell pushes nothing** (`triggerClipImmediate` early-outs into a playhead reset, Layer.h:225-235 — no structural change).
- **Consecutive triggers on the same layer merge** via existing `canMergeWith`/`mergeWith` (keep the original before-state, update after-state) so mashing cells mid-set costs one history slot per layer run, not one per click.
- No merging for toggles (bypass/solo/lock/fold) — each is one tiny command; simplicity wins in v1.

## 4. Stack limits and memory bounds

- Lower `kMaxHistory` from 500 (`UndoManager.h:44`) to **100** for v1. Typical `Clip` snapshot is single-digit KB (strings + param floats; the `juce::Image` thumbnail is refcounted/shared, ~zero marginal cost). Worst cases: image-sequence clips (~100 B per `juce::File` × frames → ~200 KB for a 2000-frame sequence) and whole-`Deck` snapshots for clear/remove. Bound: 100 entries × worst-case ~1 MB ≈ 100 MB pathological, single-digit MB realistic. Acceptable for v1; byte-accounting deferred (risk noted).
- `UndoManager::clear()` on: composition load (`loadPreset`), `kCompNew`, legacy deck load, app shutdown. No cross-session undo persistence in v1.

## 5. UI feedback

- **Menu:** `MenuBarModel.cpp:34-35` currently adds static, always-enabled "Undo"/"Redo". Give `AudioDNAMenuBar` two injected providers (e.g. `std::function<std::pair<juce::String,bool>()> getUndoState/getRedoState`); build items as `"Undo " + description` with the enabled flag, and add shortcut display text for Cmd+Z / Cmd+Shift+Z. Wire `undoManager_.onHistoryChanged → menuBarModel_->menuItemsChanged()` so the native macOS menu rebuilds. Descriptions come from `Command::description()` — e.g. "Drop 'loop.mp4'", "Clear 3 Clips", "Move Layer Up", "Add Effect 'ripple'".
- **Keyboard:** already wired (see §2) — starts working the moment commands exist.
- **Refresh after undo/redo:** one shared `syncAfterModelChange(scope)` helper: `deckView_->rebuildGrid()` (or `refresh()` for runtime-only changes), `inspectorPanel_` re-inspect BY COORDINATES (fixes the dangling-pointer class), and `renderer.setActiveDeck(...)` when deck structure or active index changed.

## 6. Build order

| Step | Work | Size |
|---|---|---|
| 1 | Plumbing: message-thread asserts in UndoManager; `CompositeCommand`; `UndoService` helpers (coordinate resolution, `syncAfterModelChange`, optional `withDeckDetached` GL fence + verify no-deadlock); menu enable/disable + dynamic names + `menuItemsChanged` wiring; stack-clear on load/new; cap→100 | M |
| 2 | `SetClipCmd` + wrap single-cell sites: #5, 6, 8, 10, 11, 12 and clip clear #4 | M |
| 3 | `SwapClipsCmd` (#3) | S |
| 4 | Composites: multi-video #7, multi-source #9, multi-select clear #4, column ops #25-26, deck/layer clear #19, 23 | M |
| 5 | Layer ops: reroute `onClearClip` through MainComponent (#13); bypass/solo hooks in LayerStrip (#14-15); add/remove/move/fold (#16-18, 20) with GL fence | M |
| 6 | Deck ops: #21-24 incl. renderer pointer updates in apply hooks | S-M |
| 7 | Effect stacks: `performEdit` hook + scope descriptor in EffectStackView; `EffectStackCmd` (#27-29) across the three inspector hosts | M |
| 8 | Triggers: `TriggerClipCmd`/`TriggerColumnCmd` with merge rules; REST paths flow through the same handlers automatically (`ApiServer` → `onTriggerClip` → `handleClipTrigger`) | M |
| 9 | Tests (below) + manual e2e checklist | M |

Steps 2-8 each end user-verifiable (perform op → Cmd+Z → visually restored), so they can ship incrementally.

## 7. Test strategy

- **Unit (Catch2, new `tests/test_undo_commands.cpp`, headless):** commands act on a bare `Composition` with UI/renderer hooks injected as no-op std::functions. For each command: execute → undo → deep-equal to initial; execute → undo → redo → deep-equal to post-state. Requires hand-written `operator==` for `Clip`/`EffectSlot`/`Layer`/`Deck` (mechanical; deliberately NOT toVar-based equality, so no dependency on the fidelity wave). Property test: random sequence of N commands, undo all → equals initial; redo all → equals final.
- **Manager tests:** already exist (`test_composition.cpp:180-249`); add merge-behavior and cap-eviction cases.
- **Coordinate-resolution tests:** remove layer → undo → verify commands targeting later layers still resolve (index consistency under linear history).
- **Integration (manual + REST):** scripted checklist — drop/undo each drop type incl. MilkDrop playlist (the toVar-loss case), drag-move/undo, deck switch/undo, effect add/remove/undo in all three inspector scopes, undo while a video plays, undo while autopilot runs (best-effort expectation), menu text/enable states.

## 8. Risk register

1. **GL-thread torn reads (pre-existing, widened by undo).** GL thread reads `Deck`/`Layer`/`Clip` lock-free every frame while the message thread mutates; `layers` vector reallocation during add/remove is the worst case and already exists today (MainComponent.cpp:2926, 2899). Mitigation: GL fence for vector-structure commands; accept field-level races as status quo. Real fix (double-buffered model handoff) is out of v1.
2. **Autopilot vs trigger-undo.** Autopilot mutates `activeClipColumn` from the GL thread (Renderer.cpp:215); undoing a trigger mid-autopilot may be immediately overridden and technically races. Accept as best-effort; document. Never create commands on the GL path.
3. **Inspector dangling pointers (pre-existing class).** Inspectors hold raw `Clip*`; undo restores/reallocations invalidate them. `syncAfterModelChange` re-inspecting by coordinates is load-bearing — build it in step 1, not later.
4. **Renderer resource lifecycle.** Video players/image sequences are keyed by `clip.id` and never closed (verified: no `closeVideoForClip` exists anywhere). Undo/redo of drops therefore reconnects automatically via id — convenient now, but if a cleanup wave later adds player disposal, redo must re-open players; put a guard (`getVideoPlayer(id) || openVideoForClip`) in `SetClipCmd`'s apply hook.
5. **Runtime state inside snapshots.** Full-value `Clip` restores rewind `playheadPosition`/`playing`. Accepted for v1 (arguably correct for structural undo); flagged so it isn't later reported as a bug.
6. **Memory worst case** (huge `sequenceFiles` lists, whole-deck snapshots) — bounded by cap 100, no byte accounting in v1.
7. **`kLayerClearClips` clears the entire deck** — its body is identical to `kDeckClearClips` (MainComponent.cpp:2940-2952 vs 2906-2918), almost certainly a bug. Undo v1 wraps existing behavior faithfully; the fix is a separate decision for Boris, not smuggled into this work.
8. **Wave-0 drift.** A cleanup builder is editing this repo concurrently; lines cited are from today's tree — resolve by symbol if drifted. Wave-0's "menu honesty" group may touch `MenuBarModel.cpp`; coordinate before step 1's menu changes.
9. **GL-fence no-deadlock claim is inferred** from JUCE semantics (`setComponentPaintingEnabled(false)` means the render thread doesn't take the message-manager lock) — must be validated empirically in build step 1 before relying on `withDeckDetached`.

Verification labels: all file:line and threading facts — verified against source this session. GL-fence deadlock-safety — inferred, validate in step 1. Step sizes — estimates.
