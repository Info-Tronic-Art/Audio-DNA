# Builder plan — routines display, slice A (Audio-DNA, main @ 4b0c39a, code = 6fe8abb)

Architect (Fable), 2026-09-27, s-rta-0927 lane R. Every `file:line` below was read on disk today at `4b0c39a`;
VERIFIED unless marked INFERRED / ASSUMED. The adopted design is `.harmony/.reports/s-rta-0926b/routine-ux/design-final.md`
(+ ADDENDUM) with `mockup.html` as the visual reference (12 states; "Still your call" 1-8, defaults ship). Where the
design's line numbers are stale (the engine changed after it was written: glide + Ease/Jump shipped in
`lane/routines-followup`, merged `04e31bb`), THIS document's numbers win.

QUESTION: What exactly does a Builder change, in what order, with what RED tests and what live evidence, to put the
routine pads above the column numbers, name bands on every layer a routine plays, a following V fader with a cyan
routine cue, and a pad settings menu (Loop/Once, Restore first/Start from now, Start: Ease/Jump, Quantize, Rename,
Remove from layers, Delete) on screen — without touching the outputs lane's code or the timing lane's probes?

APPROACH (stated first): additive engine status (deck / layers / fireSeq / startsOn / restartPending + a persisted
last-run warning) + one new `stopOnLayer`; ONE pure, juce_core-only view model (`RoutineDeckView.h`) derived from
`RoutineEngine::Status` every 30 Hz tick and pushed into `DeckView` (a new 22 px ROUTINES row of 8 custom-painted
`RoutinePad`s) and each `LayerStrip` (bands painted over the thumbnail top, x = whole-routine `stop(slot)`); the
`LayerStrip` timer pulls opacity/speed from the model (the real bug the design found) and turns the V fill cyan while a
lane-rank hand grips opacity; the Record tab loses its stop-shaped pad row; everything wires through the EXISTING
`perfRoutine*` funnel (`MainComponent.cpp:5675-5750`) — the Ease/Jump setting reuses `RoutineSetOpts::restoreStyle`
(`src/api/ApiServer.h:145-151`) and `Routine::restoreStyle` (`src/model/Routine.h:27-28`), both shipped. Seven
commits, each independently green; the UniversalParamControl "ROUTINE" cue (design slice B, pulled into A by the
birth prompt) is the LAST commit so it can be deferred alone.

---

## 1. Files and the fence

### 1.1 Concurrent lanes (why the fence is shaped this way)
- Lane O (plan5 C1, worktree `rta0927-w1`, branch `lane/outputs-c1-0927`) — its fence (`plan5-final.md:598-604`):
  NEW `src/output/*`, `src/ui/OutputWindow.h/.cpp`, `src/render/Renderer.h/.cpp`, `src/MainComponent.h/.cpp` (ctor
  output-window call, the 11+1 `outputWindow_->loadImage` fan-out deletions at `MainComponent.cpp:728-729,1545,2669,
  3573,3895,3935,3965,4350,4560,4620`, dtor `:2305-2329`, live-count calls in `openOutputOnDisplay`/`closeOutput`
  `:3772-3801`), `src/test/TestServer.h/.cpp`, `src/api/ApiServer.cpp` (state field), `CMakeLists.txt`,
  `tests/CMakeLists.txt`, its own tests/probe/docs.
- Lane T (routine timing flake, worktree `rta0927-w2`) — probes only (`.harmony/probe-routines.sh` and friends).

### 1.2 This lane's fence (lane R, worktree `.claude/worktrees/rta0927-w3`, branch `lane/routine-display-0927`)
NEW: `src/ui/RoutineDeckView.h`, `src/ui/RoutinePad.h`, `src/ui/RoutinePad.cpp`, `tests/test_routine_deck_view.cpp`,
`tests/test_routine_pad_press.cpp`, `tests/test_layer_strip_follows_model.cpp`, `tests/test_record_panel_pads_removed.cpp`,
`tests/tool_routine_deck_snapshot.cpp`, `.harmony/probe-routine-display.sh`, `.harmony/probe-routine-display.json`,
`.harmony/.reports/s-rta-0927/routine-display.md` (+ `routine-display-shots/`).
EDIT: `src/recording/RoutineEngine.h/.cpp`; `src/ui/DeckView.h/.cpp`; `src/ui/LayerStrip.h/.cpp`;
`src/ui/RecordPanel.h/.cpp`; `src/ui/RoutineBankModel.h`; `src/ui/UniversalParamControl.cpp` (C7 only);
`src/MainComponent.h` (two declarations next to `:549-550`); `src/MainComponent.cpp` — ONLY these functions/hunks:
  (a) the constructor: one line inside `deckView_->onLayerClearClip` (`:746-776`); one NEW contiguous block right after
      `routineEngine_.dispatch.notify` (`:2034-2038`); the three Record-panel routine wiring lines `:1571-1572,1581`
      (deleted in C5);
  (b) `timerCallback` (`:3693-3750`): one block appended at its end;
  (c) `routineStatusVar` (`:5752-5808`): additive keys;
  (d) NEW `renameRoutine(int)` / `deleteRoutine(int)` placed immediately after `perfRoutineRemove` (`:5736-5750`).
  NOT touched: the destructor, `resized`, `keyPressed`, `handleMenuCommand`, `onClipSelected` (`:711-745`, has a
  fan-out site at `:728-729`), `handleClipTrigger`/`handleColumnTrigger`, `handleDeckSwitch`, `openOutputOnDisplay`/
  `closeOutput`/`refreshDisplayList`, `tickFeaturePipeline`. No `outputWindow_` line is read or written by this lane.
`CMakeLists.txt`: one line `src/ui/RoutinePad.cpp` inserted directly after `src/ui/DeckView.cpp` (`:325`), never at
the list's end (lane O appends `src/output/*` there). `tests/CMakeLists.txt`: the new targets go directly after the
`test_routine_bank_model` block (`:1454-1476`), never at EOF (lane O appends at EOF). `src/api/*`: untouched (no new
route; `/api/routine/status` gains keys through `routineStatusVar` in MainComponent, `handleRoutineStatus` is a pass-
through). `.harmony/probe-routines.sh`: untouched (lane T).
Docs (C6): `CLAUDE.md` (UI Patterns + pitfall index 40 + one phrase in Key capabilities), `docs/claude/pitfalls.md`
(pitfall 40), `docs/claude/recording.md` ("Surfaces" bullet `:71-75`), `docs/claude/performance-controls.md` (one
sentence near `:39-40`), `.harmony/APP-INVENTORY.md` rows 58, 59, 85 and the ctest count (`:31`, re-run `ctest -N`).
Rig rules (`.harmony/HANDOFF.md:40-52`): live lock, `open -g` only, Quartz window-id captures only, no synthetic input,
TEMPORARY env-var hook reverted + rebuilt + `strings … | grep -c AUDIODNA_DEBUG_` = 0 before the final commit.

---

## 2. Surfaces (layout, colours, states, hit targets, paint, what each input does)

