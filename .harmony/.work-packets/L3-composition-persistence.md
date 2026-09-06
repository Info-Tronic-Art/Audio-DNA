# L3 — Composition Persistence — BUILD PACKET

Architect packet. Supersedes the design portions of
`L3-composition-persistence-RECON.md` (referenced below as RECON); the recon's
CURRENT BEHAVIOUR and CALL-SITE ENUMERATION sections stand as evidence and
are not restated here. Every load-bearing recon claim was re-verified against
HEAD `4216959` on 2026-09-05; the ones that were WRONG are listed in
§CORRECTIONS. Line numbers drift — every anchor carries its exact text.

## VERDICT: BUILDABLE — three commits, each independently shippable

## SIZE: medium-large (unchanged). Step 1 ~120 lines model+helper+tests, Step 2 ~200 lines MainComponent + 1-line header move, Step 3 ~60 lines.

## THE LANE IN ONE LINE
File > Open/Save/Save As (and Cmd+O / Cmd+S) operate on the v1 FX preset;
`Composition::loadFromFile` has zero callers; the Comp/Decks browser's three
callbacks are never assigned. A composition can be saved (only via Collect
Media) and never loaded.

---

## 0. WHAT THE RECON GOT RIGHT THAT THIS PACKET BUILDS ON (verified, not re-argued)

- Open/Save/SaveAs → `loadPreset()`/`savePreset()` (FX preset), `MainComponent.cpp` `case C::kCompOpen: loadPreset();` etc.
- `kCompNew` is the fence+clear precedent (`undoService_.withDeckDetached([this]{ composition_.initDefault(); }); undoManager_.clear();`), and its refresh leaves `ClipInspector::clip_` / `LayerInspector::layer_` dangling under a 10Hz `inspectorPanel_->refresh()` timer (`startTimerHz(30)`; `if (uiUpdateCounter_ % 3 == 0 && inspectorPanel_) inspectorPanel_->refresh();`). Verified crash class.
- 17 `Command` subclasses; none stores a raw model pointer (proof in §2).
- `CompDecksBrowser` callbacks `onCompositionLoad/onDeckLoad/onCompositionSave` unassigned; `BrowserPanel::getCompDecksBrowser()` exists (`BrowserPanel.h:35`).
- Per-layer GL resources in `CompositorEngine` keyed by `layer.id` are never released on any model change — pre-existing, OUT OF SCOPE (§7).

---

## 1. ORDER OF OPERATIONS — the composition load, step by step, with thread and WHY

All steps run on the MESSAGE THREAD (FileChooser completion callback, or a
browser-row `mouseDown`). The only cross-thread interactions are (a) the
fence's one blocking empty GL job, (b) `closeMediaForClip`'s mutex hand-off
to L1's retire list (drained on the GL thread next frame), (c)
`openVideoForClip`/`openImageSequenceForClip`'s mutex-guarded map insert.
Nothing in this lane runs on the GL thread and nothing new is added to it.

### What the GL thread reads, lock-free, that the fence must cover (verified)
- `Renderer::renderOpenGL()`: `Deck* deck = activeDeck_.load(std::memory_order_acquire); bool deckActive = (deck != nullptr);` then, ONLY inside `if (deckActive) { … }`: `compositor_.compositeDeck(*deck, …)` AND the P21 loop `if (composition_) for (auto& otherDeck : composition_->decks) … compositePersistentLayers(otherDeck, …)`. The persistent-layer loop walks `composition_->decks` directly, but it is nested under the `deckActive` check — so nulling `activeDeck_` (what `withDeckDetached` does) fences it too. RECON did not name this loop; it is why `withDeckDetached` is sufficient for a whole-composition swap and not just an active-deck mutation.
- `Renderer::getVideoFrameTexture(const Clip*, dt)` (GL thread) looks up `videoPlayers_.find(clip->id)` under `videoPlayerMutex_` and WRITES `clip->playheadPosition` / `clip->playing` — through `activeDeck_`, so fenced.
- `CompositorEngine` caches NO `Clip*`/`Layer*`/`Deck*` and no clip id across frames (grep: header has only the `VideoFrameFn` typedef; `.cpp` has zero `clip.id`/`clipId` uses). Its per-layer maps are looked up by `layer.id` per frame. Consequence: once a frame has completed after the swap, no GL-side state references an OLD clip id — closing old media AFTER the swap is safe.
- Outside the `deckActive` branch the GL thread reads POD scalars of the stable `Composition` object (`composition_->activeDeckIndex`, `globalTransitionSpeed`, `crossfaderBlendMode`, `compPositionX…`). These are pre-existing benign races during any model mutation (kCompNew, AddDeckCmd) — the object address is stable, no pointer is chased. Not this lane's.

### THE SEQUENCE (message thread, synchronous, no message-loop pump anywhere inside)

```
loadComposition(const juce::File& file):

 1. STAGE      Composition incoming;
               if (!incoming.loadFromFile(file)) → alert "could not read/parse", RETURN. Live state untouched.
 2. VALIDATE   reason = compload::validateComposition(incoming);      // §4 rules (refuse / repair)
               if (!reason.empty()) → alert reason, RETURN.           // Live state untouched.
 3. RE-MINT    compload::remintClipIds(incoming, s_nextClipId);       // §3 — EVERY clip gets a fresh id
 4. OPEN NEW   for every playable clip in `incoming`: open its media under its NEW id and fill
               thumbnail/dims INTO `incoming` (mirrors applyFileDrop/applyMultiFileDrop exactly).
               Per-clip failure (missing file) is non-fatal: skip, continue.
 5. NAME       incoming.name = file.getFileNameWithoutExtension();  (design decision, §8 Q3)
 6. SWAP       swapCompositionModel([&]{ composition_ = std::move(incoming); });
               — the shared helper below does: capture before-ids → withDeckDetached(mutation)
                 → close (before \ after) → undoManager_.clear() → refreshUiAfterModelSwap()
 7. LABEL      fileLabel_.setText("Loaded: " + file.getFileNameWithoutExtension(), dontSendNotification);
               browserPanel_->getCompDecksBrowser().refresh();
```

```
swapCompositionModel(const std::function<void()>& mutation):   // shared by kCompNew, load, deck-append

 a. before = compload::playableClipIds(composition_);            // OLD ids, read while the old model is live
 b. undoService_.withDeckDetached(mutation);                     // GL fence: activeDeck_=null, drain one frame,
                                                                 // run mutation, guard re-points renderer at
                                                                 // composition_.getActiveDeck() (the NEW deck)
 c. after  = compload::playableClipIds(composition_);
    for (id : compload::idsRetired(before, after)) previewPanel_.getRenderer().closeMediaForClip(id);
 d. undoManager_.clear();                                        // §2 — sufficient, proven
 e. refreshUiAfterModelSwap();                                   // below, internal order is load-bearing
```

