# ClipCell name-bar geometry scout — 2026-07-30 (read-only, source-cited)

Purpose: exact geometry + gesture requirements for synthetic driving of clip
drag-move and Clip > Clear (manual-runsheet items 3-4). Scouted by Explore agent;
all claims file:line-cited; the one INFERRED value is labeled.

## 1. Name-bar vs thumbnail rects (cell-local) — name bar at BOTTOM
- kNameBarHeight = 20 — src/ui/ClipCell.h:84
- Name bar = (0, H-20, W, 20) — src/ui/ClipCell.cpp:351-356
- Thumbnail = getLocalBounds().withTrimmedBottom(20) — src/ui/ClipCell.cpp:346-349
- Split: isInThumbnailArea → onTrigger, else → onSelect — src/ui/ClipCell.cpp:185-193, 358-361
- Normal 90x96 cell: name bar = (0, 76, 90, 20). SAFE CLICK POINT = cell-local (45, 86).

## 2. Cell dimensions
- kCellWidth = 90, kCellHeight = 96, kCellGap = 0, kLayerStripWidth = 250,
  kColumnTriggerHeight = 22, kDeckTabHeight = 24 — src/ui/DeckView.h:90-95
- ClipCell::resized() empty (ClipCell.cpp:178); size set by parent. Folded row
  height kFoldedHeight = 22 — src/ui/DeckView.cpp:259.

## 3. Grid layout math — DISPLAY ROWS INVERTED
- DeckView owns clipCells_[displayRow][column] in gridContent_ inside
  gridViewport_ — src/ui/DeckView.h:75,80-81; DeckView.cpp:8-11.
- displayRow = numLayers - 1 - layerIndex (highest layer index at TOP) —
  src/ui/DeckView.cpp:89-92 (confirmed :192, :359).
- Cell origin in gridContent_ (DeckView.cpp:271-290):
  x = 250 + column*90; y = displayRow*96 (nothing folded); size (90, rowH).
- gridViewport_ at (0, 22, w, h) inside DeckView (DeckView.cpp:24,44-45);
  offset by viewport scroll. DeckView placed at MainComponent.cpp:1826 after
  reduced(4) + topBar 34+1 (:1691-1692) + signalBar h+1 (:1710-1712) + row1
  24+2 (:1751,1775). SignalBar default 84, cyclable 26/expanded
  (SignalBar.cpp:100-109, SignalBar.h:48). DeckView y-origin DYNAMIC —
  INFERRED default sum 4+35+85+26 = 150; anchor off a live capture instead.

## 4. Drag-move path
- mouseDown on name bar fires onSelect first (ClipCell.cpp:191-192); mouseDrag
  calls DragAndDropContainer::startDragging, description "clip:<layer>:<col>" —
  ClipCell.cpp:196-206.
- Preconditions (:199-200): mouse-DOWN point outside thumbnail (in name bar) AND
  getDistanceFromDragStart() > 5 (strictly — drive ≥6 px). No modifiers, no
  prior-selection requirement.
- Container = MainComponent (MainComponent.h:57). Target ClipCell accepts
  "clip:" prefix (ClipCell.cpp:365-370); itemDropped → onClipMove(src→dst)
  (ClipCell.cpp:412-423) → DeckView::onClipMoved (DeckView.cpp:162-164) →
  handler MainComponent.cpp:952 → pushes SwapClipsCmd (ClipCommands.h:148,
  constructed MainComponent.cpp:1010). Undo label "Move Clip" (empty target) /
  "Swap Clips" (occupied) — MainComponent.cpp:1008. Self-drop no-op (:955).

## 5. Clip > Clear
- Menus: {Audio-DNA, Composition, Deck, Layer, Column, Clip, Output, Shortcuts,
  View} — MenuBarModel.cpp:9-10. Clip = index 5; "Clear" = FIRST item
  (kClipClear, always enabled) — MenuBarModel.cpp:115-122, MenuBarModel.h:82.
- Precondition in HANDLER only: getSelectedCells() non-empty
  (MainComponent.cpp:3833-3835) — clickable but SILENT NO-OP with no selection.
- Selection: name-bar click → ClipCell::onSelect → DeckView::selectCell →
  selectedCells_ (DeckView.cpp:137-140, 325-340).
- No named Cmd class: snapshots selected cells, writes Clip{}, pushClipEdits
  label "Clear Clip"/"Clear N Clips" — MainComponent.cpp:3845-3859.

## 6. Risk flags for synthetic driving
- FOLD-HEIGHT MIRROR-INDEX BUG (product-bug candidate): layoutGrid row-height
  loop reads deck->getLayer(displayRow) (DeckView.cpp:274) while every other
  loop mirrors (numLayers-1-displayRow). Any folded layer ⇒ row heights land on
  the MIRRORED row ⇒ all cell Y wrong. GUARD: all layers unfolded before driving.
- Folded row (22px): name bar covers y 2-22, thumbnail = 2px strip — trigger
  clicks become selects.
- Drag needs genuinely held button: JUCE drag image deleteSelf()s within 200ms
  if !isDragging() (juce_DragAndDropContainer.cpp:187-205). Continuous
  mouseDrag events required (:133-137); drop target resolved from MOUSE-UP
  screen position (:113) — last move must land in the destination cell.
- Viewport scroll must be read, not assumed: content width 250+90*numCols+30
  (DeckView.cpp:262); wide decks scroll.
- NON-RISKS verified: no viewport auto-scroll during internal drag; external-
  drag escape hatch cannot fire (no shouldDropFiles/TextWhenDraggedExternally
  overrides in src/). Pausing mid-drag safe.
- No double-click trap (only mouseDown/Drag/Up overridden; mouseUp empty —
  ClipCell.h:20-22, ClipCell.cpp:208). Right-click = immediate return (:182-183)
  — header comment promising context menu is stale (ClipCell.h:9).
- Cmd/Shift-click = multi-select toggle; Cmd+click on selected cell DESELECTS
  (ClipCell.cpp:191, DeckView.cpp:330-337). Use PLAIN click before Clip > Clear.
- rebuildGrid() destroys/recreates all cells after Clear + column ops
  (MainComponent.cpp:3859) — recapture coordinates after structural changes.
