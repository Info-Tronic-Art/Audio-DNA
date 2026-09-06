# L3 — Composition Persistence — work packet (RECON)

Recon only. No src/ or tests/ files were modified. All claims below are
labeled VERIFIED (read the code path end to end / grepped with ≥2 patterns)
or INFERRED (plausible, not traced). Line numbers WILL drift — every anchor
below carries the exact text to re-grep it.

## VERDICT: BUILDABLE
The plan's root-cause claim holds at HEAD (139 commits after the plan was
written, verified fresh). The mechanism to fix it (`withDeckDetached` +
`undoManager_.clear()`) exists and is proven correct by the kCompNew
precedent — but kCompNew's own refresh sequence has a live, independently-
reachable gap (dangling `Clip*`/`Layer*` in the inspector, guaranteed to be
hit by a 10Hz timer) that must NOT be copied verbatim into loadComposition.
Recon found one NEW leak class (per-layer GL resources, keyed by
`Layer::id`, never released on a model swap) that the plan did not name and
that is bigger than "L1's mechanism" covers — recommend carving it out (see
SPLIT).

## SIZE: medium-large
Core wiring (Step A below): ~5 files touched in src/ (MainComponent.h/.cpp,
model/Composition.h, model/Deck.h, one new small free function or two), on
the order of 150-220 new/changed lines across ~4 new private methods on
MainComponent (`loadComposition`, `saveComposition`, `saveCompositionAs`,
a shared post-swap-refresh helper) plus edits to 3 existing menu-command
cases and the CompDecksBrowser wiring call site. No Command subclass
signatures change — zero test call-site sweeps needed for that class of
risk (see CALL-SITE ENUMERATION, item 7). tests/test_composition.cpp
currently has zero file-I/O-path tests (only in-memory `toVar`/`fromVar`
round-trips) — a new headless test for `loadFromFile` round-tripping
through an actual `juce::File` would be cheap to add and is recommended,
not required.

## CURRENT BEHAVIOUR (verified) — what happens today, traced end to end

1. **File > Open** (`MainComponent.cpp:4067` `case C::kCompOpen: loadPreset(); break;`)
   and **File > Save / Save As** (`MainComponent.cpp:4069-4073`, both
   `case C::kCompSave:` and `case C::kCompSaveAs:` call `savePreset();` —
   they are IDENTICAL handlers today, no Save-vs-SaveAs distinction exists)
   all route into `MainComponent::loadPreset()` / `savePreset()`
   (`MainComponent.cpp:2266`, `:2295`). Both call `PresetManager::loadPreset`/
   `savePreset` (`MainComponent.cpp:2284`, `:2311`) against
   `previewPanel_.getEffectChain()` and `previewPanel_.getMappingEngine()` —
   the v1 FX-preset/mapping system (`src/ui/PresetManager.h/.cpp`), format
   documented at `PresetManager.h:9-25` as "version 2" preset JSON
   (`{"name","version","effects":[...],"mappings":[...]}`). **This has
   nothing to do with `Composition`.** CONFIRMS the plan's claim exactly —
   Open/Save/SaveAs today read/write FX-chain+mapping state, never touch
   `composition_.decks`.

2. `Composition::saveToFile` (`model/Composition.h:387`) has exactly ONE
   call site in the whole app: `MainComponent.cpp:4131`, inside the
   `kCompCollectMedia` handler ("Collect Media..." menu item), as a side
   effect of collecting media files into a folder. CONFIRMS the plan's
   "comps save only as a side effect of Collect Media" claim exactly.