Palette (mockup brief; hex from shipped code where it exists): background `#1a1a1a` (`DeckView.cpp:27`), pad idle
`#333333` (`LayerStrip.h:124`), pad empty / trigger `#2a2a2a` (`DeckView.cpp:360`), hairline `#1a1a1a`, teal
`#4a9a8a` (`ClipCell.cpp:7` kActiveBorder), cyan `#00e5ff` (`LookAndFeel.h:24` kAccentCyan), text `#e0e0e0`, label
`#888888`, divider `#3a3a3a`, warning `#cc3333` (`ClipCell.cpp:207`). No radius, no shadow, no gradient, flush,
1 px hairlines, no hover-revealed anything, whole words in every tooltip (`docs/claude/architecture.md:305-307`).

### 2.1 ROUTINES row — `DeckView` (new; `src/ui/DeckView.h/.cpp`)
Geometry. `kRoutineRowHeight = 22` next to the constants at `DeckView.h:124-130`. `resized()` (`DeckView.cpp:30-74`)
removes the routine row FIRST (`area.removeFromTop(kRoutineRowHeight)`), then the trigger row as today. Corner cell
(0,0,250,22) is painted in `DeckView::paint` (`:25-28`, today only `fillAll`): "ROUTINES" 10 pt bold `#888888`
`Justification::centredLeft` in (6,0,244,22); the corner note (9 pt regular `#888888`, e.g. "· Drop on B") follows the
label's measured width + 4 px, ellipsis. Pads: 8 `RoutinePad`s at (`kLayerStripWidth + i*kCellWidth`, 0, 90, 22) —
the same x as the column triggers (`:38-44`) so pad N sits over column N. Created ONCE in the `DeckView` constructor
(`:3-23`) with `addAndMakeVisible` — direct children of `DeckView`, not of `gridContent_`, so `rebuildGrid`
(`:82-220`, which clears strips/cells/triggers/tabs and `gridContent_->removeAllChildren()` `:88-92`) never destroys
them. `getNaturalHeight()` (`:286-300`) adds `kRoutineRowHeight`; `MainComponent::resized` reads it (`:2547-2559`)
and grows the deck area by 22 px (INFERRED: `kMinBottomHeight` guards the bottom panels; verify at 1728x1117 and
1280 wide).

`RoutinePad` (`src/ui/RoutinePad.h/.cpp`): `class RoutinePad : public juce::Component, public juce::SettableTooltipClient`
(the `ClipCell.h:14` idiom; the app's `tooltipWindow_` `MainComponent.h:394` shows it). Members: `int slot_`,
`RoutineDeckView::Pad spec_`; `void setSpec(const RoutineDeckView::Pad&)` (stores, `setTooltip` when the text changed,
`repaint()` when any field changed); callbacks `std::function<void(int)> onFire, onContextMenu`. THERE IS NO STOP
CALLBACK MEMBER — a compile-level pin (see 5.4). Custom `paint()` (a `TextButton` would clip at the LAF's fixed 14 pt,
`RecordPanel.cpp:432-434`):
1. fill: Empty `#2a2a2a`, else `#333333`;
2. Playing: sweep `#4a9a8a` at 30 % alpha over (0,0, round(90·progress01), 22); when `barsTotal <= 8`, 1 px `#1a1a1a`
   vertical ticks at x = round(90·k/barsTotal), k = 1..barsTotal-1;
3. frame: Empty/Idle 1 px `#1a1a1a`; Waiting 1 px `#4a9a8a`; Playing 2 px `#4a9a8a`;
4. left text at x = 4: if `warning`, a 12x12 slot with "!" 10 pt bold `#cc3333` (`ClipCell.cpp:207-212` idiom),
   then the number at x = 20; else the number at x = 4. Number: 9 pt bold `#888888` (alpha 0.4 when Empty), 8 px wide
   (single digit). Name: 10 pt `#e0e0e0`, starts 4 px after the number, `drawText(..., useEllipsesIfTooBig = true)`,
   width = (right slot x) - 4 - start;
5. right slot, right inset 4: Playing → "5/8" (`bar`/`barsTotal`) 9 pt monospaced
   (`juce::FontOptions(juce::Font::getDefaultMonospacedFontName(), 9.0f, juce::Font::plain)`, ctor VERIFIED
   `juce_FontOptions.h:76`) `#e0e0e0`; Idle → "LOOP" 8 pt `#888888` when `loop`; Waiting/Empty → nothing. (The
   key/MIDI tag is slice B.)
6. Off-deck (Waiting/Playing with `onShownDeck == false`): the WHOLE paint inside `g.beginTransparencyLayer(0.5f)` /
   `endTransparencyLayer()` — frame, sweep, digits, text all at 50 %.
Constant text per state; the only per-tick change while playing is the sweep and the digits (once per bar).

Input → engine (each through a DeckView callback → MainComponent → the funnel):
| Input | Pad state | Effect |
|---|---|---|
| left mouseDown (press, not click — lowest latency, like a pad) | Idle / Playing / Waiting | `onFire(slot)` → `perfRoutineFire(slot)` → `RoutineEngine::fire` (`RoutineEngine.cpp:557-648`): Idle fires; Playing restarts at the next boundary (`:575-607`); Waiting is a no-op (`:580`) |
| left mouseDown | Empty | nothing (`spec_.state == Empty` guard) |
| right-click / Ctrl-click (`e.mods.isPopupMenu()`, the `DeckTabButton` idiom `DeckView.h:100-106`) | any non-Empty | `onContextMenu(slot)` → `DeckView::showRoutinePadMenu(slot)` (2.3) |
| key / MIDI / OSC / REST | — | unchanged: `Binding::Action::TriggerRoutine` (`MainComponent.cpp:7252-7259`, Momentary release = `perfRoutineStop`), OSC `:2148-2151`, REST `:2079-2080` |

Tooltips (per `RoutineDeckView::Pad.tooltip`, composed in the pure model, 5.1): Empty "No routine is saved on this pad
yet. Save one in the Record tab."; Idle "Press to play. Right-click for settings."; Waiting "Starting on the next
bar." / "… next beat." / "… next two-bar line." / "… next four-bar line." from the EFFECTIVE grid (`Slot.startsOn`,
3.1) or "Starting now." for "now"; Playing on the shown deck "Playing on Layer 1, Layer 3. Press to restart from the
top." (layer NAMES of the shown deck, fallback "Layer N"); Playing off-deck "Playing on Deck B. Switch decks to see
its layers."; a warning prepends "2 settings could not be restored; 1 timeline points at a layer or clip that no
longer exists; 3 clip changes could not be made. " (only the non-zero clauses, singular/plural, then a blank).

Corner note (`RoutineDeckView::cornerNote`): every pad Empty → "· Save one in the Record tab"; else, for each deck other
than the shown one that has a Waiting/Playing routine → "· Drop, Build on B" (names joined ", ", decks joined "; ",
deck names from `Deck::name` `Deck.h:15`); else "".