```
refreshUiAfterModelSwap():   // message thread; ORDER MATTERS

 i.   inspectorPanel_->getClipInspector().setClip(nullptr);      // kills dangling Clip* AND re-points its
      inspectorPanel_->getLayerInspector().setLayer(nullptr);    // EffectStackView off &clip->effects (dead vector)
 ii.  deckView_->clearSelection(); deckView_->selectLayer(-1); deckView_->setActiveColumn(-1);
 iii. deckView_->rebuildGrid();                                  // destroys+recreates every ClipCell/LayerStrip
                                                                 // (their raw Clip*/Layer*) and the deck tabs
 iv.  inspectorPanel_->rebuildCompositionEffects();              // global-FX row COUNT (refresh() only recolors)
 v.   inspectorPanel_->refresh();                                // now safe: no dangling pointer remains
 vi.  if (auto* d = composition_.getActiveDeck()) refreshPreviewFromActiveClip(*d);
                                                                 // resets renderer's global Source/Image fallback
                                                                 // the OLD comp may have left (layers load with
                                                                 // activeClipColumn = -1 → clears source+image+label)
```

### WHY THIS ORDER — what breaks under any other

| If you… | Then… |
|---|---|
| Touch anything live before VALIDATE passes | a refused file leaves side effects. VALIDATE and STAGE operate on `incoming` only, so refusal = zero side effects (§4). |
| Load with `composition_.loadFromFile(file)` directly | two failures: (1) a well-formed wrong-shape JSON (FX preset, lone deck) silently yields zero decks; (2) `fromVar` leaves every `hasProperty`-guarded field at its OLD value when the key is absent — a stale hybrid of two compositions. A fresh `incoming` has struct defaults. |
| OPEN before RE-MINT | opens are keyed by id; a file id equal to a LIVE clip's id would `operator[]`-overwrite that clip's `VideoPlayer` in place on the message thread (the L1-FU message-thread-destroy hazard — L1-FU is NOT landed: `Renderer.cpp` `openVideoForClip` still does a bare `videoPlayers_[clipId] = std::move(player);`) and alias the loaded clip to a live decoder. Fresh ids have no live entry, so the insert destroys nothing — this lane is safe without L1-FU. |
| OPEN after SWAP | (1) thumbnails/dims would be written into LIVE `Clip` structs the GL thread may be reading (`juce::Image` assignment = ref-count pointer swap → torn read → crash class) — post-swap mutation of a live clip needs its own fence; pre-swap fill needs none. (2) The output would show the new comp (black — nothing is active yet) for the whole FFmpeg-probe duration. Pre-swap, the GL thread keeps rendering the OLD comp while the message thread opens files: the cut happens at the fence, when everything is ready. |
| Put OPEN inside the fence | `activeDeck_` is null for the duration → output blackout scaled by N × FFmpeg probe. The fenced region must be the cheap move only. (RECON's reasoning was "GL-thread block"; the GL thread is not blocked by the mutation — the message thread blocks once on the empty GL job, and the fenced window is a blackout. Same conclusion, different mechanism.) |
| SWAP outside the fence | `composition_ = std::move(incoming)` replaces `decks` storage under the GL thread's lock-free `activeDeck_->layers[].clips[]` and `composition_->decks` walks — the exact reallocation-under-read UAF class the 2026-07-28 family-fence fix closed (see `CompDecksBrowser.h:26-39`'s FUTURE-FENCE REQUIREMENT comment). |
| CLOSE OLD before the swap | the old comp's video layers go black on the output while still live (visible glitch before the cut). After the swap, the retired players are unreferenced by any frame (fence drained the in-flight one; compositor caches no clip ids) and `closeMediaForClip` is message-thread-safe by L1's design. |
| CLOSE OLD as "close everything that was live" | correct for full swap and kCompNew, WRONG for deck-append (closes media of decks that survive). `idsRetired(before, after)` is correct for all three by construction — the same liveness principle as `makeClipMediaDisposeHook`. |
| Skip `undoManager_.clear()` or run it inside the fence | §2. And house doctrine (kCompNew's comment): clear() touches only history, stays outside the fence. Order vs close/refresh is free (synchronous — no undo can interleave); do it right after the fence. |
| Refresh the inspector before nulling `clip_`/`layer_` | `ClipInspector::refresh()` does `if (clip_) { syncFromClip(); … }` — dangling pointer is non-null → UAF. And the 10Hz timer will do it for you within 100ms of returning. Null FIRST. |
| Call `inspectorPanel_->refresh()` and skip `rebuildCompositionEffects()` | `EffectStackView::refresh()` recolors existing rows only; the global-FX row count stays that of the OLD comp (RECON TRAP #2). |
| Skip `refreshPreviewFromActiveClip` | the renderer's global fallback (`activeSourceType_`/loaded image) is not deck state — the OLD comp's procedural source or still image keeps rendering underneath an empty new deck (the exact bug `handleDeckSwitch` already reconciles this way). |
| Set `composition_.name` after the swap | harmless data-wise but it is a write to the live object; do it on `incoming`. |

### kCompNew, re-based on the shared helper (mandatory, 3 lines)
```cpp
case C::kCompNew:
    // (keep the existing GL-fence comment)
    swapCompositionModel([this] { composition_.initDefault(); });
    break;
```
This fixes, for free and by the same mechanism: kCompNew's live TRAP #1/#2
(dangling inspector pointers, stale global-FX rows) AND kCompNew's
never-named media leak (`initDefault()` drops every clip; nothing closed
their players — the same L1 family). The MUTATION is unchanged
(`initDefault()` resets the same subset of fields it always did — do NOT
"improve" it to a full-struct reset; see §7).

---

## 2. THE UNDO PROBLEM — `undoManager_.clear()` is sufficient. Proof.

`UndoManager::clear()` = `history_.clear(); currentIndex_ = 0; onHistoryChanged();`
(`UndoManager.cpp:108-113`). Destroys every `Command`. Sufficiency requires
(A) no command holds a pointer/reference into the model that its DESTRUCTOR
or any later call could touch, and (B) nothing else replays or merges
commands after the swap.

(A) Every persistent member of all 17 subclasses, enumerated from the
headers (`grep -n "_\s*\(=\|;\|{\)" src/core/*Commands.h CompositeCommand.h`):

| Class | Members |
|---|---|
| SetClipCmd | `ClipLayerResolver resolver_; DeckFenceHook fence_; ClipMediaHook mediaHook_; ClipMediaDisposeHook disposeHook_; int deckIndex_, layerIndex_, column_; std::optional<Clip> before_, after_; std::string description_;` |
| ToggleClipLockCmd | resolver, 3 ints, `bool before_, after_`, string |
| SwapClipsCmd | deckResolver, fence, 2 hooks, ints, `std::optional<Clip> srcBefore_, srcAfter_, dstBefore_, dstAfter_`, `int numColumnsBefore_, numColumnsAfter_`, string |
| SetColumnCountCmd | deckResolver, fence, `int deckIndex_, before_, after_`, string |
| RemoveColumnCmd | deckResolver, fence, 2 hooks, ints, `std::vector<std::optional<Clip>> removedCells_`, string |
| ClearLayerClipsCmd | resolver, fence, 2 hooks, ints, `LayerClipsSnapshot before_, after_`, string |
| ClearActiveClipCmd | resolver, ints, `LayerRuntimeSnapshot before_, after_`, string |
| ToggleLayerFlagCmd | resolver, ints, `Flag flag_; bool before_, after_`, string |
| AddLayerCmd | deckResolver, fence, `int deckIndex_; std::optional<Layer> added_; int addedIndex_`, string |
| RemoveLayerCmd | deckResolver, fence, 2 hooks, ints, `Layer removed_`, string |
| MoveLayerCmd | deckResolver, fence, 3 ints, string |
| AddDeckCmd | `CompositionResolver compResolver_; DeckFenceHook fence_; std::optional<Deck> added_; int addedIndex_, priorActiveIndex_`, string |
| RemoveDeckCmd | compResolver, fence, 2 hooks, `int deckIndex_; Deck removed_; int priorActiveIndex_`, string |
| SwitchDeckCmd | compResolver, `DeckActivateHook activate_; int before_, after_`, string |
| EffectStackCmd | compResolver, fence, `EffectScope scope_; std::vector<Clip::EffectSlot> before_, after_`, string |
| TriggerClipCmd | resolver, ints, `LayerRuntimeSnapshot before_, after_; std::optional<bool> targetPlayingBefore_, targetPlayingAfter_`, string |
| CompositeCommand | `std::string description_; std::vector<std::unique_ptr<Command>> children_;` |

Every model reference is either (i) a coordinate (`int`s / `EffectScope`),
(ii) a VALUE snapshot (`std::optional<Clip>`, `Layer`, `Deck`,
`LayerClipsSnapshot`, `LayerRuntimeSnapshot`, `std::vector<EffectSlot>`),
or (iii) a `std::function` resolver/hook. The resolvers all capture `this`
= `MainComponent` and resolve through `undoService_` (whose `composition_`
is the stable `&composition_` member — `undoService_.setCollaborators(&composition_, …)`)
or return `&composition_` directly (`makeCompositionResolver`). No raw
`Clip*`/`Layer*`/`Deck*`/`Composition*` member exists in any subclass —
`UndoService.h:13-14` states the invariant and the headers honor it.
Destroying a command destroys values (a `Clip` owns a CPU `juce::Image
thumbnail`, nothing GL) — message-thread-safe, touches no live object.
No subclass declares a destructor.

(B) Replay/merge paths: `UndoManager::undo/redo/perform` are the only
callers of `execute()/undo()`; `perform()` also tries
`prev->canMergeWith(*cmd)` against the LAST history entry (`TriggerClipCmd`
merges by coordinate). All three are message-thread-only (asserted,
`UNDO_ASSERT_MESSAGE_THREAD`). The swap sequence is synchronous on the
message thread, so no undo/redo/perform can interleave between the fence
and `clear()`; after `clear()` there is nothing to replay or merge with.

What `clear()` actually PREVENTS (the hazard, so nobody "optimizes" it
away): a stale command re-resolves its coordinates against the NEW model.
Out-of-range → `nullptr` → no-op (proven by
`tests/test_undo_commands.cpp` "…no-op on stale coordinates (never crash)").
In-range → resolves a VALID, WRONG cell: `SetClipCmd::undo` writes
`before_` (an OLD clip whose id is closed) into the new comp and
`mediaHook_` reopens the old file under the old id; `RemoveLayerCmd::undo`
re-inserts `removed_` with old clip ids. Memory-safe, semantically wrong.
`clear()` is the whole fix; nothing else is needed. Same reasoning holds
for deck-append (RECON OPEN QUESTION 3 — settled: clear on every load,
one rule).

Other holders checked and cleared by the refresh helper: `DeckView::
selectedCells_` (coordinates → `clearSelection()`), `selectedLayerIndex_`
(`selectLayer(-1)`), `activeColumn_` (`setActiveColumn(-1)`),
`ClipCell::clip_`/`LayerStrip::layer_` (recreated by `rebuildGrid()`),
`ClipInspector::clip_`/`LayerInspector::layer_` + their `EffectStackView`
pointers (`setClip(nullptr)`/`setLayer(nullptr)` — both null-safe and both
re-point the stack view to null). `CompositionInspector`'s stack view
points at `&composition_.globalEffects` — a stable member address — so it
is re-pointed for row COUNT only (`rebuildCompositionEffects()`). Stable-
address holders needing nothing: `Renderer::composition_`, `UndoService::
composition_`, `DeckView/InspectorPanel/CompDecksBrowser::composition_`,
`TopBar::composition_` (reference), `BindingOverlay`'s reference,
`Renderer::setPerTypeAutopilotConfig(&composition_.perTypeAutopilot)`.

---

## 3. THE ID PROBLEM — re-mint every clip id on load; bump the layer/deck mints in `fromVar`

### Clip ids: RE-MINT ALL (correction to plan + RECON's "bump past max")
`s_nextClipId` is a file-static in `MainComponent.cpp:11`
(`static uint32_t s_nextClipId = 1000;`), six mint sites, all
`clip.id = s_nextClipId++`. Live consumers of a clip id: `Renderer::
videoPlayers_`/`imageSequences_` (keyed by id) and `makeClipMediaDisposeHook`'s
liveness scan ("a clip id can appear in AT MOST one live cell"). NOTHING
persistent references a clip id: `Route::targetClipId` is runtime-only
(`RoutingEngine` has no `toVar/fromVar`); `SessionRecorder::recordCuepointJump(clipId)`
has zero callers (gotcha 2026-05-21); `/api/composition` only reports ids.
Saved ids are therefore advisory, and the rule becomes:

> **On ANY load (composition or deck), every clip receives `s_nextClipId++`. `s_nextClipId` is the only minter, always.**

Why not "bump past max": (1) a file from an OLDER build with no `"id"` key
loads every clip as id 0 (`Clip.cpp:126` has no `hasProperty` guard) —
duplicates that a max-bump cannot fix; (2) hand-edited/merged files can
carry duplicates; (3) a DECK file appended into a composition whose other
decks are live can collide with THEIR ids — the liveness scan would then
refuse to close, or `openVideoForClip` would overwrite a live decoder.
Re-minting makes all three impossible and makes step 4 (OPEN NEW) safe
without L1-FU. The mint call MUST live in `MainComponent.cpp` (the static's
TU); the helper takes `uint32_t&` so it stays headless-testable.

Different/older build: ids irrelevant (re-minted); missing keys fall to
struct defaults via the existing `hasProperty` guards; structurally-empty
files are refused (§4). No version field exists in the format; none is
added (out of scope).

### Layer ids: bump `Deck::nextLayerId_` at the end of `Deck::fromVar`
`CompositorEngine`'s four per-layer GL maps are keyed by `layer.id`.
`nextLayerId_` (private, default 100, not serialized) resets on every
`Deck` constructed by `fromVar`, so post-load `addLayer()` re-mints an id a
LOADED layer already holds (e.g. file has layers 0,1,2,100,101 → next
`addLayer()` = 100) → two layers share a temporal/ring/output buffer →
feedback bleed. Fix, inside `Deck::fromVar` right after the `layers`
repopulation loop:
```cpp
for (const auto& layer : layers)
    nextLayerId_ = std::max(nextLayerId_, layer.id + 1u);
```
(`Deck.h` includes `<memory>`/`<vector>`/`<string>`/`<cstdint>` — add `<algorithm>`.)
CORRECTION to RECON TRAP #3(b): layer ids are already NOT unique across
decks today (`Deck::initDefault` gives EVERY deck layers 0,1,2), so
cross-deck sharing of those GL buffers is the status quo; this bump closes
only within-deck post-load reuse. Layer ids are not re-minted (they are
unique per deck in any file the app itself saved; out of scope for hand-
edited files — §7).

### Deck ids: bump `Composition::nextDeckId_` at the end of `Composition::fromVar` + `appendDeck`
Same shape after the `decks` loop: `nextDeckId_ = std::max(nextDeckId_, deck.id + 1u);`
(add `<algorithm>`). `deck.id` has zero runtime consumers (verified) —
this is hygiene plus what `appendDeck` (Step 3) mints from. Add to
`Composition` (public, next to `addDeck`):
```cpp
// Append a fully-formed deck (e.g. loaded from a deck file) under a fresh id.
// Returns its index. Caller is responsible for the GL fence (push_back reallocates).
int appendDeck(Deck deck)
{
    deck.id = nextDeckId_++;
    decks.push_back(std::move(deck));
    return static_cast<int>(decks.size()) - 1;
}
```

---

## 4. FAILURE HANDLING — VALIDATE-THEN-SWAP. Swap-and-rollback rejected.

Posture: every fallible operation (read, parse, validate, open media)
happens on a private `Composition incoming` BEFORE the fence. The swap
itself is `composition_ = std::move(incoming)` — implicitly-defined move
assignment over `std::vector`/`std::string`/`juce::File`/PODs (no model
struct declares a special member — verified `grep operator=|Clip(const`
across `Clip.h/Layer.h/Deck.h/Composition.h`), which cannot fail. So a
half-swapped state is UNREACHABLE: the model is either exactly the old one
or exactly the new one. Media-open failures (missing files) are per-clip
and non-fatal by existing convention (`handleClipTrigger`/`applyFileDrop`
tolerate them; Relocate Missing Files exists for the user).

Why not swap-and-rollback: rollback needs a SECOND fenced swap plus
re-opening the OLD comp's media (which a naive sequence would already have
closed) — doubling the moving parts exactly in the case (bad file, mid-set)
where the system is least trustworthy. A refused load with a one-line alert
and an untouched running set is the correct live-performance outcome.

### Validation rules — `src/core/CompositionLoad.h` (complete, header-only; the builder copies this)
```cpp
#pragma once
// Header-only, model-only (no renderer/UI) so tests/test_composition.cpp can
// drive it headless — same shape as core/MediaReconnect.h. Used by
// MainComponent's composition/deck load paths (L3, 2026-09).
#include "model/Composition.h"
#include <algorithm>
#include <cstdint>
#include <string>
#include <vector>

namespace compload
{
// Normalize + validate a freshly-deserialized Deck. "" = OK, else a human-readable
// refusal reason. REPAIRS (in place, on the staged copy only): numColumns >= 1 and
// >= every layer's clips.size(); every layer padded to numColumns.
inline std::string validateDeck(Deck& d)
{
    if (d.layers.empty())
        return "deck '" + d.name + "' has no layers";
    int cols = std::max(1, d.numColumns);
    for (const auto& l : d.layers)
        cols = std::max(cols, static_cast<int>(l.clips.size()));
    d.numColumns = cols;
    for (auto& l : d.layers)
        l.ensureColumns(cols);
    return "";
}

// REFUSES: no decks (an FX-preset .json, a deck .json or arbitrary JSON all parse fine
// but have no "decks" array); any deck without layers. REPAIRS: activeDeckIndex out of
// range -> 0. Runs validateDeck on every deck.
inline std::string validateComposition(Composition& c)
{
    if (c.decks.empty())
        return "not a composition file (no decks)";
    for (auto& d : c.decks)
        if (auto r = validateDeck(d); !r.empty())
            return r;
    if (c.activeDeckIndex < 0 || c.activeDeckIndex >= static_cast<int>(c.decks.size()))
        c.activeDeckIndex = 0;
    return "";
}

// Give EVERY clip a fresh id from `nextId` (the caller passes MainComponent's file-static
// s_nextClipId). Saved ids are advisory: nothing persistent references them, the renderer's
// media maps are keyed by id, and files from older builds / hand edits carry 0 or duplicate
// ids. Returns the number of clips re-minted.
inline int remintClipIds(Deck& d, uint32_t& nextId)
{
    int n = 0;
    for (auto& layer : d.layers)
        for (auto& cell : layer.clips)
            if (cell.has_value()) { cell->id = nextId++; ++n; }
    return n;
}
inline int remintClipIds(Composition& c, uint32_t& nextId)
{
    int n = 0;
    for (auto& d : c.decks) n += remintClipIds(d, nextId);
    return n;
}

// Ids of every clip that owns renderer-side media (Video / ImageSequence), sorted.
inline std::vector<uint32_t> playableClipIds(const Composition& c)
{
    std::vector<uint32_t> ids;
    for (const auto& d : c.decks)
        for (const auto& l : d.layers)
            for (const auto& cell : l.clips)
                if (cell.has_value() && cell->isPlayable()) ids.push_back(cell->id);
    std::sort(ids.begin(), ids.end());
    return ids;
}

// before \ after: the media ids a model mutation orphaned (what must be closed).
// Full swap / New -> all old ids. Deck append -> nothing. Inputs must be sorted.
inline std::vector<uint32_t> idsRetired(const std::vector<uint32_t>& before,
                                        const std::vector<uint32_t>& after)
{
    std::vector<uint32_t> out;
    std::set_difference(before.begin(), before.end(), after.begin(), after.end(),
                        std::back_inserter(out));
    return out;
}
} // namespace compload
```
(`<iterator>` for `std::back_inserter`.) The alert idiom is the file's own:
`if (!testMode_) juce::AlertWindow::showMessageBoxAsync(juce::MessageBoxIconType::WarningIcon, "Open Composition", "<file name>: <reason>");`
(`MainComponent.cpp` `if (!testMode_)` + `showMessageBoxAsync` at ~397 and in `loadPreset`'s "Some Mappings Dropped").

Deck file validation (Step 3): parse; `if (!parsed.getDynamicObject() || !parsed.getDynamicObject()->hasProperty("layers"))` → refuse "not a deck file"; `Deck incoming; incoming.fromVar(parsed);` → `validateDeck` → refuse or continue. (A composition file picked here has no top-level `"layers"` → refused; an FX preset likewise.)

---

## 5. THE SPLIT — three commits, in order; every intermediate state ships

### STEP 1 — model + helper + headless tests (no caller changes; pure additive)
Changes: `Deck::fromVar` bump; `Composition::fromVar` bump; `Composition::appendDeck`; NEW `src/core/CompositionLoad.h`; tests.
Tests to add to `tests/test_composition.cpp` (one `TEST_CASE` each — names are the gate):
1. `"Deck::fromVar bumps the layer-id mint past every loaded id"` — layers with ids {0,1,2,100,101} → `addLayer()` → `layers.back().id == 102`; and a default `initDefault()` deck still mints 100 (no behavior change).
2. `"Composition::fromVar bumps the deck-id mint past every loaded id"` — deck ids {0,100,250} → `addDeck()` → `decks.back().id == 251`.
3. `"Composition::appendDeck assigns a fresh id, keeps contents, returns the index"`.
4. `"compload::validateComposition refuses structurally-empty files and repairs indices/columns"` — SECTIONs: no decks → non-empty reason; a deck with zero layers → reason; `activeDeckIndex` 7 with one deck → 0; `numColumns` 0 with a 5-clip layer → `numColumns == 5` and every layer `clips.size() == 5`.
5. `"compload::remintClipIds gives every clip a unique monotonic id and advances the mint"` — two decks, ids all 0 and one duplicate pair; `nextId = 1000` → returns clip count, all ids unique, min ≥ 1000, `nextId == 1000 + count`.
6. `"compload::idsRetired is before minus after"` — {1,2,3}/{2,3,4} → {1}; superset → {}; disjoint → all; both empty → {}.
7. `"Composition saveToFile/loadFromFile round-trips through a real file and sets filePath"` — `juce::File::createTempFile(".json")`; save; load into a fresh `Composition`; deck count/names/clip names match; `loaded.filePath == file`; `file.deleteFile()`.
8. `"Composition::loadFromFile fails closed on a missing or non-JSON file"` — nonexistent → `false`; temp file containing `not json` → `false`; the target's `decks.size()` unchanged.
Gate: build exit code 0 FIRST (L1's false-green gotcha: ctest ran a stale binary after a failed build); `ctest` reports **213** (205 at HEAD, verified `ctest -N` in `build/`; +8) with 0 failures — state the number you observed, never inherit 213.
Shippable alone: yes — no caller changes; `fromVar` behavior for existing files is unchanged except the private mints.

### STEP 2 — Open / Save / Save As / Cmd+O / Cmd+S / browser comp callbacks / kCompNew re-base
Changes (all `MainComponent.h/.cpp` unless noted):
- `#include "core/CompositionLoad.h"` next to `#include "core/MediaReconnect.h"`.
- New private methods (declare in `MainComponent.h` next to `void savePreset(); void loadPreset();`):
  `void openComposition();` (FileChooser → `loadComposition(file)`), `void loadComposition(const juce::File& file);`, `void saveComposition();`, `void saveCompositionAs();`, `void swapCompositionModel(const std::function<void()>& mutation);`, `void refreshUiAfterModelSwap();`.
- `openComposition()`: house FileChooser idiom (copy `loadPreset()`'s shape verbatim): `fileChooser_ = std::make_unique<juce::FileChooser>("Open Composition...", CompDecksBrowser::getCompositionsDir(), "*.json"); flags = openMode | canSelectFiles; launchAsync([this](const juce::FileChooser& fc){ auto file = fc.getResult(); if (file == juce::File{}) return; loadComposition(file); });`
- `loadComposition(file)`: §1 sequence 1-7. Step 4's per-clip body is `applyFileDrop`'s video block and `applyMultiFileDrop`'s sequence block, applied to `incoming`'s clips:
  ```cpp
  auto& renderer = previewPanel_.getRenderer();
  for (auto& deck : incoming.decks) for (auto& layer : deck.layers) for (auto& cell : layer.clips)
  {
      if (!cell.has_value() || !cell->isPlayable()) continue;
      Clip& clip = *cell;
      if (clip.mediaType == Clip::MediaType::Video)
      {
          if (!clip.mediaFile.existsAsFile()) continue;
          if (renderer.openVideoForClip(clip.id, clip.mediaFile))
              if (auto* p = renderer.getVideoPlayer(clip.id))
              { clip.hasAlpha = p->hasAlpha(); clip.clipWidth = p->getWidth();
                clip.clipHeight = p->getHeight(); clip.thumbnail = p->getThumbnail(90, 72); }
      }
      else // ImageSequence
      {
          if (clip.sequenceFiles.empty()) continue;
          renderer.openImageSequenceForClip(clip.id, clip.sequenceFiles, clip.sequenceFps);
          auto first = juce::ImageFileFormat::loadFrom(clip.sequenceFiles[0]);
          if (first.isValid()) clip.thumbnail = first.rescaled(90, 72, juce::Graphics::lowResamplingQuality);
      }
  }
  ```
  (`Clip::thumbnail` is NOT serialized — `Clip.cpp` has no "thumbnail" property — so without this, loaded video/sequence cells are blank. Image clips need nothing: `ClipCell::setClip` loads the image file itself, `ClipCell.cpp` `if (clip_->mediaType == Clip::MediaType::Image && clip_->mediaFile.existsAsFile()) … ImageFileFormat::loadFrom`.)
- `saveComposition()`: `if (composition_.filePath != juce::File() && composition_.filePath.getParentDirectory().isDirectory()) { if (composition_.saveToFile(composition_.filePath)) { fileLabel_ "Saved: …"; browser refresh; } else alert "Save failed: <path>"; } else saveCompositionAs();` Silent overwrite (standard "Save"; RECON OPEN QUESTION 1 — settled).
- `saveCompositionAs()`: `auto dir = CompDecksBrowser::getCompositionsDir(); dir.createDirectory(); fileChooser_ = make_unique<FileChooser>("Save Composition As...", dir.getChildFile(juce::String(composition_.name) + ".json"), "*.json"); flags = saveMode | canSelectFiles | warnAboutOverwriting; (the enum value is `juce::FileBrowserComponent::warnAboutOverwriting` = 128 in this JUCE 8 checkout — verified `build/_deps/juce-src/.../juce_FileBrowserComponent.h`; there is no `warnAboutOverwritingExistingFiles` flag) launchAsync: result empty → return; saveFile = withFileExtension("json") like savePreset; if (composition_.saveToFile(saveFile)) { composition_.filePath = saveFile; composition_.name = saveFile.getFileNameWithoutExtension().toStdString(); fileLabel_; browserPanel_->getCompDecksBrowser().refresh(); } else alert.` (`saveToFile` is `const` and never sets `filePath` — only `loadFromFile` does — RECON was right; Save As must set it or a later Save cannot find it.)
- Menu cases (anchors: `case C::kCompOpen:\n            loadPreset();`, `case C::kCompSave:\n            savePreset();`, `case C::kCompSaveAs:\n            savePreset();`): → `openComposition();` / `saveComposition();` / `saveCompositionAs();`.
- **Key handlers (RECON MISSED THESE):** in `MainComponent::keyPressed`, anchors `// Cmd/Ctrl+S = save preset` … `savePreset();` and `// Cmd/Ctrl+O = load preset` … `loadPreset();` → `handleMenuCommand(AudioDNAMenuBar::kCompSave);` / `handleMenuCommand(AudioDNAMenuBar::kCompOpen);` (same dispatch shape as the `Cmd+X` block just above them). The menu items carry no KeyPress (`MenuBarModel.cpp` `menu.addItem(kCompSave, "Save", true, false)`), so the shortcuts are wired ONLY here; without this change Cmd+S saves the FX preset while the menu's Save saves the composition — the lane's headline defect surviving under its most-used gesture. Update the two comments. `savePreset()/loadPreset()` keep their row-1 button callers (`savePresetButton_.onClick`, `loadPresetButton_.onClick`, `MainComponent.cpp:32-33`) — those are the FX-preset surface and L-DEL's to delete; do not touch them.
- kCompNew: replace its four statements with `swapCompositionModel([this] { composition_.initDefault(); });` (keep the comment; append one line: "routed through the shared swap helper so New also closes orphaned media and re-points the inspectors").
- Browser wiring, right after `browserPanel_->setComposition(&composition_);` (`MainComponent.cpp:~1406`):
  ```cpp
  browserPanel_->getCompDecksBrowser().onCompositionLoad = [this](const juce::File& f) { loadComposition(f); };
  browserPanel_->getCompDecksBrowser().onCompositionSave = [this] { saveComposition(); };
  ```
  (`onCompositionSave` = menu Save semantics — existing path overwrites, else Save As dialog into the compositions dir. NOT the "Save Deck" button's silent `<name>.json` write: with the default name that would silently overwrite `Untitled.json` on every click.)
- `src/ui/CompDecksBrowser.h`: move `static juce::File getCompositionsDir();` and `static juce::File getDecksDir();` from under `private:` (lines 72-73) to the `public:` section. **RECON said these are public; they are not** (`private:` at line 44). No `.cpp` change.
Gate: build 0; ctest 213/213 (no new tests — nothing headless can drive `MainComponent`); fence check; app-launch health + §6 checklist (owner-attended: TCC mic prompt re-fires after every rebuild — gotcha 2026-07-25).
Shippable alone: yes — Open/Save/Save As/Cmd+O/Cmd+S/browser comp load+save/New all correct; the browser's Decks rows remain the silent no-op they are today (no regression).

### STEP 3 — browser Decks row: append-and-activate
- New private `void appendDeckFromFile(const juce::File& file);` (NOT `loadDeck` — that name is taken by the legacy PresetManager path, `MainComponent.h` `void loadDeck();`).
- Body: read+parse (`juce::JSON::parse(file.loadFileAsString())`); refuse if not an object with `"layers"`; `Deck incoming; incoming.fromVar(parsed);` `compload::validateDeck` → refuse on reason; `compload::remintClipIds(incoming, s_nextClipId)`; OPEN NEW for `incoming`'s playable clips (same loop as Step 2 — factor it as `void openMediaForDeck(Deck&)` and have `loadComposition` call it per deck); `if (incoming.name.empty()) incoming.name = file.getFileNameWithoutExtension().toStdString();` then
  `swapCompositionModel([&] { composition_.activeDeckIndex = composition_.appendDeck(std::move(incoming)); });`
  The guard inside `withDeckDetached` re-points the renderer at the NEW active deck; `idsRetired` is empty (nothing left) so nothing closes; `rebuildGrid()` rebuilds the deck tabs (`setupDeckTabs()` runs inside it — verified). `fileLabel_` + browser refresh as in Step 2.
- Wire: `browserPanel_->getCompDecksBrowser().onDeckLoad = [this](const juce::File& f) { appendDeckFromFile(f); };`
Why append, not replace-in-place (RECON's reading): a performer clicking a saved deck mid-set must not lose the deck they are on, and append never closes live media (zero outgoing ids) — it is the exact fenced `push_back` shape `AddDeckCmd` already proves. Repeated clicks accumulate decks, as Add Deck does; Remove Deck exists. History is cleared (one rule: every load clears).
Gate: build 0; ctest 213/213; fence; app: click a saved deck row → deck count +1, new tab active, `/api/composition` shows it, its clip ids ≥ 1000 and unique across the whole composition.
Deferrable: if the session runs short, stop after Step 2 — its end state is shippable.

Could the whole lane land as ONE commit? Technically yes, but Step 1 carries the only machine-provable gate and Step 2's review is materially easier against a green Step 1. Three commits.

---

## 6. THE FENCE — exact write list per step (reviewer checks `git diff --name-only` against this)

| Step | May write |
|---|---|
| 1 | `src/model/Deck.h` · `src/model/Composition.h` · `src/core/CompositionLoad.h` (NEW) · `tests/test_composition.cpp` |
| 2 | `src/MainComponent.h` · `src/MainComponent.cpp` · `src/ui/CompDecksBrowser.h` (two declarations move to `public:` — nothing else) |
| 3 | `src/MainComponent.h` · `src/MainComponent.cpp` |

No CMake change (header-only helper; `test_composition` already has `${SRC_DIR}` on its include path and links `juce_core/juce_events/juce_graphics`).
Explicitly NOT writable in any step: `src/render/Renderer.{h,cpp}` (L1-FU's fence — the adjacent `openVideoForClip` fix is THAT packet's), `src/render/CompositorEngine.*`, `src/core/ClipCommands.h`, `src/core/DeckCommands.h`, `src/core/UndoService.*`, `src/core/UndoManager.*`, `src/ui/CompDecksBrowser.cpp`, `src/ui/BrowserPanel.*`, `src/ui/TopBar.*`, `src/ui/PresetManager.*`, `src/ui/MenuBarModel.*`, `src/model/Clip.*`, `src/model/Layer.*`, any other test file, `CMakeLists.txt`, `tests/CMakeLists.txt`.

---

## 7. WHAT NOT TO DO — out of scope, including the tempting-while-in-the-file items

- **L1-FU** (`closeMediaForClip` inside `openVideoForClip`/`openImageSequenceForClip`): its own packet, `Renderer.cpp` is outside this fence. This lane does not need it (fresh ids never hit a live entry). Do not "just add the two lines while you're there".
- **CompositorEngine per-layer GL release** on model change (RECON TRAP #3): its own lane; needs a GL-thread-safe release path analogous to L1's retire list. Do not call `releaseGL()` from the message thread.
- **kCompNew's mutation semantics.** Keep `composition_.initDefault()` exactly; do not widen it to a full-struct reset (it deliberately leaves crossfader/transform/autopilot/P23/output fields alone; `outputDisplay` etc. have unknown consumers in L-OUT territory).
- **Cmd+Shift+S / menu KeyPress registration** for the comp items: new surface, not asked.
- **A dirty flag / "unsaved changes?" prompt** on New/Open/Quit. Tempting, new feature, not this lane.
- **Recent-files, autosave, async/threaded file loading** (large-file UI stall is accepted: the GL output keeps rendering the old comp meanwhile).
- **`TopBar` sync after load**: `TopBar` initializes `fadeSlider_` from `composition_.globalTransitionSpeed` at construction and its quantize/multiplier widgets write to the model but never re-read it; after a load those three widgets show STALE values until touched (the loaded values ARE in effect — the renderer reads the model). `TopBar` has no sync method and is outside the fence. Recorded follow-up: `TopBar::syncFromComposition()` (~10 lines) called from `refreshUiAfterModelSwap()`. Named in the human checklist as expected.
- **`ApiServer` un-marshaled `onTriggerClip/onTriggerColumn/onSwitchDeck`** (`MainComponent.cpp:~1678-1680`, no `callAsync`, unlike `oscHandler_`'s at ~1706-1710): pre-existing background-thread model mutation; a curl racing a load can crash. Not this lane's. Do not drive `/api/trigger*` during the load gate.
- **Layer-id re-minting** for hand-edited/older files with duplicate layer ids within a deck (shared GL buffers). The `fromVar` bump is the whole of this lane's layer-id work.
- **A format `version` field**, a custom `FilenameFilter` for the chooser (RECON OPEN QUESTION 2 — post-parse refusal is sufficient), Save-Deck button changes (`CompDecksBrowser.cpp` is outside the fence and already works).
- **The legacy `PresetManager::saveDeck/loadDeck` + `deckSaveButton_/deckLoadButton_` + `savePresetButton_/loadPresetButton_`** (row-1, L-DEL's). Untouched.
- **P23 UI** (`autoPresetOnGenre` etc.), the structural-scene handler body, `smartAutopilotEnabled`/`genrePresetNames` consumers. Parked per plan; RECON's correction (preset-by-name half also has zero readers) stands.
- **Fixing the recon's inventory of "three vacate paths"/"both layer loops"-style counts elsewhere.** Not this lane.

---

## 8. TRAPS

1. **`CompDecksBrowser::getCompositionsDir()` is PRIVATE.** Step 2 will not compile until the two statics move to `public:` (fence includes the header for exactly this).
2. **Cmd+S / Cmd+O are in `keyPressed`, not the menu.** Miss them and the shortcut still saves the FX preset.
3. **Never `composition_.loadFromFile(file)` on the live object** — wrong-shape files "succeed" with zero decks, and absent keys keep OLD values (every `hasProperty` guard). Stage into `incoming`.
4. **Thumbnails/dims go into `incoming` BEFORE the swap.** A post-swap write into a live `Clip` races the GL thread (`juce::Image` ref-count swap; `hasAlpha/clipWidth/clipHeight` read during compositing).
5. **Do not "simplify" `idsRetired(before, after)` to "close everything before".** Breaks deck-append (closes surviving decks' media) and blacks out the old comp before the cut.
6. **`s_nextClipId` is TU-local** to `MainComponent.cpp`; call `compload::remintClipIds(x, s_nextClipId)` from there. Never add a second minter.
7. **`withDeckDetached` is NOT reentrant** (`jassert(... || !fenceActive_)`). The mutation lambda must be the bare move/append/`initDefault()` — nothing that itself fences (`Deck::setClip` handlers, `handleFileDrop`, effect-stack edits) and not `handleDeckSwitch`.
8. **Null the inspectors FIRST** inside `refreshUiAfterModelSwap()`; `rebuildGrid()` and `refresh()` come after. The 10Hz timer is the enforcement.
9. **`ClipInspector`'s `EffectStackView` points at `&clip->effects`** — a vector INSIDE the dead model. `setClip(nullptr, …)` re-points it (`effectStackView_.setEffects(nullptr, EffectScope::none())`). `CompositionInspector`'s points at `&composition_.globalEffects` — stable address, needs `rebuildEffectStack()` for count only.
10. **Layers load with `activeClipColumn = -1`** (runtime fields are not serialized — verified `Layer.cpp` has no `activeClipColumn` key). After a load NOTHING plays until triggered. Expected. Do not auto-trigger to "fix" it. This is also why `refreshPreviewFromActiveClip` post-load lands in its `!foundActiveClip` branch and clears the old fallback — correct.
11. **`loadFromFile` sets `filePath`; `saveToFile` (const) does not.** Save As sets `filePath` (and `name`) itself.
12. **Renderer's `prevActiveDeckIndex_`**: a load whose `activeDeckIndex` differs from the current one triggers the P25 deck-transition crossfade from the last frame of the old comp (`globalTransitionSpeed` of the NEW comp). Cosmetic and intended.
13. **`FileChooser` callback captures raw `this`** — house idiom in this file (`loadPreset`, `kCompCollectMedia`, `kClipReplaceContent`); follow it, don't invent a SafePointer variant here.
14. **ctest false green** (L1 gotcha): a failed build leaves the previous test binary; check the build exit code BEFORE quoting ctest; the number must be 213.
15. **Deck file format** for Step 3 is `Deck::toVar()` (what the browser's "Save Deck" button writes to `~/Library/Application Support/AudioDNA/decks/<name>.json`) — NOT `PresetManager`'s `DeckState`, NOT a composition file. Both of the latter are refused by the `"layers"` check.
16. **Move-assignment of `Composition` destroys the OLD model on the message thread, inside the fence.** That is fine (model objects own only CPU state — `juce::Image` thumbnails, strings, vectors); GL/FFmpeg resources live in the renderer keyed by id and are retired separately. Do not move the old model out to "destroy it later" — it adds a second object to reason about for nothing.
17. **TSan noise**: the POD-scalar reads in `renderOpenGL()` (deck-transition/transform blocks) during any mutation pre-date this lane; a TSan run will report them for kCompNew too. Not new, not this lane's.
18. **`refreshUiAfterModelSwap()` must not call `resized()`** — the `kDeckNew`/`kLayerNew` handlers don't; `DeckView` lays itself out from `rebuildGrid()`.

---

## 9. HOW TO PROVE IT

### Machine can gate
- Build: `cmake -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build --config Release -j$(sysctl -n hw.ncpu)` → exit 0, 0 errors (checked FIRST).
- `ctest --test-dir build` → **213/213** after Step 1 (205 + 8 named TEST_CASEs above), unchanged through Steps 2-3. Tests 4-6 are the falsifiable oracles for §3/§4 (refusals, clamps, re-mint uniqueness+monotonicity, set-difference).
- Fence: `git diff --name-only <prev>..HEAD` ⊆ the step's list in §6, per commit.
- App health after Steps 2/3 (existing convention): `open build/AudioDNA_artefacts/Release/Audio-DNA.app`; poll `/api/health` on 7070 (~12s); zero crash markers; graceful teardown; `[MilkDrop] Loaded 30 presets` still present (cross-lane regression check L1 used).
- Scriptable load oracle (browser row, NOT the native chooser — `ax_press.py` cannot reach native file dialogs, HANDOFF loose end 6): plant `~/Library/Application Support/AudioDNA/compositions/L3-gate.json` (2 decks named `L3-A`/`L3-B`, a few clips, one video clip pointing at an existing file), open the Comp/Decks tab, synthetic-click the row (recipes in gotcha 2026-07-30), sleep ≥1s (do not poll DURING the load — `/api/composition` reads the model on the HTTP thread), then `GET /api/composition`: deck names == `["L3-A","L3-B"]`, every clip `id` ≥ 1000 and unique across all decks, `activeClipColumn == -1` everywhere. Then Cmd+S → file mtime changes and content re-parses.
- Crash oracle for RECON TRAP #1: name-bar-click a clip cell (Clip tab active) → load via the browser row → sleep 0.5s → `/api/health` still answers, no crash marker.

### Only Boris at the machine can see
- **Playback after reload**: build ≥2 decks with ≥2 video clips and one image sequence; Save As; File > New; File > Open the file; trigger the clips — video advances, sequence animates (proves OPEN NEW + `getVideoFrameTexture` under re-minted ids). The SEQ badge (HANDOFF "only Boris can check", 3 sessions unseen) becomes observable here for free.
- **No aliasing**: after the load, drop ONE new video into an empty cell; it plays ITS file, and the loaded video clips still play theirs.
- **Old comp gone**: have a MilkDrop/procedural Source clip ACTIVE, then Open a comp with none — output/preview go dark rather than keeping the old source (proves `refreshPreviewFromActiveClip` post-load).
- **Refusal**: Open an FX preset `.json` from `PresetManager::getPresetsDirectory()` → one alert, nothing changes, the running deck keeps playing.
- **Global FX rows**: Composition tab open, comp A has 3 global effects, comp B has 0 → after Open B the list is empty (RECON TRAP #2); after Open A it shows 3.
- **Save vs Save As**: Save on an unsaved comp prompts; after Save As, Save is silent; browser list shows the new name; inspector's composition label shows the file's base name.
- **Deck append (Step 3)**: click a saved deck row → new tab, active, its clips trigger and play; the previous deck's clips still play when you switch back.
- **Expected stale (recorded, §7)**: TopBar fade slider / quantize / multiplier widgets show pre-load values until touched.
- Optional: run the load cycle ~20× with video-heavy comps under `build-asan` and watch process memory — flat-ish (decoders retired) is pass; a steady climb is the `CompositorEngine` per-layer leak (§7) and is evidence for that lane, not a failure of this one.

---

## 10. CORRECTIONS — where this packet overrides the RECON or the plan

1. **`getCompositionsDir()/getDecksDir()` are private** (`CompDecksBrowser.h:44` `private:` … `:72-73`), not "already public" (RECON FILES TOUCHED #1/#5, FENCE). `CompDecksBrowser.h` enters Step 2's fence.
2. **Cmd+S / Cmd+O** are hard-wired to `savePreset()/loadPreset()` in `MainComponent::keyPressed` — RECON's "three menu cases" is an undercount of the surfaces on the wrong object (5, not 3: 3 menu cases + 2 key handlers; the 2 row-1 buttons stay by design).
3. **Clip ids: re-mint all on load**, not "bump `s_nextClipId` past the max loaded id" (plan + RECON) — the bump fails on older/dup-id files and on deck-append into a live composition.
4. **Reconnect happens BEFORE the swap** (RECON THE CHANGE 3e put it after) — thumbnail/dim fill into live clips would race the GL thread, and the output should stay on the old comp during FFmpeg probes.
5. **Close-old happens AFTER the swap, by set difference** (RECON 3c: before, all). Same memory safety; no pre-cut blackout; correct for deck-append.
6. **The fence's cost is an output blackout, not a GL-thread block** (RECON 3d rationale). The GL thread renders deck-less during the mutation; the message thread blocks once on the empty GL job.
7. **`Clip::thumbnail` is not serialized** — RECON's reconnect step ("reuse `makeClipMediaHook`'s body") would leave video/sequence cells blank; this packet mirrors `applyFileDrop`/`applyMultiFileDrop` instead.
8. **Layer ids already collide across decks today** (`Deck::initDefault` → 0,1,2 in every deck); RECON TRAP #3(b)'s "newly loaded layer can inherit an old layer's GL resource" is the status quo for every second deck — the `fromVar` bump closes within-deck post-load reuse only.
9. **`onDeckLoad` = append-and-activate**, not "load into the active deck slot" (RECON step 7 / TRAP #7 / OPEN QUESTION 3).
10. **kCompNew also leaks the old comp's media** (never named) — fixed by routing it through `swapCompositionModel`.
11. **The P21 persistent-layer loop** (`Renderer.cpp` `for (auto& otherDeck : composition_->decks)`) walks the decks vector outside `activeDeck_` — RECON did not name it; it is nested under `deckActive`, so the existing fence covers a whole-composition swap. Load-bearing for the claim "withDeckDetached is sufficient".
12. **`swapCompositionModel` is mandatory, not "recommended"** (RECON FILES TOUCHED #1 / OUT OF SCOPE last bullet) — leaving kCompNew on its own path would ship a known crash beside its fix.
13. HANDOFF's `ctest 205/205` — confirmed (`ctest -N` → 205). RECON's 17-command count — confirmed by member enumeration, not just class count.
14. Design decision not in either doc: **composition `name` = file base name on Load and Save As** (New still resets to "Untitled"); `CompositionInspector` shows `name` read-only and Collect Media names its folder from it — this keeps browser row, inspector label and Collect Media folder agreeing. One line each; reversible.

## 11. OPEN QUESTIONS (for Boris; defaults chosen so the builder is not blocked)
- Q1 Cmd+S/Cmd+O → composition (default YES; the FX-preset "Save"/"Load" row-1 buttons remain until L-DEL).
- Q2 Decks row appends (default YES) vs replaces the active deck.
- Q3 Name-from-filename rule (default YES).
- Q4 TopBar stale widgets after load: accept for now (default) or pull the ~10-line `TopBar::syncFromComposition()` into a Step 4 with `src/ui/TopBar.{h,cpp}` added to the fence.