3. `Composition::loadFromFile` (`model/Composition.h:393`) has **zero**
   callers anywhere in src/ or tests/ — confirmed by two independent grep
   patterns that agree (see CALL-SITE ENUMERATION #1). The only
   `.loadFromFile(` call in the whole repo that is NOT the declaration
   itself is `bindingManager_.loadFromFile(...)` (`MainComponent.cpp:4729`,
   an unrelated MIDI/OSC-binding system). CONFIRMS the plan's claim exactly.

4. `MainComponent::kCompNew` (`MainComponent.cpp:4055-4067`) is the ONLY
   place in the app today that replaces the live `composition_`'s contents
   wholesale:
   ```
   case C::kCompNew:
       undoService_.withDeckDetached([this] { composition_.initDefault(); });
       undoManager_.clear();
       if (deckView_) deckView_->rebuildGrid();
       if (inspectorPanel_) inspectorPanel_->refresh();
       break;
   ```
   This IS a correct, working precedent for the GL-fence + undo-history part
   of a model swap (see ROOT CAUSE below for what it is missing).

5. `BrowserPanel` (constructed at `MainComponent.cpp:1403`, wired to
   `composition_` at `MainComponent.cpp:1406`
   `browserPanel_->setComposition(&composition_);`) owns a live, visible
   `CompDecksBrowser` tab ("Comp/Decks", `BrowserPanel.h:35/58`). Its three
   callbacks `onCompositionLoad`, `onDeckLoad`, `onCompositionSave`
   (`CompDecksBrowser.h:40-42`) are declared but **never assigned** anywhere
   in the app (confirmed — see CALL-SITE ENUMERATION #2). Clicking a saved
   composition/deck entry, or the "Save Composition" button, silently no-ops
   today (`if (owner_.onCompositionLoad) …` guards are always false —
   `CompDecksBrowser.cpp:105-108`, `:137-139`, `:185`). The "Save Deck"
   button (`saveDeckBtn_`, `CompDecksBrowser.cpp:79-89`) is NOT one of the
   three dead callbacks — it already works today, writing `deck->toVar()`
   directly with no callback indirection.

6. Genre-aware automation (`MainComponent.cpp:598-630`): the genre-changed
   handler (gated on `composition_.autoPresetOnGenre`, reads
   `composition_.genreDeckAssignment[genre]`, calls `handleDeckSwitch`) is
   fully implemented and correct — VERIFIED, not a stub. The
   structural-state-changed handler
   (gated on `composition_.structuralSceneEnabled`) is a **bare log**
   (`std::cerr << "[P23] Structural state: " …`) — VERIFIED, matches the
   plan's claim exactly, this is a real, present-tense gap, not merely
   flag-gated. `composition_.smartAutopilotEnabled` has **zero** readers
   anywhere outside `model/Composition.h` — VERIFIED with two independent
   grep patterns (CALL-SITE ENUMERATION #9). **Correction to the plan:**
   `composition_.genrePresetNames[8]` (the per-genre effect-preset-name
   field) *also* has zero runtime readers — the genre-changed handler only
   consumes `genreDeckAssignment`, never `genrePresetNames`. The plan's
   "still parked" list names only `smartAutopilotEnabled` and the
   structural-scene handler; it does not mention that half of the
   genre-preset feature (the preset-by-name half) is equally unreachable.
   Nothing in this lane needs to fix that — it is orthogonal to whether
   loading works — but "activates for free" should be read as "the
   deck-switch half of genre automation activates for free," not the whole
   P23 family.

7. **Correction to a stale repo gotcha:** `.harmony/gotchas.md:55-61`
   records "Composition transform fields are runtime-only (not serialized)"
   as a 2026-05-21 finding, but its own text says it was **FIXED
   2026-07-17** (Wave 1-C). Read fresh at HEAD: `compPositionX/Y`,
   `compScale`, `compRotation`, `compAnchorX/Y` ARE unconditionally written
   in `toVar()` (`Composition.h:186-191`) and read back (guarded by
   `hasProperty` for back-compat) in `fromVar()` (`Composition.h:283-294`).
   The task prompt's framing ("a gotcha says transform fields are
   runtime-only") is describing a resolved historical gotcha, not a current
   defect — confirmed serialized today.

## ROOT CAUSE (verified) — the precise defect, with anchors

The plan's stated root cause (Open/Save/SaveAs wired to the wrong system;
`loadFromFile` has zero callers) is CONFIRMED exactly as stated — see
CURRENT BEHAVIOUR #1-3. Building `loadComposition` "fenced per the kCompNew
template" is the right instinct, but the template itself is **incomplete**
in a way that matters more once it's reachable via File > Open than it did
for File > New:

### TRAP #1 (HIGH SEVERITY, VERIFIED, crash-class) — kCompNew's refresh leaves dangling `Clip*`/`Layer*` in the inspector, and a 30Hz timer guarantees the dereference

- `ClipInspector` holds a raw `Clip* clip_ = nullptr;` (`ui/ClipInspector.h:69`),
  set by `ClipInspector::setClip(Clip*, EffectScope)` (`ui/ClipInspector.cpp:746`).
  `LayerInspector` holds a raw `Layer* layer_ = nullptr;` (`ui/LayerInspector.h:67`)
  the same way. These get set whenever the user clicks a clip cell / layer
  strip (`MainComponent.cpp:637-643`, `inspectorPanel_->inspectClip(clip, …)`,
  which auto-switches the inspector to the Clip tab — documented behaviour,
  `ui/InspectorPanel.h:19-22`).
- `composition_.initDefault()` (called inside kCompNew's fence) does
  `decks.clear()` (`Composition.h:110`), which destroys every `Deck` →
  every `Layer` → every `Clip` object the old model owned. Any
  `clip_`/`layer_` pointer captured before the swap is now dangling.
- kCompNew's post-fence refresh is exactly two calls:
  `deckView_->rebuildGrid()` and `inspectorPanel_->refresh()`
  (`MainComponent.cpp:4066-4067`). Neither re-points or nulls
  `clipInspector_.clip_` / `layerInspector_.layer_`.
  `InspectorPanel::refresh()` (`ui/InspectorPanel.cpp:170-178`) only
  refreshes the currently ACTIVE tab: `case Tab::Clip: clipInspector_.refresh(); break;`
  → `ClipInspector::refresh()` (`ui/ClipInspector.cpp:817-819`):
  `if (clip_) { syncFromClip(); … }` — dereferences the dangling pointer
  unconditionally if `clip_` is non-null (it is: a dangling pointer is
  never null).
- **This is not a "maybe" — `MainComponent::timerCallback()` runs at 30Hz
  (`MainComponent.cpp:257` `startTimerHz(30)`) and unconditionally calls
  `inspectorPanel_->refresh()` at ~10Hz** (`MainComponent.cpp:2583-2584`:
  `if (uiUpdateCounter_ % 3 == 0 && inspectorPanel_) inspectorPanel_->refresh();`).
  So: click any clip cell (Clip tab becomes active) → Composition > New
  Composition → within ≤100ms the timer fires the dangling dereference.
  This is a **pre-existing, live bug in kCompNew today**, not something L3
  introduces — but the plan's own words are "fenced per the kCompNew
  template," and copying that template verbatim into loadComposition
  reproduces the exact same crash on the far more common File > Open path.
- The general-purpose fix already exists in the codebase for the
  undo/redo case: `MainComponent::refreshAfterUndoRedo`
  (`MainComponent.cpp:3729-3789`) re-points both inspectors BY COORDINATE
  through `undoService_.resolveClip`/`resolveLayer` (null-safe:
  `ClipInspector::setClip(nullptr, …)` and `LayerInspector::setLayer(nullptr, …)`
  are both null-safe, `ClipInspector.cpp:746-761`, `LayerInspector.cpp:737-749`).
  But re-resolving BY COORDINATE is the wrong tool after a *full model
  swap* — the old coordinate might resolve to a perfectly valid cell in the
  NEW composition that has nothing to do with what was selected before
  (silently shows/edits the wrong clip, doesn't crash — arguably worse).
  The correct move for a full swap is to unconditionally clear the
  selection, not re-resolve it: `inspectorPanel_->getClipInspector().setClip(nullptr)`,
  `inspectorPanel_->getLayerInspector().setLayer(nullptr)`,
  `deckView_->clearSelection()` (`ui/DeckView.cpp:345-348`, clears
  `selectedCells_`, safe — it's coordinate-based, not a pointer),
  optionally `deckView_->selectLayer(-1)` (`ui/DeckView.cpp:368-373`, cosmetic
  only — `selectedLayerIndex_` is a plain int, never dangling, just
  possibly "wrong-looking selected" until this is called).

### TRAP #2 (MEDIUM SEVERITY, VERIFIED, stale-UI-class) — the composition inspector's global-effects rows do not rebuild on a plain refresh

- `CompositionInspector`'s `effectStackView_` is pointed at
  `&comp->globalEffects` once, at `setComposition()`
  (`ui/CompositionInspector.cpp:384-390`) or explicitly re-pointed via
  `rebuildEffectStack()` (`ui/CompositionInspector.cpp:401-408`,
  `effectStackView_.setEffects(&composition_->globalEffects, …)`).
  This pointer target is `&composition_.globalEffects` — the *address of
  the vector object*, which is a stable member of the stable `Composition`
  instance, so it does NOT dangle across a load (unlike TRAP #1's raw
  Clip*/Layer* into vector *elements*). It is, however, STALE in row
  *count*: `CompositionInspector::refresh()`
  (`ui/CompositionInspector.cpp:436-442`) calls
  `effectStackView_.refresh()`, and `EffectStackView::refresh()`
  (`ui/EffectStackView.cpp:137-139`) only updates existing rows' colors —
  it does not add/remove rows to match a changed vector size
  (`if (!effects_) return;` then iterates `rows_`, skipping any
  `row.effectIndex >= effects_->size()` — safe from OOB, but stale rows
  from the pre-load composition remain visible until something calls
  `setEffects`/`rebuildEffectStack` again).
  `refreshAfterUndoRedo` already knows this and explicitly calls
  `inspectorPanel_->rebuildCompositionEffects()` (`MainComponent.cpp:3787`)
  for exactly this reason. kCompNew's plain `inspectorPanel_->refresh()`
  does NOT call `rebuildCompositionEffects()` — same class of gap as TRAP
  #1, lower severity (stale display, not memory-unsafe, since the pointer
  target itself is stable).

### TRAP #3 (moderate severity, VERIFIED, NEW — not named by the plan) — per-layer GL resources in `CompositorEngine` are keyed by `Layer::id` and are never released on a composition swap; L1's mechanism does not reach them

- The plan says "close-all media via L1's mechanism," and L1
  (commit `349f676`) is scoped to exactly two maps in `Renderer`:
  `videoPlayers_` and `imageSequences_` (both `std::unordered_map<uint32_t, …>`
  keyed by **clip** id, `render/Renderer.h:411,415`), closed via
  `Renderer::closeMediaForClip(uint32_t clipId)` (`render/Renderer.cpp:1005-1038`).
- `CompositorEngine` (`render/CompositorEngine.h`) separately owns FOUR
  more maps keyed by **layer** id (not clip id):
  `feedbackProcessors_` (`:145`), `layerTemporalBuffers_` (`:158`,
  each entry an FBO+texture pair), `layerRingBuffers_` (`:180`, each entry
  up to 480 downscaled frames — the struct's own comment says "~120MB at
  480×270" per layer that uses it), and `layerOutputFBOs_`/
  `layerOutputTexStorage_`/`layerOutputTextures_` (`:254-258`). All are
  populated lazily via `getOrCreate*(layer.id, …)` — confirmed call site
  `CompositorEngine.cpp:753`: `getOrCreateFeedbackProcessor(layer.id)`.
- The ONLY code that ever calls `.clear()`/`glDelete*` on these maps is
  `CompositorEngine::releaseGL()` (`CompositorEngine.cpp:20-71`, invoked
  from GL-context teardown, `Renderer.cpp:758`) and, partially, `resize()`
  (`CompositorEngine.cpp:76-106`, output-FBOs only, on window resize).
  Neither runs on a composition load. **Confirmed with a targeted grep**
  (CALL-SITE ENUMERATION #6) that `DeckCommands.h` — which implements
  `RemoveLayerCmd`/`RemoveDeckCmd`, the only OTHER place a `Layer` dies
  today — has zero calls into `CompositorEngine`. So this leak already
  exists today for ordinary Remove Layer/Remove Deck (pre-existing, not
  introduced by this lane) — but it is a much smaller, rarer trigger than
  "every File > Open," which can replace dozens of layers across multiple
  decks in one call.
- Consequence: every composition load (a) leaks every temporal buffer/ring
  buffer/feedback processor/output FBO the OLD composition's layers had
  allocated (unbounded GPU memory growth across repeated loads in one
  session — a VJ set that opens several comps could accumulate hundreds of
  MB), and (b) because `Deck::nextLayerId_` (private, `model/Deck.h:191`,
  default 100, NOT serialized) resets to its class-default on every
  freshly-constructed/loaded `Deck` (fromVar never touches it —
  `model/Deck.h:168-185` sets `name`/`id`/`numColumns`/`layers` only), a
  **newly loaded layer can be assigned the same small id space** that a
  pre-load layer already occupied in `CompositorEngine`'s maps, silently
  inheriting a stale, wrongly-sized GL resource from an unrelated old
  layer (a rendering-correctness bug, not a crash: e.g. temporal-feedback
  bleed from a layer that no longer exists).
- `deck.id` (as opposed to `layer.id`) was checked too and has **zero**
  runtime consumers anywhere outside `model/Composition.h`/`model/Deck.h`
  (CALL-SITE ENUMERATION #7) — so `Composition::nextDeckId_`
  (`Composition.h:405`, same "not serialized, resets on load" shape) is
  low-severity: an id-collision there has no known functional
  consequence today, only a latent one if a future feature starts keying
  something off `deck.id`.
- **Recommendation:** treat the full CompositorEngine fix (a GL-thread-safe
  release path analogous to L1's retire-list, since these are raw
  `glDelete*` calls, not FFmpeg-safe-to-close-off-thread like
  `VideoPlayer`/`ImageSequence`) as a SEPARATE follow-up lane — it is its
  own "medium" scope, structurally similar to L1 itself, and is not
  required for loadComposition to be memory-safe (it is a leak + a
  latent-correctness risk, not a UAF). DO include the cheap, purely
  additive, zero-risk half in this lane: bump `nextDeckId_`/`nextLayerId_`
  past the max loaded id inside `Composition::fromVar()`/`Deck::fromVar()`
  respectively (mirrors the plan's own `s_nextClipId` ask, same shape,
  same files already being touched for serialization) — this at least
  stops NEW post-load layers/decks from colliding with old ids, even
  though it does not reclaim the leaked GL memory. See OUT OF SCOPE.

### `s_nextClipId` (the plan's own named risk) — confirmed real, confirmed the right shape

- `static uint32_t s_nextClipId = 1000;` (`MainComponent.cpp:11`, internal
  linkage — file-static, so the bump-logic MUST live in
  `MainComponent.cpp`, it cannot be added to `Composition.h`/`Deck.h`/`Clip.h`).
  Six increment sites, all `clip.id = s_nextClipId++;`
  (`MainComponent.cpp:1071,1200,1259,1433,3810,3884` — confirmed exhaustive,
  CALL-SITE ENUMERATION #4).
- `Clip::id` IS serialized (`Clip.cpp:7` write, `:126` read) — confirmed.
- Collision consequence, traced end to end: `Renderer::videoPlayers_`/
  `imageSequences_` are keyed by `clip.id` (`render/Renderer.h:411,415`).
  If a loaded composition contains a clip with `id == 1500` and the app's
  `s_nextClipId` is still at its default 1000, the very next clip the user
  drops (BEFORE reaching 1500 increments) will eventually mint `id = 1500`
  again — `openVideoForClip(1500, newFile)` (`Renderer.cpp:982-990`) does
  `videoPlayers_[1500] = std::move(player);`, silently **overwriting** the
  loaded clip's still-referenced VideoPlayer entry with the new clip's
  player. The loaded clip (if still present in another cell — recall the
  `makeClipMediaDisposeHook` invariant, `MainComponent.cpp:3629-3637`,
  "a clip id can appear in AT MOST one live cell at a time") would then be
  either orphaned (dangling map entry pointing at the WRONG file) or, if
  it's still live, actively broken (rendering the new clip's video under
  its own id). Fix: after a successful load, walk every clip in
  `composition_.decks` and set
  `s_nextClipId = std::max(s_nextClipId, maxLoadedClipId + 1);` in
  `MainComponent.cpp` (same translation unit as the static).

## FILES TOUCHED — exhaustive list, each with why

1. `src/MainComponent.cpp` — the bulk of the change:
   - Replace `case C::kCompOpen: loadPreset();` with a new
     `loadComposition()` private method (FileChooser scoped to
     `CompDecksBrowser::getCompositionsDir()` — reuse that static, it's
     already public: `ui/CompDecksBrowser.h` declares
     `static juce::File getCompositionsDir();`).
   - Split `kCompSave`/`kCompSaveAs` (currently both call `savePreset()`,
     `MainComponent.cpp:4069-4073`) into real `saveComposition()` (writes
     to `composition_.filePath` if it's set and exists, else falls back to
     Save-As behaviour — `composition_.filePath` currently has ZERO
     readers anywhere in the app, confirmed CALL-SITE ENUMERATION #8, it is
     only ever WRITTEN by `loadFromFile` at `Composition.h:400`) and
     `saveCompositionAs()` (always prompts).
   - New private helper (recommend, not required) that both `kCompNew` AND
     the new `loadComposition()` call, e.g.
     `swapCompositionModel(std::function<void()> mutation)`, doing:
     fence the mutation via `undoService_.withDeckDetached`, then
     `undoManager_.clear()`, `inspectorPanel_->getClipInspector().setClip(nullptr)`,
     `inspectorPanel_->getLayerInspector().setLayer(nullptr)`,
     `deckView_->clearSelection()`, `deckView_->selectLayer(-1)`,
     `deckView_->rebuildGrid()`, `inspectorPanel_->refresh()`,
     `inspectorPanel_->rebuildCompositionEffects()`. Sharing this ALSO
     fixes kCompNew's TRAP #1/#2 gaps as a byproduct — recommended, but if
     the builder prefers to touch only `loadComposition`'s own path to keep
     the diff minimal, TRAP #1 must still be fixed there independently
     (kCompNew's existing bug is real either way, but is not this lane's
     mandate to fix in isolation).
   - Bump `s_nextClipId` after a successful load (loop over
     `composition_.decks[*].layers[*].clips[*]`, same shape as the existing
     Collect Media loop at `MainComponent.cpp:4093-4118`).
   - Close old media (before the swap) / reconnect new media (after the
     swap) — see THE CHANGE.
   - Wire `browserPanel_->getCompDecksBrowser().onCompositionLoad = …`,
     `.onDeckLoad = …`, `.onCompositionSave = …` (near the other
     `browserPanel_->…` wiring, `MainComponent.cpp:1403-1420`).
2. `src/MainComponent.h` — declare the new private methods
   (`loadComposition`, `saveComposition`, `saveCompositionAs`, optionally
   `swapCompositionModel`).
3. `src/model/Composition.h` — add max-id bump for `nextDeckId_` at the end
   of `fromVar()` (`Composition.h:352-361` is where `decks` gets
   repopulated; add the bump right after that loop).
4. `src/model/Deck.h` — add max-id bump for `nextLayerId_` at the end of
   `fromVar()` (`Deck.h:174-184` is where `layers` gets repopulated; add
   the bump right after that loop).
5. `src/ui/CompDecksBrowser.h`/`.cpp` — **no changes needed.** The three
   callbacks and `getCompositionsDir()`/`getDecksDir()` are already public
   and already correctly shaped (`std::function<void(const juce::File&)>`
   for load, `std::function<void()>` for save) — this lane only needs to
   ASSIGN them from MainComponent, not touch this class.

## CALL-SITE ENUMERATION — the grep commands run + their raw output

**#1 — `Composition::loadFromFile` has zero callers (two agreeing patterns):**
```
$ grep -rn "\.loadFromFile(" --include="*.cpp" --include="*.h" src/ tests/
src/MainComponent.cpp:4729:                    bindingManager_.loadFromFile(results.getFirst());
```
```
$ grep -rn "loadComposition" --include="*.cpp" --include="*.h" src/ tests/
(no output — exit code 1, zero matches)
```
Only `bindingManager_.loadFromFile` exists as a call; `composition_.loadFromFile`
appears nowhere, and the symbol `loadComposition` (the plan's proposed name)
does not exist yet anywhere in the codebase — it is to be created, not a
dead function being revived.

**#2 — CompDecksBrowser's three callbacks are never assigned (two agreeing patterns):**
```
$ grep -rn "getCompDecksBrowser()\." src/
(no output)
$ grep -rn "\.onCompositionLoad\s*=\|\.onDeckLoad\s*=\|\.onCompositionSave\s*=" src/
(no output)
```
Both patterns agree: zero assignments repo-wide. (The callbacks ARE invoked
internally inside `CompDecksBrowser.cpp` itself — `:107`, `:139`, `:185` —
but always guarded by `if (owner_.onXxx)`, which is always false.)

**#3 — `composition_.saveToFile`/`.loadFromFile` full call inventory:**
```
$ grep -rn "saveToFile\|loadFromFile" --include="*.cpp" --include="*.h" src/ tests/
src/MainComponent.cpp:4131:    composition_.saveToFile(compFile);      # kCompCollectMedia only
src/MainComponent.cpp:4714:    bindingManager_.saveToFile(...)         # unrelated (bindings)
src/MainComponent.cpp:4729:    bindingManager_.loadFromFile(...)       # unrelated (bindings)
src/ui/RecordPanel.cpp:59,78: recorder_->saveToFile/loadFromFile(...)  # unrelated (session recorder)
src/model/Composition.h:387,393: declarations
src/recording/SessionRecorder.{h,cpp}, src/binding/BindingManager.{h,cpp}: unrelated classes' own methods
```

**#4 — `s_nextClipId`, exhaustive:**
```
$ grep -n "s_nextClipId" -r src/ tests/
src/MainComponent.cpp:11:static uint32_t s_nextClipId = 1000;
src/MainComponent.cpp:1071:                clip.id = s_nextClipId++;
src/MainComponent.cpp:1200:        clip.id = s_nextClipId++;
src/MainComponent.cpp:1259:        clip.id = s_nextClipId++;
src/MainComponent.cpp:1433:        clip.id = s_nextClipId++;
src/MainComponent.cpp:3810:    clip.id = s_nextClipId++;
src/MainComponent.cpp:3884:    clip.id = s_nextClipId++;
```
Six sites, all in MainComponent.cpp (internal-linkage static — confirmed no
extern/header declaration exists, so no other TU can read or bump it).

**#5 — Undo Command subclass count, exhaustive:**
```
$ grep -n "^class .*: public Command" src/core/ClipCommands.h src/core/DeckCommands.h \
    src/core/EffectCommands.h src/core/TriggerCommands.h src/core/CompositeCommand.h
EffectCommands.h:86:class EffectStackCmd : public Command
TriggerCommands.h:54:class TriggerClipCmd : public Command
ClipCommands.h:70:class SetClipCmd : public Command
ClipCommands.h:136:class ToggleClipLockCmd : public Command
ClipCommands.h:178:class SwapClipsCmd : public Command
DeckCommands.h:54:class SetColumnCountCmd : public Command
DeckCommands.h:95:class RemoveColumnCmd : public Command
DeckCommands.h:253:class ClearLayerClipsCmd : public Command
DeckCommands.h:329:class ClearActiveClipCmd : public Command
DeckCommands.h:363:class ToggleLayerFlagCmd : public Command
DeckCommands.h:408:class AddLayerCmd : public Command
DeckCommands.h:472:class RemoveLayerCmd : public Command
DeckCommands.h:554:class MoveLayerCmd : public Command
DeckCommands.h:620:class AddDeckCmd : public Command
DeckCommands.h:700:class RemoveDeckCmd : public Command
DeckCommands.h:811:class SwitchDeckCmd : public Command
CompositeCommand.h:15:class CompositeCommand : public Command
```
17 subclasses — CONFIRMS the plan's count exactly (this was one of the two
counts the task explicitly warned might be wrong; it is not, this time).
Second pattern (raw pointer members, to check whether any subclass caches
`Deck*`/`Layer*`/`Clip*` across calls instead of re-resolving by
coordinate):
```
$ grep -n "Deck\* \|Layer\* \|Clip\* " src/core/{Clip,Deck,Effect,Trigger}Commands.h \
    src/core/CompositeCommand.h | grep -v "resolveDeck\|resolveLayer\|resolveClip\|(Deck\*\|(Layer\*\|(Clip\*\|std::function"
```
Every hit is either a local `resolve()`/inline resolver call inside a
method body (re-resolved on every `execute()`/`undo()`) or a doc-comment
explicitly stating "never stores a raw Layer*/Clip*" (`ClipCommands.h:15`,
`TriggerCommands.h:53`). No subclass declares a persistent raw
Deck*/Layer*/Clip* MEMBER. This is why `undoManager_.clear()` (not a
partial invalidation) is the correct fix: commands are coordinate-indexed,
so a stale command replayed against the new model does not crash (a
too-large index resolves to nullptr — proven by
`tests/test_undo_commands.cpp:1116` "Deck commands no-op on stale
coordinates (never crash)" and `:1379` the layer equivalent) but CAN
silently resolve to a valid, WRONG cell if the new model happens to have a
deck/layer/column at that same index — that is the actual hazard
`undoManager_.clear()` closes, not a crash.

**#6 — CompositorEngine per-layer maps: nothing releases them on layer/deck removal:**
```
$ grep -n "releaseLayer\|removeLayerResources\|compositor_\.\|CompositorEngine" src/core/DeckCommands.h
(no output)
```
```
$ grep -n "\.erase(\|feedbackProcessors_\.\|layerTemporalBuffers_\.\|layerRingBuffers_\.\|layerOutputTextures_\.\|layerOutputFBOs_\.\|layerOutputTexStorage_\." src/render/CompositorEngine.cpp
41:    feedbackProcessors_.clear();       # inside releaseGL() — GL context teardown only
49:    layerTemporalBuffers_.clear();     # inside releaseGL()
60:    layerOutputFBOs_.clear();          # inside releaseGL()
61:    layerOutputTexStorage_.clear();    # inside releaseGL()
62:    layerOutputTextures_.clear();      # inside releaseGL()
73:    layerRingBuffers_.clear();         # inside releaseGL()
103-105:                                  # inside resize() — output FBOs only, on window resize
```
Both patterns agree: no per-layer release exists on the ordinary
Remove-Layer/Remove-Deck path today, and none of these `.clear()` calls run
on a composition swap.

**#7 — `deck.id` has zero runtime (non-serialization) consumers:**
```
$ grep -rn "\bdeck\.id\b\|deck->id\b" --include="*.cpp" --include="*.h" src/ | grep -v "model/Deck.h\|model/Composition.h"
(no output)
```

**#8 — `Composition::filePath` has zero readers anywhere:**
```
$ grep -rn "\.filePath\b" --include="*.cpp" --include="*.h" src/ | grep -v "model/Composition.h"
(no output)
```
Only written, at `Composition.h:400` inside `loadFromFile`. A real `Save`
(as opposed to `Save As`) implementation needs to start reading this field.

**#9 — `smartAutopilotEnabled` / `genrePresetNames` zero-readers (two agreeing patterns):**
```
$ grep -rn "autoPresetOnGenre\s*=\|structuralSceneEnabled\s*=\|smartAutopilotEnabled\s*=\|genreDeckAssignment\[" --include="*.cpp" --include="*.h" src/ | grep -v "model/Composition.h"
src/MainComponent.cpp:615:        int deckIdx = composition_.genreDeckAssignment[genre];   # READ only, not an assignment target
```
```
$ grep -rln "autoPresetOnGenre\|structuralSceneEnabled\|genreDeckAssignment\|genrePresetNames\|smartAutopilotEnabled" --include="*.cpp" --include="*.h" src/
src/MainComponent.cpp
src/model/Composition.h
```
Only two files in the entire repo ever mention any P23 field. No UI
control (CompositionInspector, PreferencesDialog, MenuBarModel) exists
anywhere to set `autoPresetOnGenre`/`structuralSceneEnabled`/
`genreDeckAssignment`/`genrePresetNames` to anything other than their
compile-time defaults — the ONLY on-ramp to a non-default value today is a
hand-edited (or future-tooling-produced) composition JSON file. Worth
flagging to whoever tests this lane: "genre auto-switch activates for
free" is true of the CODE PATH, but there is no in-app way to author a
composition with `autoPresetOnGenre: true` short of hand-editing the
saved JSON (or, after this lane ships, a future settings UI — not part of
this lane).

## THE CHANGE — step by step

This is written as ONE coherent sequence; see SPLIT below for how to land
it as more than one commit.

1. **`Composition::fromVar` (`model/Composition.h`)**: immediately after
   the existing `decks` repopulation loop (ends at `Composition.h:361`),
   add:
   ```cpp
   for (const auto& d : decks)
       nextDeckId_ = std::max(nextDeckId_, d.id + 1);
   ```
   (needs `<algorithm>` for `std::max`, or hand-roll the comparison — file
   currently only includes `<cstdint>`/`<string>`/`<vector>`).

2. **`Deck::fromVar` (`model/Deck.h`)**: immediately after the existing
   `layers` repopulation loop (ends at `Deck.h:184`), add the analogous
   bump for `nextLayerId_` against every loaded `layer.id`.

3. **`MainComponent::loadComposition()`** (new private method,
   `MainComponent.h`/`.cpp`):
   a. Launch a `juce::FileChooser` in open mode, default directory
      `CompDecksBrowser::getCompositionsDir()`, filter `"*.json"`.
   b. In the async callback (already message-thread, like every other
      `fileChooser_->launchAsync` use in this file): load into a
      **temporary** `Composition temp;` via `temp.loadFromFile(file)` —
      NOT directly onto the live `composition_` — so a bad/wrong-format
      file (see TRAP note below) never touches live state. `loadFromFile`
      already fails closed on unreadable/unparseable files
      (`Composition.h:396-398`, returns false before calling `fromVar`) —
      but it does NOT fail on a well-formed-but-wrong-shape JSON (e.g. an
      FX-preset `.json` or a lone-deck `.json`, both of which parse fine
      as objects but have no `"decks"` array, or an empty one) — additively
      check `if (temp.decks.empty()) { show an error, return; }` after a
      structurally-successful load, since `Composition::getActiveDeck()`
      returning nullptr forever is effectively a bricked session with no
      in-app recovery other than manually adding a deck back.
   c. If validation passes: walk the CURRENT (about to be replaced)
      `composition_.decks` and call
      `previewPanel_.getRenderer().closeMediaForClip(clip.id)`
      (`render/Renderer.h:188`) for every playable clip — this is
      message-thread-safe already (`Renderer.cpp:1007-1010`'s own comment:
      "May run on the message thread… close() itself is thread-safe").
      Do this BEFORE the swap, using the OLD clip ids — after the swap
      those ids are gone. This does not need the GL fence (its own two
      mutexes, `videoPlayerMutex_`/`imageSeqMutex_`, already guard against
      the concurrent GL-thread reader).
   d. Perform the actual swap fenced:
      `undoService_.withDeckDetached([this, &temp] { composition_ = std::move(temp); });`
      — fencing only the cheap assignment (not the file I/O/JSON parse
      that already happened in step b) keeps the GL-thread block as short
      as possible, unlike kCompNew's template which fences
      `initDefault()` (also cheap, so this distinction doesn't matter for
      kCompNew, but does matter here where step (b) could be a
      non-trivial file read for a large composition).
   e. After the fence returns: walk `composition_.decks` and call
      `renderer.openVideoForClip(clip.id, clip.mediaFile)` /
      `openImageSequenceForClip(clip.id, clip.sequenceFiles, clip.sequenceFps)`
      for every playable clip (both message-thread-safe per their own
      header comments, `render/Renderer.h:175-179`) — this is the
      "reconnect every playable cell" step. (The existing
      `makeClipMediaHook()` lambda at `MainComponent.cpp:3596-3606` does
      almost exactly this per-clip check already — reuse its body/logic
      rather than re-deriving it, it already handles the
      reopen-iff-missing-or-different-file case via `needsVideoReopen`.)
   f. Bump `s_nextClipId` past the max id found while walking
      `composition_.decks` in step (e) (or fold it into the same walk).
   g. `undoManager_.clear();`
   h. `inspectorPanel_->getClipInspector().setClip(nullptr);`
      `inspectorPanel_->getLayerInspector().setLayer(nullptr);`
      (fixes TRAP #1 for this path — REQUIRED, not optional, given the
      10Hz timer).
   i. `deckView_->clearSelection(); deckView_->selectLayer(-1);`
   j. `deckView_->rebuildGrid();`
   k. `inspectorPanel_->refresh(); inspectorPanel_->rebuildCompositionEffects();`
      (fixes TRAP #2).
   l. Update any "current file" UI label the way `loadPreset()` currently
      does (`MainComponent.cpp:2313-2314`, `fileLabel_.setText(...)`) —
      cosmetic parity, not required for correctness.

4. **`MainComponent::saveComposition()`** (new): if
   `composition_.filePath != juce::File()` and
   `composition_.filePath.getParentDirectory().exists()`, call
   `composition_.saveToFile(composition_.filePath)` directly, no dialog.
   Otherwise delegate to `saveCompositionAs()`.

5. **`MainComponent::saveCompositionAs()`** (new): `juce::FileChooser` in
   save mode, default directory `CompDecksBrowser::getCompositionsDir()`,
   then `composition_.saveToFile(file)` — note `saveToFile` does NOT set
   `composition_.filePath` itself (only `loadFromFile` does,
   `Composition.h:400`), so `saveCompositionAs` must set
   `composition_.filePath = file;` itself after a successful save, or a
   subsequent plain `Save` will not find it.

6. **Menu wiring** (`MainComponent.cpp`, replace `:4067-4073`):
   `kCompOpen` → `loadComposition()`; `kCompSave` → `saveComposition()`;
   `kCompSaveAs` → `saveCompositionAs()`.

7. **CompDecksBrowser wiring** (near `MainComponent.cpp:1403-1420`, right
   after `browserPanel_->setComposition(&composition_);`):
   ```cpp
   browserPanel_->getCompDecksBrowser().onCompositionLoad = [this](const juce::File& f) {
       loadComposition(f);   // loadComposition takes the target file directly here,
   };                        // not via its own FileChooser — factor the FileChooser
                              // launch out of loadComposition() into the kCompOpen
                              // case, and give loadComposition(const juce::File&) the
                              // load-temp/validate/close-old/swap/reconnect/refresh body.
   browserPanel_->getCompDecksBrowser().onDeckLoad = [this](const juce::File& f) {
       // Loads ONE deck into the ACTIVE deck slot, not a whole-composition swap —
       // narrower blast radius (only the active deck's layers/clips are replaced),
       // but the SAME hazards apply at deck scope: old clip media in the active
       // deck must close, new clip media must reconnect, the inspector's clip_/
       // layer_ must be nulled if they pointed into the active deck, and this
       // mutation must still go through undoService_.withDeckDetached (it
       // reallocates the active deck's layers vector) + undoManager_.clear().
   };
   browserPanel_->getCompDecksBrowser().onCompositionSave = [this] { saveComposition(); };
   ```
   `onDeckLoad`'s scope is smaller than the full composition swap
   (Deck::fromVar only, applied to `composition_.getActiveDeck()`), but it
   is NOT free of the fencing/undo-clear/media-close/reconnect/inspector-
   null requirements — it needs the same treatment, scoped to one deck's
   clips instead of the whole composition's. Do not skip the fence just
   because it's "smaller."

## FENCE — every file this lane will WRITE

- `src/MainComponent.h`
- `src/MainComponent.cpp`
- `src/model/Composition.h`
- `src/model/Deck.h`

No other file needs a write for the core lane. (`CompDecksBrowser.h`/`.cpp`
are read-and-reused, not modified — their public surface already fits.)

## TRAPS — what will bite the builder

1. **TRAP #1 above (dangling `Clip*`/`Layer*`) is the single highest-risk
   item in this lane.** It is easy to miss because kCompNew "looks like it
   works" in casual testing (nobody sits with a clip selected for 100ms
   doing nothing after hitting New Composition, and even if they do, a
   crash there gets attributed to something else). It becomes MUCH more
   visible once Open is real, because Open is used far more often and by
   more experienced users who leave a clip selected while browsing files.
   Do not skip step 3h.
2. **Threading is uniform message-thread for this whole lane** — every
   call this packet names (`FileChooser::launchAsync` callback,
   `Composition::loadFromFile`, `closeMediaForClip`, `openVideoForClip`,
   `openImageSequenceForClip`, `undoService_.withDeckDetached`,
   `undoManager_.clear()`, every UI refresh call) is documented and/or
   VERIFIED message-thread-safe. The ONLY GL-thread-must-be-current
   operation anywhere near this lane is `CompositorEngine::releaseGL()`
   (TRAP #3), which this lane deliberately does NOT call — do not add a
   call to it without first building the GL-thread-safe retire-list this
   packet recommends deferring.
3. **`withDeckDetached` is not reentrant** (`UndoService.cpp:34-42`,
   asserted). Do not nest the composition-swap fence inside anything else
   that is itself fenced — it must be a top-level call from the menu
   handler / FileChooser callback, exactly like kCompNew's call site is.
4. **A failed load must leave `composition_` completely untouched** —
   this is why step 3b uses a temporary `Composition`, not
   `composition_.loadFromFile(file)` directly. Calling `loadFromFile`
   directly IS safe for a genuinely malformed/unreadable file (fails
   before `fromVar` runs) but is NOT safe for a well-formed-JSON,
   wrong-shape file (empty `decks` array) — that case silently succeeds
   and leaves you with a comp that has zero decks and no in-app way back
   without hand-adding a deck.
5. **Don't reorder step 3c relative to step 3d.** Old clip ids must be
   read from `composition_.decks` BEFORE the swap overwrites it. (Trivial
   once stated, easy to get backwards while refactoring kCompNew's
   one-line `initDefault()` call into a multi-step sequence.)
6. **`s_nextClipId` is a file-static in `MainComponent.cpp`** — the bump
   logic must live there, it cannot be added to `Composition.h`/`Clip.h`
   the way the `nextDeckId_`/`nextLayerId_` bumps can (those ARE private
   members of the classes doing their own `fromVar`, so they belong there
   instead).
7. **`onDeckLoad`'s scope is a single deck, not the whole composition** —
   resist the temptation to route it through the exact same
   `swapCompositionModel`/`loadComposition` helper unmodified; it needs
   its own, narrower version that only touches the active deck's clips
   (see step 7's inline note). Reusing 100% of the composition-swap logic
   for a deck-scoped load would incorrectly close/reconnect media for
   EVERY deck in the composition, not just the one being replaced.
8. **Don't confuse this lane's "Deck" persistence with the pre-existing,
   unrelated `PresetManager::saveDeck`/`loadDeck` system**
   (`ui/PresetManager.cpp`, "Deck save/load" section, wired to
   `MainComponent::saveDeck()`/`loadDeck()` at `MainComponent.cpp:2749,2803`,
   themselves wired to standalone toolbar buttons
   `deckSaveButton_`/`deckLoadButton_`, `MainComponent.cpp:187-188` — NOT
   the Composition menu). This is a third, independent "deck" file format
   (a legacy `DeckState` struct with an embedded FX preset,
   `PresetManager.cpp:478-524`), unrelated to `Deck::toVar()`/`fromVar()`
   (the format `CompDecksBrowser`'s "Save Deck" button and `onDeckLoad`
   use). Three different "save a deck" surfaces exist in this app
   simultaneously; this lane only touches the `Deck::toVar()`/`fromVar()`
   one.

## HOW TO PROVE IT WORKS

Automatable (headless, Catch2, no GL/app needed):
- Round-trip `Composition::saveToFile`/`loadFromFile` through a real
  `juce::File` (existing tests only exercise in-memory `toVar`/`fromVar` —
  CALL-SITE ENUMERATION confirms zero existing `loadFromFile`/`saveToFile`
  test coverage). Cheap to add, not required to ship this lane.
- A test that after `nextDeckId_`/`nextLayerId_` bump-fix, loading a
  composition whose decks/layers have ids ≥100 and then calling
  `addDeck()`/`addLayer()` produces a NEW id strictly greater than any
  loaded id (regression test for the fix in THE CHANGE steps 1-2).

Needs the running app (a human, or the `run` skill driving it, per this
repo's convention — this recon does NOT run or build anything):
- Select a clip cell (Clip tab becomes active) → File > Open a saved
  composition → wait >150ms without touching anything else → confirm no
  crash (this is the concrete, reproducible regression test for TRAP #1;
  it is silent/invisible in a quick manual click-through, the timer needs
  a moment to fire).
- Build a composition with several clips across ≥2 decks, Save, quit/reopen
  (or just File > New then File > Open the saved file) — confirm every
  clip's media plays (video advances, image sequences animate) without
  having to re-drop the file — this exercises the reconnect step (3e) and
  the media-map-collision fix (`s_nextClipId` bump) together.
- With the same file, drop ONE new video clip after loading — confirm its
  playback is correct (not silently aliased to a loaded clip's decoder) —
  this is the concrete reproduction for the `s_nextClipId` collision bug
  if the bump (step 3f) is missing or wrong.
- Hand-edit a saved composition JSON to set `"autoPresetOnGenre": true`
  and a non-`-1` `"genreDeckAssignment"` entry for a genre your input
  source can trigger; load it; confirm the deck-switch fires on that genre
  change (this is currently the ONLY way to exercise that path end to end
  — there is no in-app UI to set it, see CALL-SITE ENUMERATION #9).
- Only a human can judge: whether the Composition inspector's "Global
  Effects" section visually looks right immediately after a load with a
  populated `globalEffects` list vs. an empty one (TRAP #2's regression
  test — row count must match, not just values).
- Only a human, and only over a longer session, can judge TRAP #3's GPU
  memory growth: repeatedly load compositions that use temporal-feedback
  or Screen-Split/Frame-Stutter layers (the ones that populate
  `layerTemporalBuffers_`/`layerRingBuffers_`) many times in one session
  and watch GPU memory (Activity Monitor / `previewPanel_`'s own
  FPS/frame-time readout degrading is a proxy) — this is the concrete
  reproduction for the deferred leak, useful evidence if it's later
  escalated into its own lane.

## OUT OF SCOPE — what you deliberately are NOT doing and why

- **TRAP #3's full fix** (a GL-thread-safe release mechanism for
  `CompositorEngine`'s four per-layer maps, analogous to L1's retire
  list). Reasoning: it is its own medium-sized lane (no existing
  "close-then-retire" pattern for raw `glDelete*` calls the way
  `VideoPlayer`/`ImageSequence` have `close()`), it is a pre-existing leak
  independent of this lane (Remove Layer/Remove Deck already trigger it,
  just less often), and it is a leak+correctness risk, not a crash —
  lower urgency than the two TRAPs this lane does fix. Recommend a named
  follow-up lane (or folding into `L-DEL`'s legacy-cleanup pass, since
  `L-DEL` already touches adjacent territory per the plan).
- **A settings UI for `autoPresetOnGenre`/`structuralSceneEnabled`/
  `genreDeckAssignment`/`genrePresetNames`.** Not asked for by the plan;
  loading a hand-authored file is sufficient to prove the mechanism works.
  Building the UI is a separate, larger feature.
- **The structural-scene handler's actual behavior** (currently a bare
  log). The plan explicitly parks this; recon confirms it is unchanged.
- **`smartAutopilotEnabled`'s consumer.** Same — explicitly parked by the
  plan, confirmed still zero readers.
- **`genrePresetNames`'s consumer** (the preset-by-name half of P23,
  as opposed to the deck-switch half). Not named by the plan at all; flagged
  above as a correction, but implementing it is new feature work, not a
  persistence-wiring fix.
- **The legacy `PresetManager::saveDeck`/`loadDeck` `DeckState` system**
  and its toolbar buttons. Untouched, unrelated file format, orthogonal to
  this lane (see TRAP #8).
- **Rewriting kCompNew to share the new helper** is RECOMMENDED (see FILES
  TOUCHED #1) but not mandatory — if the builder chooses to leave kCompNew
  exactly as it is today (with its own pre-existing TRAP #1/#2 bugs
  un-fixed) and only fix the new `loadComposition` path, that is an
  acceptable, smaller-diff choice; note it explicitly in the commit
  message if taken, since it leaves a known, now-doubly-confirmed bug in
  File > New Composition unaddressed.

## OPEN QUESTIONS — anything not settled from source

1. Should `saveComposition()`'s silent-overwrite-if-`filePath`-set
   behavior prompt a confirmation dialog, or just overwrite like most
   "Save" menu items do? No existing convention in this app to copy from
   (Preset Save always prompts via FileChooser today — there is no
   existing "Save" vs "Save As" distinction anywhere in the app to model
   this on). Left to the builder/Boris.
2. Should the FileChooser filter for File > Open be narrowed beyond
   `"*.json"` (e.g. checking the JSON has a `"decks"` key before even
   offering to load, to head off the "user picks an FX preset .json by
   mistake" case at the file-picker level rather than post-parse)? JUCE's
   `FileChooser` wildcard filtering is extension-only, so this would
   require a custom `FileBrowserComponent::FilenameFilter` or just
   catching it post-parse as this packet recommends (step 3b) — the
   latter is simpler and is what's specified above; flagging the
   alternative in case Boris prefers stronger up-front guarding.
3. `onDeckLoad`'s exact undo/redo interaction with the REST of the
   composition (only the active deck's coordinate space is replaced,
   other decks are untouched) was not traced against every one of the 17
   Command subclasses individually — the general resolver argument
   (CALL-SITE ENUMERATION #5) covers it in principle (stale coordinates
   into the untouched decks remain valid; stale coordinates into the
   replaced deck no-op or, per the same silent-wrong-cell risk as the
   full-composition case, could resolve wrong) but a builder implementing
   `onDeckLoad` should re-derive this rather than assume it is identical
   in shape to the full-composition case — this packet did not exhaustively
   verify deck-scoped undo interaction the way it did for the
   full-composition swap.