### 2.2 Routine bands — `LayerStrip` (new paint + hit test; `src/ui/LayerStrip.h/.cpp`)
Geometry at the shipped 250x96 strip (`LayerStrip.cpp:594-675`: `mainH = 76`, `leftColW = 78`, sliders at x 78/100/122,
`thumbnailBounds_ = (144, 0, 76, 76)` `:656,:662`, F at 220). Bands are painted right after the thumbnail block
(`:497-514`), before the transport display, inside `thumbnailBounds_`: band k (k = 0,1) = (144, 16k, 76, 16). Draw at
most TWO (`bandsToDraw`, 5.1): with three or more, band 2's label is "Build +1". Bands are drawn only when
`thumbnailBounds_.getHeight() >= 48` — a folded row (`Layer::folded` `Layer.h:70`, `kFoldedHeight = 22`
`DeckView.cpp:310`) shows none; the pad carries it.
Paint per band: backing black 70 % (`Colours::black.withAlpha(0.7f)`); for k = 1 a 1 px `#1a1a1a` line along its top
(the divider both critics asked for); name 9 pt bold `#00e5ff`, inset 3 px, ellipsis, width 76-3-16; x slot = the
rightmost 16x16: 1 px `#3a3a3a` vertical divider on its left, ASCII "x" 10 pt `#e0e0e0` centred (Pitfall 6: no
unicode glyph at this size); hairline 1 px TEAL `#4a9a8a` along the band's bottom row, width = round(76·progress01)
(the critic-passed mockup uses teal — `mockup.html` `.band-hairline{background:var(--teal)}` — not the cyan of
design 2.3.2: a cyan line under a cyan word read as an underline, `critic-graphic-design-r2.md`). Waiting: the whole
band at 50 % alpha (`beginTransparencyLayer(0.5f)`), no hairline. Removal is an abrupt vanish (slice D polishes it).
Hit test — `mouseDown` (`LayerStrip.cpp:728-741`), NEW branch BEFORE the transport-scrub branch: for each drawn band k,
if its x slot (144+60, 16k, 16, 16) contains the point → `onRoutineRemove(bands[k].slot)`; return. A point inside a
band but outside its x falls through to today's `onSelect(layerIndex_)` — no new accidental action. No band right-
click menu in slice A (the x is the action; "What it moves" arrives with slice B and brings the menu).
API: `void setRoutineBands(std::vector<RoutineDeckView::Band> bands)` (stores; `repaint(thumbnailBounds_.withHeight(32))`
when slot/state/name/count changed); `std::function<void(int slot)> onRoutineRemove;`. Timer (`:719-726`) adds
`repaint(thumbnailBounds_.withHeight(32))` while any band is Playing (the hairline creeps).
Repaint/data flow: `DeckView::setRoutineView` fans `bandsByLayer[layerIdx]` to `layerStrips_[displayRow]` with the
mirror `displayRow = numLayers - 1 - layerIdx` (`DeckView.cpp:108-111`). Strips are rebuilt by `handleDeckSwitch`
(`MainComponent.cpp:5138-5139`) — the next 30 Hz push re-arms the bands (≤ 33 ms without them).

