# Live Performance Controls Reference

> Moved from CLAUDE.md (claudemd-split). Layer Router, Per-Type Autopilot, Live Performance Controls (P20-P21).

---

### Layer Router System (P20)

The Layer Router source (`layer_router`) lets one layer use another layer's rendered output as its input texture. This enables feedback loops, picture-in-picture, and cross-layer effects.

**Architecture**:
- `CompositorEngine::compositeShow()` saves each layer's final clip texture (after effects, transform, before keying/blending) into `layerOutputTextures_` keyed by layer ID
- When a clip has `sourceType == "layer_router"`, `Renderer::renderSource()` intercepts it, reads the `u_src_layer` param to determine which layer index to read, and returns the saved texture
- The "Source Layer" param maps [0,1] to layer indices 0-9
- Self-reference safety: if a layer routes to itself, it gets the previous frame's output (one frame delay). Circular references between two layers produce feedback effects.
- Every layer of the shared stack publishes its output, whatever deck the grid shows (lane bf9b; the router reads `composition_->layers` only inside the frame's `deckActive` gate).

---

### Per-Type Autopilot (P20)

Composition-level automation that sets different beat timings per layer type:
- **Opaque layers**: cycle every N beats (default 16)
- **Transparent layers**: cycle every N beats (default 8), optional randomization
- **FX Only layers**: cycle every N beats (default 4), optional randomization
- Config in `Composition::PerTypeAutopilotConfig`, UI in CompositionInspector "Per-Type Autopilot" section

---

### Live Performance Controls (P21)

**Binding System Extensions**:

- `Binding::TriggerMode` — `Toggle` (default, press to toggle) or `Momentary` (held = active, release = deactivate)
- `Binding::CCMode` — `Absolute` (0-127 → 0-1) or `Relative` (< 64 = decrement, > 64 = increment, for endless encoders)
- `Binding::TargetMode` — `ByPosition` (survives reorder), `ThisItem` (follows clip by ID), `Selected` (current UI selection)
- `Binding::velocityToOpacity` — maps MIDI velocity to clip opacity on trigger
- New actions: `AdjustLayerOpacity`, `LayerTransport` (play/pause toggle), `ToggleEffectBypass`, `AdjustMacro`
- `GlobalStop` ("Stop" in the bind overlay) stops every running and waiting routine and nothing else, exactly like the TopBar Stop (`[]`, "Stop all routines") — no clip stop or rewind and no audio-transport stop; the Play / Pause binding still stops a playing audio file (s-rta-0926b).
- `TriggerRoutine` fires a routine whose own restore style (`Routine::restoreStyle`: Ease, the default, glides its knobs onto the bar; Jump cuts to them on the bar — `POST /api/routine/set {"restoreStyle": "ease"|"jump"}`) applies at its start, every loop return and a restart (s-rta-0926b; `docs/claude/recording.md`). The same routine fires from its pad in the deck's ROUTINES row (a press = fire/restart, never stop); a Momentary binding's release still stops it (s-rta-0927; `docs/claude/recording.md` "Surfaces").

**Decks are boxes of clips** (lane bf9b, s-rta-1002b; Boris: "treat the decks as just a box of clips and I can switch between 20 decks looking for a clip and the playing will not be affected" / "Whatever is in the layer should be what is playing and there should be nothing else"):
- **The model**: `Composition::layers` is ONE shared layer stack (each layer's settings + its trigger tuple; no clips). `Composition::decks` are boxes: `Deck::rows[i]` (`ClipRow`) holds the clips of shared layer i, and every deck always has exactly `layers.size()` rows. A layer names its clip by `ClipRef` = (deck id, column) in each tuple slot -- active, previous, pending (Pitfall 63). What layer i plays is `Composition::playing(i)`, never the shown deck's row (Pitfall NN); `forEachLayer` walks what plays, `forEachClip` every clip of every box. `CompositorEngine::compositeShow` composites the shared stack once per frame; GL history is keyed by the shared layer (Pitfall 35).
- **A deck switch changes only the grid**: `MainComponent::handleDeckSwitch` (every entry -- a tab, a strip badge, REST `switch_deck`, OSC `/audiodna/deck/{n}`, bindings, a replayed activeDeck lane) sets `activeDeckIndex`, the renderer's fence token and `DeckView::showDeck` (the strips stay, the cells are re-pointed). No playing clip, position, speed, fade, effect or queued trigger changes, and nothing plays unseen: there is no off-screen deck clock, no per-deck autopilot and no deck-to-deck fade any more (the TopBar has no "Fade:" control). The genre deck auto-switch is inert (`docs/claude/effects.md`).
- **Firing**: a cell of ANY deck fires into its row's shared layer (`handleClipTrigger` resolves the cell on the given or the shown deck, `Composition::fire`); the same column from another deck is a new clip (a crossfade, not a retrigger). A column fire (`handleColumnTrigger`, `Composition::triggerColumn`) fires the SHOWN deck's column into every layer without Ignore Column Trigger; a layer whose cell in that column is empty goes empty, also when it was playing another deck's clip (Boris Q5's default). Bindings: ByPosition fires the shown deck's cell; Selected = the first shared layer playing a clip, fired from that clip's own deck (a removed deck: nothing); ThisItem searches every live deck for its clip and fires it from its own deck (its velocity lands on that clip). A momentary release releases exactly the refs its press fired (`MainComponent::releaseMomentaryRefs`).
- **Queued triggers survive a switch** and fire on their beat; "leaving a deck cancels its queue" (L5) is gone from every path. Only Remove Deck cancels the queues INTO the deck it removes.
- **Autopilot**: one for the show (Pitfall 38); a layer advances within the deck its playing clip came from (its SOURCE deck); a removed source does not advance.
- **A clip that leaves its layer** (replaced or cleared: no layer's active or previous ref names it) is not drawn, so nothing advances it -- its playhead freezes, its decode thread parks and its ring trims after 1 s; its playing flag is left as it was. Fired again it RESUMES from where it stopped (`MainComponent::syncActivatedPlayhead` makes the model agree with the player, contract C3); a retrigger of the active cell restarts at the in-point; a beat-snapped clip seeks to the beat phase; a never-played clip starts at its in-point.
- **Remove Deck while one of its clips plays** (Boris Q1's default: it keeps playing until replaced): the deck moves to `Composition::retiredDecks_` -- not shown, not saved, not indexed, its id stays reserved -- while any layer's active OR previous ref names it (a fade out of it finishes); queued triggers into it are cancelled. A retired deck no ref names any more is reaped inside the next fenced edit (`UndoService::withDeckDetached` calls `Composition::reapRetiredDecks`; a hook disposes its media), never on a timer (a reap needs a fence, which holds the canvas 1-2 frames). Undo of Remove Deck moves the retired deck back, its clips still playing (a reaped one comes back from the snapshot). The undo hint names the layers still playing: `Undo Remove "Deck 2" -- Layer 2 keeps playing its clip`.
- **On screen**: each layer strip's thumbnail carries a small opaque badge, bottom-left, clear of the routine bands: the tab number of the deck its clip came from -- dim when that deck is the one shown, normal text when another, "x" for a removed deck, none when the layer is empty or folded; its tooltip names the deck ("From deck '<name>' (tab N)" / "From a removed deck"); a click on it shows that deck (never selects the layer). A deck tab shows a dot while some layer plays, or fades out of, one of its clips. A cell is lit only on the deck its playing clip came from; the column header is lit only on the deck the column was fired from. Badge and dots re-read the model on the strip / main 30 Hz timers, compare-before-set, repainting only what changed (Pitfalls 41, 57).
- **Saving** keeps the boxes (every live deck's rows) and the shared layers' settings; retired decks and what plays are not saved -- after opening a show every layer starts empty, as before. The saved shape: a top-level `"layers"` array (settings) + `"decks"` whose rows hold only `"clips"`; `"persistent"` and `"globalTransitionSpeed"` are never written.
- **Old shows** (no top-level `"layers"`) are converted by `src/model/ShowMigration.h`: the shared stack has as many layers as the deck with the most rows; layer i's settings come from the FIRST deck (file order) that has row i; every deck is padded; colliding layer ids are re-minted. ONE note, logged once (`old show converted: ...`), names every dropped row whose settings differed (with the layer effects / connections it carried), every layer whose file said `"persistent": true` and a dropped deck fade (>= 2 decks); nothing dropped = no note. The note shows in the yellow load notice in the header row, beside the audio-device notice ("Old show converted -- layer looks now come from the first deck (hover for details)"; the tooltip is the whole note; cleared by the next save, load or New, or a click; never takes focus) -- the same label shows `Composition::routineLoadNote` and the deck-id refusal (Pitfall 36). Load Deck reads a deck file per row: a row with `"type"` is a legacy row whose settings apply only when it ADDS a shared layer; a new row adds a default layer; a deck with more rows than the show has layers adds layers (Boris Q2's default).
- **What an old show's per-deck data becomes**:

| store | lives on | at conversion |
|---|---|---|
| layer settings incl. layer effects, layer scalar connections, per-layer autopilot settings | the Layer | the winning (first) deck's row keeps its own; a dropped row loses its own (the note names it when it differs) |
| ByPosition / Selected bindings | (layer index, column) on the shown deck | no migration |
| ThisItem bindings | a clip id | no migration (now searched in every live deck) |
| routine / take keys | deck index + name, layer index + name | Layer scope resolves the shared layer by position / name; Clip scope as before, on rows |
| PerfState v1 (no `"layers"`) | per deck | shared layers restored from its captured active deck only |
| PerTypeAutopilotConfig | Composition | untouched |

- **Persistent layers: removed** (2026-10-02, Boris: "I want to remove the persistent"). A file's `"persistent": true` is read only by the converter, to name it in the note; the only "keep this layer" control is **Ignore Column Trigger** (`Layer::ignoreColumnTrigger`, the Layer tab's Layer section; `Composition::triggerColumn` skips it).
- Guards: `tests/test_show_model.cpp` (T1-T16, M1-M7), `tests/test_layer_strip_source_deck.cpp` (badge, dots, grid); live: `.harmony/probe-boxes.sh` (K1-K10).

**Deck tab row**: '+' = New Deck / Load Deck...; right-click a tab = Save Deck / Save Deck As... / Rename Deck... / Duplicate Deck / Remove Deck (the menu is headed by the deck's name; Remove shows a 10-s `Undo Remove "<name>"` button flush right in the row, no dialog); the Deck menu mirrors every action for the active deck. The Compositions browser tab is the library (row click = open / append as a new tab; right-click = Open / Show in Finder / Delete... to the Trash, confirmed). `DeckView::DeckTabButton` intercepts `isPopupMenu()` in `mouseDown` because a JUCE Button fires `onClick` on ANY mouse button (a right-click used to switch decks). Geometry and menus: `src/ui/DeckTabRow.h` (pure, `tests/test_deck_tab_row.cpp`). (Moved from CLAUDE.md's UI Patterns, s-rta-0926b canvas merge.)

**Beat Snap Granularity**: `Clip::BeatSnapMode` enum (Off, Beat, Bar, TwoBar, FourBar). `Layer::processPendingTrigger(beatInBar, barCount)` now checks the snap granularity before firing queued triggers. A queued trigger lives in the layer's tuple word (`LayerRuntimeCell`, Pitfall 63): a cancel (clear, momentary release, a Remove Deck of the deck it was fired from) and the beat's fire are both compare-exchanges of that word, so a cancel can never be outrun by a beat; the GL's fire tries at most 16 times, then waits for the next qualifying beat (lane tsan, s-rta-1002). A deck switch never cancels a queued trigger (lane bf9b).

**Ableton Link**: Optional (`-DAUDIODNA_BUILD_LINK=ON`, which defines `AUDIODNA_HAS_LINK=1`). `LinkSync` class wraps `ableton::Link`, updates cached BPM/phase via atomics. The only way to switch it on is the TopBar "Link" toggle (no REST, OSC, MIDI-binding or menu route; the on/off state is not saved in settings, presets, compositions or takes, so every launch starts with Link off). When enabled, MainComponent's ~30 Hz timer re-sends Link's tempo (`applyTempoCommand("link", bpm, Human)`): manual mode on + `BPMTracker::followExternalTempo()`.
- **Tempo only, never the phase** (s-rta-0926b bpm2, LINK-RAMP ruling b): a Link tempo, changed or unchanged, never realigns the beat phase -- it only changes the rate the phase runs at, so a peer's tempo ramp gives a continuous phase (`test_bpm_stabilization` "[link]"). Following Link's beat/bar phase is **not implemented**: `LinkSync::getBeatPhase()` is kept up to date but nothing reads it, so the app's beats are not aligned to other Link peers' beats. Tap and Resync realign; a REST/OSC/typed `set_bpm` never does (s-rta-0926b plan3 A).
- **Default build (`AUDIODNA_BUILD_LINK` OFF, `CMakeLists.txt`)**: Link is not compiled in and is honestly unavailable. `LinkSync::isAvailable()` is false, `setEnabled(true)` is ignored (`isEnabled()` stays false) and `getBPM()` returns 0, so the timer never feeds a tempo and nothing can force manual mode at a made-up 120 BPM (`test_link_sync`). The TopBar toggle keeps its place but is disabled and dimmed (0.4 alpha), with the tooltip "Ableton Link is not included in this build" (`test_topbar_link_toggle`).

**Key-up routing**: `MainComponent::keyStateChanged()` polls all momentary-bound keys and fires release actions. MIDI note-off already routed through `BindingManager::processMidiNoteOff()`. A momentary release calls `Layer::releaseMomentary(column)` (one CAS, no pre-check): the clip it started is cleared, and a release BEFORE its quantized beat cancels its own queued trigger -- the pad never latches on (lane tsan, ruling amendment 10).

---