### 2.3 Pad settings menu — `DeckView::showRoutinePadMenu(int slot)`
Built from the pure `RoutineDeckView::padMenu(const Pad&)` item list (5.1) with the `showDeckTabMenu` idiom
(`DeckView.cpp:476-501`): `juce::PopupMenu`, `addSectionHeader(name)`, `addItem(id, label, enabled, ticked)`, a
"Quantize" `addSubMenu`, `menu.setLookAndFeel(&getLookAndFeel())` exactly as `:492` and `:514` do in this file (the
app LAF is now the default too — `LookAndFeel.h:9-20`; loose end 7 retires all three lines together later),
`showMenuAsync(Options().withTargetComponent(pad).withParentComponent(getTopLevelComponent()))` (CLAUDE.md UI
Patterns). Items, in order (ids = `RoutineDeckView::PadMenu` enum values):
Loop ✓/Once · Restore first ✓/Start from now · Start: Ease ✓/Start: Jump · Quantize ▸ Off/Beat/Bar/2 Bar/4 Bar ·
Rename... · Remove from layers (ENABLED only when Waiting/Playing — the logic critic's r2 SHOULD) · — · Delete routine.
No "Stop" item anywhere. Results → DeckView callbacks → MainComponent:
| Item | Callback → engine |
|---|---|
| Loop / Once | `onRoutineSet(slot, {loop = true/false})` → `perfRoutineSet` (`MainComponent.cpp:5692-5734`; takes effect at the next end `:5703-5706`) |
| Restore first / Start from now | `onRoutineSet(slot, {restoreState = …})` → same |
| Start: Ease / Jump | `onRoutineSet(slot, {restoreStyle = "ease"/"jump"})` → `perfRoutineSet` `:5708-5709` → `Routine::restoreStyle`; engine reads it at a fire, a re-fire and a loop's last beat (`RoutineEngine.cpp:476,534-541,587-594`) — SHIPPED, reused as is |
| Quantize ▸ | `onRoutineSet(slot, {quantize = "off"/"beat"/"bar"/"2bar"/"4bar"})` → `:5710-5711` (next start) |
| Rename... | `onRoutineRename(slot)` → `MainComponent::renameRoutine`: the `renameDeck` AlertWindow idiom (`:3273-3291`; text editor, Rename/Cancel, `enterModalState(true, …)`) → `perfRoutineSet({name})` (`:5712-5713`, at once) |
| Remove from layers | `onRoutineRemoved(slot)` → `perfRoutineStop(slot, false)` → `stop(slot)` (`RoutineEngine.cpp:650-663`, non-destructive) |
| Delete routine | `onRoutineDeleted(slot)` → `MainComponent::deleteRoutine`: confirm AlertWindow "Delete routine \"Drop\" from this show? This cannot be undone." [Delete/Cancel] (the plan6 rule: a destructive library action sits behind a confirm) → `perfRoutineRemove(slot)` (`:5736-5750`, stops then erases) |

### 2.4 Layer X (clear) also takes routines off that layer (default, question 2)
`deckView_->onLayerClearClip` (`MainComponent.cpp:746-776`): ONE added line right after the null checks and before the
`before` snapshot: `routineEngine_.stopOnLayer(composition_.activeDeckIndex, layerIdx);` (3.2). Whole-routine
`stop(slot)` semantics; the undo command still restores the clip only (routine run state is not a command) — disclosed.

### 2.5 The V fader follows the model + cyan cue — `LayerStrip`
Bug (VERIFIED): `setLayer` pulls `layer_->opacity` once (`LayerStrip.cpp:685`); `refresh()` (`:710-717`) and the 30 Hz
`timerCallback` (`:719-726`) never re-pull, so a routine, REST `set_layer_opacity` (→ `manualWrite`,
`MainComponent.cpp:1901-1904`), MIDI or OSC write never moves the fader. Fix: NEW public seam `void syncFromModel()`
called from `timerCallback` (the `TopBar::syncMasterFromComposition` precedent, `tests/CMakeLists.txt:1700-1701`):
- V: `if (layer_ && !opacitySlider_.isMouseButtonDown())` (`juce_Component.h:2031`) → shown =
  `layer_->scalarConns[Opacity].isConnected() ? layer_->eff(LayerScalar::Opacity) : layer_->opacity` (the
  LayerInspector rule `LayerInspector.cpp:911-916`; opacity's toNorm is identity `ScalarParams.h:99`) →
  `setValue(shown, dontSendNotification)` when it differs by > 1e-4 (never fires `onValueChange` → no grip, no write).
- S: same for the active clip's `speed / 4` (a routine CAN drive `speed`, `ControlPath.h:59-63`), skipped while
  `speedSlider_.isMouseButtonDown()`. K and F are not routine-reachable and have no other writer: unchanged (surgical).
- Cyan cue: `OpacitySliderLookAndFeel::drawLinearSlider` (`:111-135`) fills with the hard-coded `0xff4a7a6a` (`:123`);
  change the fill to `slider.isColourSpecified(Slider::trackColourId) ? slider.findColour(trackColourId) :
  Colour(0xff4a7a6a)`. `syncFromModel` sets `opacitySlider_.setColour(trackColourId, kAccentCyan)` while
  `conn.grip.kind != None && conn.grip.rank == static_cast<uint8_t>(Hand::Lane)` (`ParamConnection.h:105-124`,
  `ManualWrite.h:24`; stamped `ManualWrite.cpp:171-178`, cleared `:191-204`), else `removeColour`; repaint the slider
  on a change. Honest limit (design 2.3.4): rank-based, so a take Replay lights it too and it blinks between a
  routine's gestures — "a recorded hand is moving this now"; the steady named cue is slice C.
- Sliders get component ids for tests: `opacitySlider_.setComponentID("layerOpacity")`, `speedSlider_ "layerSpeed"`
  (the `RecordPanel.cpp:130` idiom).

### 2.6 Record tab — pad row removed (`src/ui/RecordPanel.h/.cpp`)
Delete: `onFireRoutine`, `onStopRoutine`, `onRoutineStatus` (`RecordPanel.h:53-54,56`; keep `onSaveRoutine` `:55`),
`applyRoutines` `:78`, `applyPad` `:80`, `routinesLabel_`/`routinePads_`/`lastRoutineView_` `:110-112`; the pad-row
construction `RecordPanel.cpp:121-139`; `applyRoutines`' calls `:195,:226,:373` and its body `:343-358`; `applyPad`
`:320-341`; the pad layout `:432-450` (keep the Save Routine label/rows/notice `:452-475`). Keep `runRoutineAction`
(`:360-374`, the Save button uses it `:182`) minus its `applyRoutines` call. `MainComponent.cpp:1571-1572,1581`
deleted. `RoutineBankModel.h`: delete `RoutineBankView`/`deriveRoutineBankView` (`:33-91`), keep
`kRoutineBeatsPerBar` (`:16`, used by the new model) and `clampRoutineBarRange` (`:24-31`, used by the Save row).
`tests/test_routine_bank_model.cpp`: delete the six view cases (`:7-79`), keep the five clamp cases. Retire
`tests/tool_routine_strip_snapshot.cpp` + its target (`tests/CMakeLists.txt:1507-1535`) — it only rendered the pad row.

### 2.7 Unchanged in slice A
Clip cells (a routine's triggers light through `DeckView.cpp:153,244` / `ClipCell.cpp:157-161`); TopBar bar count
(already 1-2-3-4, `TopBar.cpp:528-534` — mockup screen 12 needs no work); TopBar Stop = routines only
(`MainComponent.cpp:632-640`, `:7245-7250`).

---

## 3. Model / engine changes (additive only) and persistence

### 3.1 `RoutineEngine::Status::Slot` (`RoutineEngine.h:69-87`) gains
`int deck = -1; std::vector<int> layers; bool touchesComp = false; bool restartPending = false; uint32_t fireSeq = 0;
std::string startsOn;` — filled for pending AND running slots in `publishStatus` (`RoutineEngine.cpp:725-762`) from
`Running` (`:126-149`), which gains `int deck = -1; std::vector<int> layers; bool touchesComp = false; uint32_t fireSeq
= 0;` computed ONCE in `fire()` right after `compileRoutine` (`:613`) by a file-local `footprint(const Program&)`:
walk `program->preamble` (`Fired.target`, `Program.h:40-47`), `preambleContinuous` (`PreambleSet.target` `:61-66`),
`discrete` (`Fired.target`), `continuous` (`ContLane.target` `:49-55`); `deck` = the first `target.deck >= 0`;
`layers` = sorted, deduplicated `target.layer >= 0`; `touchesComp` = any target with `layer == -1`. RESOLVED targets,
never `ControlPath::layer` (a rebound-by-name lane may sit on another index, `ControlPath.h:14-17`). INFERRED (pinned
by 5.2 a): for a deck-relative key `ResolvedTarget.deck` is the active deck at fire (`Program.cpp:563-605` →
`resolveKey`; `facts.md` 1.2). `fireSeq` = `fires_` after its increment (`:622`) — the fire order (bands newest first).
`restartPending` = `Running::restartRequested` (`:142`). `startsOn` (pending only) = the effective grid
`effectiveSnap(lastForcedSnap_, r.ownSnap)` as "now"/"beat"/"bar"/"2bar"/"4bar" — NEW private
`RoutineSnap lastForcedSnap_ = RoutineSnap::Off;` written at the top of `tick()` (`:418-439`) and in `fire()`.
Warning persistence: NEW private `struct LastRun { std::string uuid; int unresolved = 0, preambleUnresolved = 0, skipped
= 0; } lastRun_[kBankSize];` written when a `Running` ends (the once-end `:513-521` and both stop paths `:650-676`) —
`publishStatus` copies these three counters into an IDLE slot whose `uuid` matches, so the "!" survives the run
(mockup screen 9); `stopAll()` (`:665-676`: composition load, shutdown, Stop) clears `lastRun_`. The mutex-guarded
copy (`:760-761,764-768`) now carries eight tiny vectors — fine at 30 Hz + the HTTP thread.

### 3.2 `void RoutineEngine::stopOnLayer(int deck, int layer)` (public, next to `stop` `:118`)
`ROUTINE_ENGINE_ASSERT_MESSAGE_THREAD()`; collect every `running_` slot with `r.deck == deck` and `layer ∈ r.layers`;
call `stop(slot)` for each (whole-routine, grips released `:650-663`); `publishStatus` runs inside `stop`. No-op when
nothing matches.

### 3.3 REST (`MainComponent::routineStatusVar` `:5752-5808`)
Each `bank[]` entry gains `"deck"`, `"layers"` (array of ints), `"touchesComp"`, `"restartPending"`, `"fireSeq"`,
`"startsOn"`; the existing `unresolved`/`preambleUnresolved`/`skipped` keys now persist on an idle slot per 3.1. No new
route (nothing in `src/api`). `/api/routine/set` already accepts `restoreStyle` (`ApiServer.cpp:1640+`,
`ApiServer.h:145-151`).

### 3.4 Persistence
Nothing new is saved: every setting the menu writes already round-trips in the composition JSON (`Routine::toVar`
`Routine.h:118-146`: `loop`, `restoreState`, `restoreStyle`, `quantize`, `name`). Run state (pending/running, bands,
warnings) is never persisted. `Composition::fromVar` reads `routines`/`routineBank` guarded (`Composition.h:629-667`).

### 3.5 View model — `src/ui/RoutineDeckView.h` (NEW, header-only, juce_core only — `RoutineEngine.h` is juce_core-only
by rule, `RoutineEngine.h:14-15`, `.harmony/notebook.md:1484-1486`; include `ui/RoutineBankModel.h` for `kRoutineBeatsPerBar`)
```cpp
struct RoutineDeckView {
    enum class State { Empty, Idle, Waiting, Playing };
    struct Pad { int number = 0; juce::String name; State state = State::Empty; bool onShownDeck = true;
                 bool loop = false, restoreFirst = true, startEase = true; juce::String quantize;   // menu ticks
                 float progress01 = 0.0f; int bar = 0, barsTotal = 0; bool warning = false; juce::String tooltip; };
    Pad pads[RoutineEngine::kBankSize];
    struct Band { int slot = -1; juce::String name; State state = State::Idle; float progress01 = 0.0f; };
    std::map<int, std::vector<Band>> bandsByLayer;   // shown deck's layer index -> ALL bands, newest fireSeq first
    juce::String cornerNote;
    enum class PadMenu : int { Loop = 1, Once, RestoreFirst, StartFromNow, StartEase, StartJump, QuantizeOff,
                               QuantizeBeat, QuantizeBar, QuantizeTwoBar, QuantizeFourBar, Rename, RemoveFromLayers,
                               DeleteRoutine };
    struct MenuItem { PadMenu id; juce::String label; bool enabled, ticked, separatorBefore, inQuantizeSubmenu; };
};
struct RoutineSettingsChange { std::optional<bool> loop, restoreState; juce::String restoreStyle, quantize; };
RoutineDeckView deriveRoutineDeckView(const RoutineEngine::Status&, int shownDeck,
                                      const std::vector<juce::String>& deckNames,
                                      const std::vector<juce::String>& shownDeckLayerNames);
std::vector<RoutineDeckView::Band> bandsToDraw(const std::vector<RoutineDeckView::Band>&);   // <= 2, "+N" on the 2nd
std::vector<RoutineDeckView::MenuItem> padMenu(const RoutineDeckView::Pad&);
RoutineSettingsChange settingsChangeFor(RoutineDeckView::PadMenu);   // Loop -> {loop=true}, StartJump -> {restoreStyle="jump"} ...
```
Rules: `state` from `Slot.state` ("empty"/"idle"/"pending"/"running"); `onShownDeck = slot.deck < 0 || slot.deck ==
shownDeck`; `bar = floor(position / 4) + 1` clamped to `barsTotal = ceil(lengthBeats / 4)`; `progress01 = position /
lengthBeats` clamped [0,1] (0 when `lengthBeats <= 0`); `warning = unresolved + preambleUnresolved + skipped > 0`;
`restoreFirst = restoreState`, `startEase = restoreStyle != "jump"`; bands only for Waiting/Playing slots with `deck ==
shownDeck`, one per `layers` entry, sorted by `fireSeq` descending; `cornerNote` per 2.1; tooltips per 2.1. There is
NO press-action field: a pad has one press action, Fire.

---

## 4. REST / test hooks — every mockup state on screen without synthetic input

Production binary, port 7070, `open -g`, live lock held. Fixture `.harmony/probe-routine-display.json` (shape of
`.harmony/probe-routines.json`, `@ROOT@` substituted like `probe-routines.sh` does): deck "A" (active; 3 layers L1/L2/L3,
2 columns, a `test_card.png` clip on L1 col 0 and L3 col 0) and deck "B" (1 layer), PLUS three routines written by hand
in the composition JSON (`Composition.h:418-429` keys `routines` / `routineBank`; `Routine.h:118-146`;
`ControlPath.h:73-128`, scope strings `:182-193`) — no take, no recording needed:
- "Drop" (`uuid` "fx-drop", pad 0): `lengthBeats` 32, `quantize` "bar", `loop` false, `lanes` [], `preamble` = two
  continuous entries — `{"key":{"scope":"layer","deck":{"i":0,"rel":true,"name":"A"},"layer":{"i":0,"id":0,"name":"L1"},
  "control":"scalar","scalar":"opacity"},"norm":0.3}` and the same for layer `{"i":2,"id":2,"name":"L3"}` with `norm`
  0.8 → footprint layers {0, 2}, 8 bars.
- "Build" (pad 1): `lengthBeats` 16, `loop` true, one entry on layer 0 (`norm` 0.6) → layers {0}, "LOOP" tag.
- "Wash" (pad 2): `lengthBeats` 4, one entry on layer `{"i":5,"id":5,"name":"L6"}` → Missing at compile →
  `preambleUnresolved` 1 → the "!" pad (after its 2 s run it is idle AND still flagged, 3.1).
INFERRED: `ControlPath::fromVar` reads `deck.rel` (its `toVar` writes it `:82`) — the builder verifies by loading the
fixture and reading `bank[0].name == "Drop"` and, after a fire, `preambleUnresolved == 0`.
State → how it is put on screen (all `POST` unless noted; `set_bpm 120` first puts the tracker in manual LOCKED mode
with a 2 s bar, `probe-routines.sh:20-24`):
| Mockup state | Route(s) | Shot |
|---|---|---|
| 8 Empty bank | launch; the default composition has no routines | window shot at +3 s: "ROUTINES · Save one in the Record tab", eight dim numbers |
| 1 Idle | `/api/load_composition` fixture | "1 Drop", "2 Build LOOP", "3 Wash" |
| 2 Waiting | `/api/routine/set {"slot":0,"quantize":"4bar"}`, then `/api/routine/fire {"slot":0}`; shot at +1.0 s while `status.bank[0].state == "pending"` (a 4-bar line is 2-8 s away); then `set` quantize back to "bar" | thin teal frame; dim "Drop" bands on L1 and L3 strips, no hairline |
| 3 Playing on two layers | wait for `state == "running"`; shot at +9 s (bar 5 of 8) | thick frame, sweep ~56 %, "5/8", bands with hairline on L1 + L3; V fader of L1 at 0.3 / L3 at 0.8 (the restore moved them: the follow fix) |
| 4 Two on one layer | `/api/routine/fire {"slot":1}`; after its bar | L1 strip: "Build" on top (newer fireSeq), "Drop" below with a divider; L3: "Drop" only |
| 5 Off-deck | `/api/switch_deck {"deck":1}` | pads 1-2 at 50 %, no bands, corner "· Drop, Build on A"; `status.bank[0].deck == 0` and `position` still advancing |
| 6 Removal (after) | `/api/switch_deck {"deck":0}`; `/api/routine/stop {"slot":0}` | bands of Drop gone everywhere, pad 1 idle, `layers == []`; Build's band stays |
| 9 Warning | `/api/routine/fire {"slot":2}`; shot while running and again 3 s later (idle, still "!") | red "!" before "3"; tooltip text is pinned by the ctest (a hover cannot be produced) |
| 7 Layer X | TEMPORARY env-var hook (never committed; `.harmony/notebook.md:1603-1611,1614-1622`): `AUDIODNA_DEBUG_SHOW=layer_x:0` calls `deckView_->onLayerClearClip(0)` at T+N s after a scripted fire | L1 cleared, every routine that touched L1 gone from every strip, their pads idle; permanent proof of the semantics = the engine ctest (5.2 c) |
| 10 Pad menu | hook `pad_menu:1` opens `showRoutinePadMenu(1)` then `createComponentSnapshot` of the top-level window synchronously (a PopupMenu dismisses within ~50 ms in a background app — plan6's idiom, `notebook.md:1614-1618`; absolute `AUDIODNA_DEBUG_SNAP` path, `:1746-1750`) | flat `#2a2a2a` menu: Loop ✓ … Start: Ease ✓ / Start: Jump … Remove from layers (enabled: Build is playing) · — · Delete routine |
| 11 Record tab after | hook `AUDIODNA_DEBUG_TAB=record` (the s-rta-0926b idiom) | Save Routine row + notice line, no pad row |
| 12 TopBar | already shipped (`TopBar.cpp:528-534`) | none needed |
| V fader follows / cyan cue | `/api/set_layer_opacity {"layer":1,"opacity":0.3}` → L2's V fader drops (no routine); during state 3 the L1/L3 V fills are cyan while Drop's restore hands grip them (they blink between gestures — expected) | two shots, looked at |
Headless complement (no window at all): `tool_routine_deck_snapshot` (ctest-excluded, like the retired tool
`tests/CMakeLists.txt:1507-1535`) renders (a) a 970x22 row of eight `RoutinePad`s and (b) one 250x96 `LayerStrip` with
bands, for synthetic `RoutineDeckView` states empty / idle / waiting / playing 5-of-8 / two-on-one-layer / off-deck /
warning → PNGs via `createComponentSnapshot`. Harmony eyeballs both sets against `routine-ux/shots/*.png`.

---

## 5. Tests — RED on today's code (each with its RED expectation)

### 5.1 `tests/test_routine_deck_view.cpp` (NEW target after `:1454-1476`; links `Catch2 + juce_core` ONLY, like
`test_routine_bank_model`) — RED: the header does not exist (compile failure recorded as RED). Cases (one TEST_CASE each):
empty pad (number 1, State::Empty, tooltip names the Record tab); idle + LOOP (`loop`, tooltip "Press to play. Right-click
for settings."); waiting (`Slot.state "pending"`, `deck 0`, `layers {0,2}`, `startsOn "bar"` → State::Waiting, progress 0,
tooltip "Starting on the next bar.", bands on layers 0 and 2 with State::Waiting, none on 1); playing (`position 17.0`,
`lengthBeats 32` → bar 5, barsTotal 8, progress 0.53125; tooltip "Playing on Layer 1, Layer 3. Press to restart from the
top."; bands on exactly `layers`); off-deck (`deck 0`, shownDeck 1 → `onShownDeck false`, no bands, cornerNote
"· Drop on A"; tooltip "Playing on A. Switch decks to see its layers."); newest first (`fireSeq` 3 above 1 on layer 0)
and `bandsToDraw` with three bands → two, the second labelled "Build +1"; warning text composed from counts (2/1/3 and
the singular forms); corner note when every pad is empty; `padMenu`: order, Start: Ease ticked when `startEase`, Loop
ticked when `loop`, Remove from layers enabled iff Waiting/Playing, Delete last with a separator; `settingsChangeFor`
maps StartJump → `restoreStyle "jump"`, QuantizeTwoBar → `"2bar"`. The five `clampRoutineBarRange` cases stay in
`test_routine_bank_model.cpp` unchanged.

### 5.2 `tests/test_routine_engine.cpp` additions (target `:920-964`; the `Rig`/`FakeDispatch` at `:99-215`) — RED: the
`Status::Slot` fields and `stopOnLayer` do not exist (compile failure = RED; record it):
(a) fire `test7Routine` (layers 0 and 1, `:217+`) on `comp.activeDeckIndex = 0` → `slot(0).deck == 0`, `layers == {0,1}`,
`fireSeq == 1`, `state == "pending"`, `startsOn == "bar"`; after `runTo(4.0)` still `deck 0`, `layers {0,1}`,
`state "running"`; (b) with a second deck appended (`comp.appendDeck`) and `activeDeckIndex = 1`, a deck-relative
routine reports `deck == 1` (pins the INFERRED resolution rule); (c) two routines — one touching layers {0,1}, one
touching {0} only: `stopOnLayer(0, 1)` stops the first only; `stopOnLayer(1, 0)` (other deck) stops nothing;
`stopOnLayer(0, 0)` stops both; every grip released (`fd.count(Ev::Release, key)`); (d) warning persists: a routine
whose preamble names layer 5 → while running `preambleUnresolved == 1`; after its once-end `state == "idle"` and
`preambleUnresolved == 1`; after `stopAll()` → 0; (e) `restartPending` is true after a second `fire` while running and
false after the restart lands; (f) G1-G12 and J1-J4 unedited and green (timing untouched).

### 5.3 `tests/test_layer_strip_follows_model.cpp` (NEW GUI target: copy the `test_right_click_reset` link set
`:1657-1679` + `${SRC_DIR}/ui/LayerStrip.cpp` + `${SRC_DIR}/ui/LookAndFeel.cpp`; `ScopedJuceInitialiser_GUI`; add
the ONE .cpp per linker-named symbol, never MainComponent/Renderer — `:1702-1706`). Cases: (a) `Layer layer; LayerStrip
strip; strip.setLayer(&layer, 0); strip.setSize(250, 96); layer.opacity = 0.3f; strip.syncFromModel();` →
`dynamic_cast<juce::Slider*>(strip.findChildWithID("layerOpacity"))->getValue() == 0.3` — RED today: `syncFromModel`
does not exist (compile RED); the sibling case that compiles today: `REQUIRE(strip.findChildWithID("layerOpacity") !=
nullptr)` fails (no ids) — a running RED; (b) connected opacity shows `eff()`: set `layer.scalarConns[Opacity].source`
connected and `scalarLive[Opacity].v = 0.7f` → slider 0.7; (c) cyan cue: `layer.scalarConns[Opacity].grip = {Held, rank
Lane}` → after `syncFromModel` `isColourSpecified(trackColourId)` and the colour == kAccentCyan; `rank HumanHeld` → not
specified; released → not specified; (d) speed follows the active clip's `speed`. NOT covered (disclosed): "skipped while
dragging" — `isMouseButtonDown` needs a real mouse source; code review checks the guard.

### 5.4 `tests/test_routine_pad_press.cpp` (NEW GUI target, same harness) — RED: `RoutinePad` does not exist. A visible
pad (Pitfall 34) with `setSpec` Playing: a synthesised left `mouseDown` (the `leftClickOn` idiom `test_topbar_link_toggle.cpp:33-38`)
→ `onFire` called once, `onContextMenu` 0; `mods = popupMenuClickModifier` → `onContextMenu` once, `onFire` 0; Empty →
neither; Waiting → `onFire` once (the engine makes it a no-op). Compile-level pin of the removed grammar:
`static_assert(!requires(RoutinePad& p) { p.onStop; }, "a routine pad has one press action: Fire");`.

### 5.5 `tests/test_record_panel_pads_removed.cpp` (NEW; links `RecordPanel.cpp + LookAndFeel.cpp + juce_gui_basics`, the
retired tool's link set `:1515-1523`) — a RUNNING RED today: `RecordPanel p; CHECK(p.findChildWithID("routinePad0") ==
nullptr)` fails (the pad exists, `RecordPanel.cpp:130`); `CHECK(p.findChildWithID("saveRoutine") != nullptr)` and
`"routineFromBar"`/`"routineToBar"`/`"routineName"` present (the Save row survives).

### 5.6 C7 — `test_right_click_reset`-harness case in a NEW `tests/test_param_control_routine_cue.cpp`: a bound
`UniversalParamControl` (visible, 260x24) with `conn.grip = {Held, rank Lane}` → `createComponentSnapshot`, sample the
value-digit pixel (`x ≈ kTriangleSize + 72 + 30, y 12`) is cyan and the hint slot contains "ROUTINE"-coloured pixels
(the `test_topbar_link_toggle` pixel-sampling method); `rank HumanHeld` → digits `kTextPrimary`. RED: no cue today.

### 5.7 Live gate — NEW `.harmony/probe-routine-display.sh` (+ `.json`); mechanics copied from `probe-deck-tabs.sh`
(`shot()` `:76-95`, lock gate, `open -g`, `zero_windows`) and `probe-routines.sh` (`rt.py` `:99-200`, `set_bpm`).
Rows, RED on the `build/` app first:
d1 `bank[0]` has `deck`/`layers`/`fireSeq`/`startsOn`/`restartPending` (RED: absent) · d2 fire pad 0 → within 0.5 s
`pending`, `deck 0`, `layers [0,2]`, `startsOn "bar"` (RED: keys absent) · d3 after the edge: `running`, `layers [0,2]`,
`position` advancing · d4 fire pad 1 → `bank[1].fireSeq > bank[0].fireSeq` · d5 `switch_deck 1` → `bank[0].deck == 0`
and `position` grows over 1 s (no stop on switch) · d6 `switch_deck 0`; `stop slot 0` → `idle`, `layers []` · d7 (--hook
only) layer X: after `layer_x:0`, `bank[0].state == "idle"` and `bank[1].state == "idle"` (both touched L1), the clip
cleared (`/api/composition` `activeClipColumn == -1`) · d8 fire pad 2 → `preambleUnresolved == 1` while running and
3 s later while `idle` (RED: an idle slot reports 0 today) · d9 shots 01-idle, 02-waiting (with the pending witness),
03-playing, 04-two-on-one, 05-offdeck, 06-removed, 08-empty, 09-warning, 12-fader-follow — window-only Quartz captures,
non-blank, looked at · d10 (--hook) 07-layer-x, 10-pad-menu (hook snapshot), 11-record-tab · d11 the app quits; 0
Audio-DNA windows in the FULL window list (screen-safety law). Existing probes re-run unchanged: `probe-routines`
(`ROUTINES_BUILD_DIR=build-lane ROUTINES_RECORD_PAUSE=1.8`, expect 98/0 — its status keys are additive; the known
timing flake is lane T's), `probe-deck-tabs`, `probe-manual-bpm`.

---

## 6. The 12 mockup states — in slice A? how shot?
1 Idle A live · 2 Waiting A live (4-bar quantize trick) · 3 Playing on two layers A live (the inspector row in the mockup
is C7: hook `AUDIODNA_DEBUG_LAYER=0` selects L1 in the Layer tab for that shot, or the headless tool) · 4 Two on one
layer A live · 5 Off-deck A live · 6 Removal A live (after; the "before" is shot 03) · 7 Layer X A, hook shot · 8 Empty
bank A live · 9 Warning A live (tooltip text = ctest) · 10 Pad menu A, hook snapshot · 11 Record tab after A, hook shot ·
12 TopBar — shipped, not in this lane.

---

## 7. CLAUDE.md UI Patterns that apply
- ResettableSlider: no new slider is added; the V/S sliders stay `ResettableSlider` with their `setDefaultValue`
  (`LayerStrip.cpp:386,418`); `syncFromModel` never fights a right-click reset (the reset writes the model, the timer
  re-reads it).
- PopupMenu: `showMenuAsync` with `.withParentComponent(getTopLevelComponent())` + the file's own `setLookAndFeel`
  line (`DeckView.cpp:492-495`) → square, app colours (`LookAndFeel.h:9-20`; `BORIS_DECISIONS.md:382`).
- Dialogs: AlertWindow via the `renameDeck` idiom (`MainComponent.cpp:3273-3291`) — square, bold title kept.
- DragAndDrop: none new (bands and pads accept no drops; `LayerStrip`'s fx-drop target `:919-947` is untouched).
- Unicode at small sizes (Pitfall 6): ASCII "x" and "!" only. Whole words everywhere (tooltips, menu, notes).
- Pitfall 34: the pad/strip tests make components visible and sized.
- New UI Pattern paragraph to add (C6): "Routine pads and bands: a pad's press is always Fire (restart while playing,
  no-op while waiting); there is no stop control — a routine leaves by the band x (whole routine), the layer X, its own
  end, or Stop (routines only). Pads are model-driven from `RoutineEngine::Status` via `deriveRoutineDeckView` every
  30 Hz tick, never from panel memory."
- New pitfall 40 (C6): "`LayerStrip` sliders must follow the model from the timer: `setLayer` pulled `layer_->opacity`
  once and nothing re-pulled it, so REST/MIDI/OSC/routine writes never moved the fader. `syncFromModel()` (30 Hz)
  re-reads opacity (eff() when connected) and clip speed, skipping a slider under the mouse."

---

## 8. Risks, must-not-change, defaults

Must not change: `RoutineEngine::tick`/`fire`/`startNow`/`scheduleGlides`/`stepGlides`/`releaseGlides` bodies and
constants (`RoutineEngine.h:58-60`), `laneOwner_` arbitration (`SlotSink` `:55-111`), `stop`/`stopAll` semantics
(`stopOnLayer` composes `stop`), Ease/Jump (`Running::jump` `:136`, `:476,534-541,587-594`), TopBar Stop and the
GlobalStop binding = routines only (`MainComponent.cpp:632-640,7245-7250`), `perfRoutine*` signatures, every existing
`/api/routine/status` key, `.harmony/probe-routines.sh` + its fixture (lane T), the Record tab's Save Routine row and
notice, clip-cell active lighting, `DeckView::refresh()`'s 39 call sites (the pads are pushed, never refreshed).

Risks (strongest first):
- Strongest counterargument to the plan: Boris asked for routines "in the layer strip"; a pad row above the columns is a
  second launch surface and pad N sits over column N. It loses because a routine spans layers (fired from one strip it
  would lie about where it lives), the bands give exactly "the same name on every layer it plays", and the
  column-trigger row is the app's existing fires-across-layers grammar (`DeckView.cpp:349-372`). Tier 4 decides; moving
  the row above the deck tabs (question 1) is a `resized()`/`getNaturalHeight()` change only — the model and engine
  are identical.
- Merge with lane O: same files (`MainComponent.h/.cpp`, `CMakeLists.txt`, `tests/CMakeLists.txt`), disjoint hunks as
  fenced in 1.2; whoever merges second rebases and re-runs ctest. A textual conflict is possible only in the two
  CMake lists if both lanes ignore the "not at EOF" rule.
- +22 px deck height: the bottom panels lose 22 px at a small window; `kMinBottomHeight` (`MainComponent.h:424`) caps
  it. Verify at the maximized width and at 1280.
- Bands hide the top 16/32 px of a 76 px thumbnail (SRC/SEQ badge precedent, `ClipCell.cpp:38-40`); fallback if Tier 4
  rejects: a 16 px fourth strip row (`kCellHeight` 112) — same model.
- Whole-routine removal from one layer's band may read as "layer 3 lost it too"; visible cause (both bands vanish
  together); per-layer let-go is slice C if question 3 flips.
- Rank-based V cue also lights for a take Replay and blinks between gestures (disclosed; steady named cue = slice C).
- `ResolvedTarget.deck` for deck-relative keys is INFERRED = active deck; 5.2 (b) pins it before any UI depends on it.
- The fixture's hand-written routine JSON depends on `ControlPath::fromVar` reading `deck.rel` (INFERRED); the probe's
  first row (`bank[0].name == "Drop"`, `preambleUnresolved == 0` after a fire) catches a wrong guess.
- Layer-X → `stopOnLayer` has no permanent live guard (no REST route reaches the X; adding one would touch `src/api`,
  lane O's file) — the engine ctest + the hook witness are the proof; a `POST /api/clear_layer_clip` route is a
  follow-up for a probe-hygiene lane.
- Status copy at 30 Hz + the HTTP thread now carries small vectors (`statusMutex_` `:169`) — negligible; the
  footprint is computed once at fire, never per tick.
- Not addressed (design 2.8): autopilot advancing clips under a routine; replay-vs-routine tick order; "next up" cell
  marks; key/MIDI tag on idle pads (slice B).

Defaults applied for Boris's 8 "Still your call" questions (mockup.html): 1 row at the TOP above the column numbers ·
2 layer X clears the clip AND takes routines off that layer (`stopOnLayer`) · 3 band x removes the routine from ALL
its layers (`stop(slot)`) · 4 a second routine on a shared layer: both keep playing, each keeping the knobs it moves
(engine as shipped) · 5 a hand-triggered clip on a routine's layer: the routine's next recorded point still fires
(engine as shipped) · 6 bar count "5/8" on the pad only · 7 key/MIDI tag always shown — RECORDED, built in slice B
(not in A) · 8 held key/MIDI runs while held (`TriggerRoutine` Momentary, `MainComponent.cpp:7252-7259`, unchanged).
Decided by the ADDENDUM and already shipped: glide lands on the bar over the last beat; Start: Ease/Jump per routine;
Stop = routines only; TopBar Bar 1-2-3-4-1.

---

## 9. Commit sequence (each independently green: build + full serial ctest; RED evidence recorded before each)

C1 `feat(routine-display A/engine)`: `RoutineEngine.h/.cpp` (3.1, 3.2), `MainComponent::routineStatusVar` keys (3.3),
`tests/test_routine_engine.cpp` (5.2). RED: compile failure of the new cases on base. Gate: ctest; `probe-routines`
unchanged (additive keys).
C2 `feat(routine-display A/view model)`: `src/ui/RoutineDeckView.h`, `tests/test_routine_deck_view.cpp` + its target
(5.1). RED: header absent. Gate: ctest.
C3 `fix(routine-display A/strip)`: `LayerStrip` — `syncFromModel` + slider ids + trackColour fill + cyan cue + bands
paint/hit test/`setRoutineBands`/`onRoutineRemove` (2.2, 2.5); `tests/test_layer_strip_follows_model.cpp` + target
(5.3). RED: the running `findChildWithID` failure + compile failure. Nothing calls `setRoutineBands` yet — the strip is
inert but correct. Gate: ctest; `probe-deck-tabs` (strips unchanged in layout).
C4 `feat(routine-display A/pads)`: `RoutinePad.h/.cpp`, `DeckView` row + menu + `setRoutineView` (2.1, 2.3),
`MainComponent` wiring block + `timerCallback` push + `onLayerClearClip` line + `renameRoutine`/`deleteRoutine` +
two `.h` declarations (1.2), `CMakeLists.txt:325` line, `tests/test_routine_pad_press.cpp` + target (5.4),
`tests/tool_routine_deck_snapshot.cpp` + ctest-excluded target. RED: `RoutinePad` absent. Gate: ctest; Harmony looks at
the tool's PNGs. This is the commit Boris can feel.
C5 `refactor(routine-display A/record tab)`: RecordPanel pad row removed, `RoutineBankModel.h` trimmed,
`test_routine_bank_model.cpp` trimmed, `tests/test_record_panel_pads_removed.cpp` (5.5), the retired tool + target
deleted, `MainComponent.cpp:1571-1572,1581` deleted. RED: the running failure of 5.5 on base. Gate: ctest.
C6 `probe+docs(routine-display A)`: `.harmony/probe-routine-display.sh/.json` (5.7; RED run on `build/` recorded,
GREEN on build-lane, shots in `.harmony/.reports/s-rta-0927/routine-display-shots/`), CLAUDE.md UI Patterns + pitfall
index + Key capabilities phrase, `docs/claude/pitfalls.md` 40, `docs/claude/recording.md` Surfaces bullet,
`docs/claude/performance-controls.md` sentence, `.harmony/APP-INVENTORY.md` rows 58/59/85 + ctest count, the lane
report. The env-var hook: applied for the `--hook` shots only, `git apply -R`, REBUILD, `strings … | grep -c
AUDIODNA_DEBUG_` = 0 and `git diff --stat -- src/MainComponent.cpp` empty before this commit.
C7 `feat(routine-display A/knob cue)` — pulled forward from design slice B by the birth prompt, deferrable alone:
`UniversalParamControl::paint` (`UniversalParamControl.cpp:173-230`): when `conn_ && conn_->grip.kind != None &&
conn_->grip.rank == Lane` draw the value digits (`:188-193`) in `kAccentCyan` and "ROUTINE" 8 pt cyan-50 % in the
collapsed hint slot (`:218-229`) instead of the source name (the engine publishes NaN for a gripped connection anyway,
`ConnectionEngine.cpp:86-95`); the triangle stays the SIGNAL cue (`:147-169`); `tests/test_param_control_routine_cue.cpp`
(5.6). Gate: ctest; shot 03 re-taken with the Layer tab open (hook).

Done looks like: Boris presses pad 1 at the top of the deck, watches it wait (thin frame) then sweep with "1/8", sees
"Drop" banded on Layer 1 and Layer 3 with a creeping teal line, sees both V faders move to the recorded look with a
cyan fill, twists one and watches it fight back at the next move, right-clicks the pad and flips Start: Jump, presses
the x on either band and watches both bands go while the look holds — and never sees a stop button.

STATUS: COMPLETE — plan written to .harmony/.reports/s-rta-0927/plan-routine-display-A.md and returned in full in-band.
