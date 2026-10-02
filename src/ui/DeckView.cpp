#include "ui/DeckView.h"
#include "ui/UiPaintCounters.h"

DeckView::DeckView()
{
    thumbnails_.onLanded = [this] { refresh(); };   // s-rta-0928: a thumbnail landed -- waiting cells re-pull
    setOpaque(false);

    // Grid viewport for scrollable content
    gridContent_ = std::make_unique<juce::Component>();
    gridViewport_.setViewedComponent(gridContent_.get(), false);
    gridViewport_.setScrollBarsShown(true, true);
    addAndMakeVisible(gridViewport_);

    // plan6 §6.2: the Remove-Deck undo hint -- created once, hidden until showUndoHint (Pitfall 34: a Component is
    // invisible by default; addChildComponent keeps it so), never rebuilt with the tabs.
    undoHintBtn_ = std::make_unique<juce::TextButton>();
    undoHintBtn_->setColour(juce::TextButton::buttonColourId, juce::Colour(0xff2a2a2a));
    undoHintBtn_->setColour(juce::TextButton::textColourOffId, juce::Colour(0xffcccccc));
    undoHintBtn_->onClick = [this] {
        if (onUndoHint) onUndoHint();
        hideUndoHint();
    };
    addChildComponent(undoHintBtn_.get());

    // s-rta-0927: the eight routine pads, created once (never in rebuildGrid).
    for (int i = 0; i < RoutineEngine::kBankSize; ++i)
    {
        auto pad = std::make_unique<RoutinePad>(i);
        pad->setSpec(lastRoutineView_.pads[i]);
        pad->onFire = [this](int slot) { if (onRoutineFired) onRoutineFired(slot); };
        pad->onContextMenu = [this](int slot) { showRoutinePadMenu(slot); };
        addAndMakeVisible(pad.get());
        routinePads_[static_cast<size_t>(i)] = std::move(pad);
    }
}

void DeckView::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff1a1a1a));

    // s-rta-0927: the ROUTINES row's corner cell -- the label, then the corner note after it.
    const juce::Font label(juce::FontOptions(10.0f, juce::Font::bold));
    g.setColour(juce::Colour(0xff888888));
    g.setFont(label);
    g.drawText("ROUTINES", juce::Rectangle<int>(6, 0, kLayerStripWidth - 6, kRoutineRowHeight),
               juce::Justification::centredLeft, false);
    if (routineCornerNote_.isNotEmpty())
    {
        const int noteX = 6 + juce::GlyphArrangement::getStringWidthInt(label, "ROUTINES") + 4;
        g.setFont(juce::Font(juce::FontOptions(9.0f)));
        g.drawText(routineCornerNote_, juce::Rectangle<int>(noteX, 0, kLayerStripWidth - 4 - noteX, kRoutineRowHeight),
                   juce::Justification::centredLeft, true);
    }
}

void DeckView::resized()
{
    auto area = getLocalBounds();

    // s-rta-0927: the ROUTINES row at the very top; pad N over column N.
    auto routineRow = area.removeFromTop(kRoutineRowHeight);
    for (size_t i = 0; i < routinePads_.size(); ++i)
        routinePads_[i]->setBounds(kLayerStripWidth + static_cast<int>(i) * (kCellWidth + kCellGap), routineRow.getY(),
                                   kCellWidth, kRoutineRowHeight);

    // Column trigger row at top (shifted right by layer strip width)
    auto triggerRow = area.removeFromTop(kColumnTriggerHeight);

    // Position column triggers
    int triggerX = kLayerStripWidth;
    for (size_t i = 0; i < columnTriggers_.size(); ++i)
    {
        columnTriggers_[i]->setBounds(triggerX, triggerRow.getY(),
                                       kCellWidth, kColumnTriggerHeight);
        triggerX += kCellWidth + kCellGap;
    }

    // Calculate grid height to know where tabs should go
    int numLayers = 0;
    if (composition_)
        if (auto* deck = composition_->getActiveDeck())
            numLayers = deck->getNumLayers();

    int gridHeight = (kCellHeight + kCellGap) * numLayers;

    // Grid viewport — only as tall as the layers need
    int viewportHeight = std::min(gridHeight, area.getHeight() - kDeckTabHeight);
    gridViewport_.setBounds(area.removeFromTop(viewportHeight));

    // Deck tabs immediately after the grid (attached to bottom of last layer); the "+" after the last tab and the
    // Remove-Deck undo hint flush right (plan6 §6.1 DeckTabRow::layout -- tabs never move for the hint).
    auto tabArea = area.removeFromTop(kDeckTabHeight);
    const auto L = DeckTabRow::layout(tabArea.getWidth(), static_cast<int>(deckTabs_.size()),
                                      undoHintBtn_->isVisible() ? undoHintBtn_->getWidth() : 0);
    for (size_t i = 0; i < deckTabs_.size(); ++i)
        deckTabs_[i]->setBounds(tabArea.getX() + L.tabs[i].x, tabArea.getY(), L.tabs[i].w, kDeckTabHeight);
    if (plusTab_)
        plusTab_->setBounds(tabArea.getX() + L.plus.x, tabArea.getY(), L.plus.w, kDeckTabHeight);
    if (L.hint.w > 0)
        undoHintBtn_->setBounds(tabArea.getX() + L.hint.x, tabArea.getY(), L.hint.w, kDeckTabHeight);
    else if (undoHintBtn_->isVisible())
        hideUndoHint();                         // no room next to the "+": never overlap it

    // Layout grid content inside viewport
    layoutGrid();
}

void DeckView::setComposition(Composition* comp)
{
    composition_ = comp;
    rebuildGrid();
}

void DeckView::rebuildGrid()
{
    // plan6 §6.2: every structural change retires the Remove-Deck undo hint (removeDeck shows it AFTER its rebuild).
    hideUndoHint();

    // Clear existing
    layerStrips_.clear();
    clipCells_.clear();
    columnTriggers_.clear();
    deckTabs_.clear();
    gridContent_->removeAllChildren();

    if (!composition_)
        return;

    auto* deck = composition_->getActiveDeck();
    if (!deck)
        return;

    int numLayers = deck->getNumLayers();
    int numCols = deck->numColumns;

    // Create layer strips and clip cells
    // Layers are displayed top-to-bottom in REVERSE order (highest layer at top)
    clipCells_.resize(static_cast<size_t>(numLayers));

    for (int displayRow = 0; displayRow < numLayers; ++displayRow)
    {
        // Display row 0 = highest layer index (top of screen = top layer)
        int layerIdx = numLayers - 1 - displayRow;
        auto* layer = deck->getLayer(layerIdx);
        if (!layer) continue;

        // Layer strip
        auto strip = std::make_unique<LayerStrip>();
        strip->setThumbnails(&thumbnails_);
        strip->setLayer(layer, layerIdx);

        // Wire callbacks
        strip->onSelect = [this](int idx) {
            selectLayer(idx);
            if (onLayerSelected) onLayerSelected(idx);
        };
        // Undo v1 #13-15: bubble clear/bypass/solo to MainComponent so the
        // mutation is wrapped in an undo command (command wrapping happens only
        // at user entry points). LayerStrip already toggled bypass/solo live and
        // repainted; MainComponent just records the change for undo.
        strip->onClearClip = [this](int idx) {
            if (onLayerClearClip) onLayerClearClip(idx);
        };
        strip->onBypass = [this](int idx, bool bypassed) {
            if (onLayerBypass) onLayerBypass(idx, bypassed);
        };
        strip->onSolo = [this](int idx, bool solo) {
            if (onLayerSolo) onLayerSolo(idx, solo);
        };
        strip->onEffectDropped = [this](int idx, const juce::String& effectDesc) {
            if (onLayerEffectDropped) onLayerEffectDropped(idx, effectDesc);
        };
        strip->onRoutineRemove = [this](int slot) {   // s-rta-0927: a band's x
            if (onRoutineRemoved) onRoutineRemoved(slot);
        };

        gridContent_->addAndMakeVisible(strip.get());
        layerStrips_.push_back(std::move(strip));

        // Clip cells for this layer
        auto& layerCells = clipCells_[static_cast<size_t>(displayRow)];
        layerCells.resize(static_cast<size_t>(numCols));

        for (int col = 0; col < numCols; ++col)
        {
            auto cell = std::make_unique<ClipCell>();
            cell->setGridPosition(layerIdx, col);
            cell->setThumbnails(&thumbnails_);
            cell->setClip(layer->getClipAt(col));
            cell->setActive(layer->runtime().activeClipColumn == col);

            // Wire callbacks
            cell->onTrigger = [this](int li, int c) {
                if (onClipTriggered) onClipTriggered(li, c);
            };
            cell->onSelect = [this](int li, int c, bool addToSel) {
                selectCell(li, c, addToSel);
                if (onClipSelected) onClipSelected(li, c, addToSel);
            };
            cell->onFileDrop = [this](int li, int c, const juce::File& file) {
                if (onFileDropped) onFileDropped(li, c, file);
            };
            cell->onMultiFileDrop = [this](int li, int c, const std::vector<juce::File>& files) {
                if (onMultiFileDropped) onMultiFileDropped(li, c, files);
            };
            cell->onMultiVideoDrop = [this](int li, int c, const std::vector<juce::File>& files) {
                if (onMultiVideoDropped) onMultiVideoDropped(li, c, files);
            };
            cell->onMixedFilesDrop = [this](int li, int c, const std::vector<juce::File>& images,
                                             const std::vector<juce::File>& videos) {
                if (onMixedFilesDropped) onMixedFilesDropped(li, c, images, videos);
            };
            cell->onEffectDrop = [this](int li, int c, const juce::String& effectName) {
                if (onEffectDropped) onEffectDropped(li, c, effectName);
            };
            cell->onSourceDrop = [this](int li, int c, const juce::String& sourceId) {
                if (onSourceDropped) onSourceDropped(li, c, sourceId);
            };
            cell->onMilkDropDrop = [this](int li, int c, const std::string& path) {
                if (onMilkDropDropped) onMilkDropDropped(li, c, path);
            };
            cell->onMilkDropPlaylistDrop = [this](int li, int c, const std::vector<std::string>& paths) {
                if (onMilkDropPlaylistDropped) onMilkDropPlaylistDropped(li, c, paths);
            };
            cell->onClipMove = [this](int srcL, int srcC, int dstL, int dstC) {
                if (onClipMoved) onClipMoved(srcL, srcC, dstL, dstC);
            };

            gridContent_->addAndMakeVisible(cell.get());
            layerCells[static_cast<size_t>(col)] = std::move(cell);
        }
    }

    // Column trigger buttons
    setupColumnTriggers();

    // Deck tabs
    setupDeckTabs();

    fanRoutineBands();   // s-rta-0927: fresh strips carry the last pushed bands at once

    // A structural rebuild (layer add/remove, column count change) can leave
    // selectedCells_ pointing at layer/column coordinates that no longer
    // exist in the freshly created grid. Drop those entries so selection
    // never outlives the cells it refers to, then re-apply visuals — the
    // new ClipCells default to unselected regardless of what selectedCells_
    // says, so without this a still-valid selection would look invisible.
    selectedCells_.erase(
        std::remove_if(selectedCells_.begin(), selectedCells_.end(),
            [&](const CellPos& p) {
                return p.column < 0 || p.column >= numCols || deck->getLayer(p.layer) == nullptr;
            }),
        selectedCells_.end());
    updateSelectionVisuals();

    // Layout everything
    if (getWidth() > 0 && getHeight() > 0)
        resized();
}

void DeckView::refresh()
{
    if (!composition_) return;
    auto* deck = composition_->getActiveDeck();
    if (!deck) return;

    int numLayers = deck->getNumLayers();

    for (int displayRow = 0; displayRow < static_cast<int>(layerStrips_.size()); ++displayRow)
    {
        int layerIdx = numLayers - 1 - displayRow;
        auto* layer = deck->getLayer(layerIdx);
        if (!layer) continue;

        layerStrips_[static_cast<size_t>(displayRow)]->refresh();

        auto& layerCells = clipCells_[static_cast<size_t>(displayRow)];
        for (size_t col = 0; col < layerCells.size(); ++col)
        {
            if (layerCells[col])
            {
                layerCells[col]->setClip(layer->getClipAt(static_cast<int>(col)));
                layerCells[col]->setActive(layer->runtime().activeClipColumn == static_cast<int>(col));
            }
        }
    }

    // Update column trigger highlights
    for (size_t col = 0; col < columnTriggers_.size(); ++col)
    {
        bool isActive = static_cast<int>(col) == activeColumn_;
        columnTriggers_[col]->setColour(
            juce::TextButton::buttonColourId,
            isActive ? juce::Colour(0xff3a5a4a) : juce::Colour(0xff2a2a2a));
    }

    // Update deck tabs: active colour, plus the label and tooltip (a Rename / Save As changes them) --
    // compare-before-set, refresh runs at UI rate.
    for (size_t i = 0; i < deckTabs_.size(); ++i)
    {
        bool isActive = static_cast<int>(i) == composition_->activeDeckIndex;
        deckTabs_[i]->setColour(
            juce::TextButton::buttonColourId,
            isActive ? juce::Colour(0xff3a5a4a) : juce::Colour(0xff2a2a2a));
        if (i < composition_->decks.size())
        {
            const juce::String label(composition_->decks[i].name);
            if (deckTabs_[i]->getButtonText() != label)
                deckTabs_[i]->setButtonText(label);
            const auto tip = tabTooltipFor(composition_->decks[i]);
            if (deckTabs_[i]->getTooltip() != tip)
                deckTabs_[i]->setTooltip(tip);
        }
    }

    repaint();
}

void DeckView::setActiveColumn(int col)
{
    activeColumn_ = col;
    refresh();
}

int DeckView::getNaturalHeight() const
{
    if (!composition_) return 200;
    auto* deck = composition_->getActiveDeck();
    if (!deck) return 200;

    static constexpr int kFoldedHeight = 22;
    int totalRowHeight = 0;
    for (int i = 0; i < deck->getNumLayers(); ++i)
    {
        auto* layer = deck->getLayer(i);
        totalRowHeight += (layer && layer->folded) ? (kFoldedHeight + kCellGap) : (kCellHeight + kCellGap);
    }
    return kRoutineRowHeight + kColumnTriggerHeight + totalRowHeight + kDeckTabHeight;
}

void DeckView::layoutGrid()
{
    if (!composition_) return;
    auto* deck = composition_->getActiveDeck();
    if (!deck) return;

    int numCols = deck->numColumns;
    int numLayers = deck->getNumLayers();
    static constexpr int kFoldedHeight = 22; // P24.12: collapsed row height

    // Calculate total content height with variable row heights
    int contentWidth = kLayerStripWidth + (kCellWidth + kCellGap) * numCols + 30;
    int contentHeight = 0;
    for (int i = 0; i < numLayers; ++i)
    {
        auto* layer = deck->getLayer(i);
        contentHeight += (layer && layer->folded) ? (kFoldedHeight + kCellGap) : (kCellHeight + kCellGap);
    }
    gridContent_->setSize(contentWidth, contentHeight);

    int y = 0;
    for (int displayRow = 0; displayRow < static_cast<int>(layerStrips_.size()); ++displayRow)
    {
        // Display row 0 = highest layer index (top of screen = top layer) —
        // mirror the index like rebuildGrid()/refresh()/updateSelectionVisuals()
        // do, so a folded layer's row height is read from the right layer.
        int layerIdx = numLayers - 1 - displayRow;
        auto* layer = deck->getLayer(layerIdx);
        int rowH = (layer && layer->folded) ? kFoldedHeight : kCellHeight;

        // Layer strip on the left
        layerStrips_[static_cast<size_t>(displayRow)]->setBounds(
            0, y, kLayerStripWidth, rowH);

        // Clip cells
        auto& layerCells = clipCells_[static_cast<size_t>(displayRow)];
        for (int col = 0; col < static_cast<int>(layerCells.size()); ++col)
        {
            int x = kLayerStripWidth + col * (kCellWidth + kCellGap);
            if (layerCells[static_cast<size_t>(col)])
                layerCells[static_cast<size_t>(col)]->setBounds(x, y, kCellWidth, rowH);
        }

        y += rowH + kCellGap;
    }
}

void DeckView::setupColumnTriggers()
{
    columnTriggers_.clear();

    if (!composition_) return;
    auto* deck = composition_->getActiveDeck();
    if (!deck) return;

    for (int col = 0; col < deck->numColumns; ++col)
    {
        auto btn = std::make_unique<juce::TextButton>(juce::String(col + 1));
        btn->setColour(juce::TextButton::buttonColourId, juce::Colour(0xff2a2a2a));
        btn->setColour(juce::TextButton::textColourOffId, juce::Colour(0xff888888));

        int capturedCol = col;
        btn->onClick = [this, capturedCol] {
            if (onColumnTriggered)
                onColumnTriggered(capturedCol);
        };

        addAndMakeVisible(btn.get());
        columnTriggers_.push_back(std::move(btn));
    }
}

void DeckView::clearSelection()
{
    selectedCells_.clear();
    updateSelectionVisuals();
}

void DeckView::selectCell(int layerIndex, int column, bool addToSelection)
{
    if (!addToSelection)
        selectedCells_.clear();

    // Toggle if already selected (Cmd+click to deselect)
    auto it = std::find_if(selectedCells_.begin(), selectedCells_.end(),
        [&](const CellPos& p) { return p.layer == layerIndex && p.column == column; });

    if (it != selectedCells_.end() && addToSelection)
        selectedCells_.erase(it);
    else if (it == selectedCells_.end())
        selectedCells_.push_back({layerIndex, column});

    updateSelectionVisuals();
}

void DeckView::selectLayer(int layerIndex)
{
    selectedLayerIndex_ = layerIndex;
    for (auto& strip : layerStrips_)
        strip->setSelected(strip->getLayerIndex() == layerIndex);
}

void DeckView::updateSelectionVisuals()
{
    if (!composition_) return;
    auto* deck = composition_->getActiveDeck();
    if (!deck) return;

    int numLayers = deck->getNumLayers();

    for (int displayRow = 0; displayRow < static_cast<int>(clipCells_.size()); ++displayRow)
    {
        int layerIdx = numLayers - 1 - displayRow;
        auto& layerCells = clipCells_[static_cast<size_t>(displayRow)];

        for (size_t col = 0; col < layerCells.size(); ++col)
        {
            if (!layerCells[col]) continue;

            bool isSel = std::any_of(selectedCells_.begin(), selectedCells_.end(),
                [&](const CellPos& p) {
                    return p.layer == layerIdx && p.column == static_cast<int>(col);
                });

            layerCells[col]->setSelected(isSel);
        }
    }
}

void DeckView::setupDeckTabs()
{
    deckTabs_.clear();
    ++tabRowBuilds_;

    if (!composition_) return;

    for (size_t i = 0; i < composition_->decks.size(); ++i)
    {
        auto& deck = composition_->decks[i];
        auto btn = std::make_unique<DeckTabButton>(juce::String(deck.name));
        // Active colour at creation too: a rebuild (a deck switch, New / Load / Remove) is not always followed by
        // refresh(), and a freshly built row must still show which deck is on screen.
        btn->setColour(juce::TextButton::buttonColourId,
                       static_cast<int>(i) == composition_->activeDeckIndex ? juce::Colour(0xff3a5a4a)
                                                                           : juce::Colour(0xff2a2a2a));
        btn->setColour(juce::TextButton::textColourOffId, juce::Colour(0xffcccccc));
        btn->setTooltip(tabTooltipFor(deck));

        int capturedIdx = static_cast<int>(i);
        btn->onClick = [this, capturedIdx] { tabClicked(capturedIdx); };
        btn->onContextMenu = [this, capturedIdx] { showDeckTabMenu(capturedIdx); };

        addAndMakeVisible(btn.get());
        deckTabs_.push_back(std::move(btn));
    }

    // plan6 §6.2: the "+" -- a 24x24 square after the last tab, inactive-tab colours, ASCII only (Pitfall 6).
    plusTab_ = std::make_unique<juce::TextButton>("+");
    plusTab_->setColour(juce::TextButton::buttonColourId, juce::Colour(0xff2a2a2a));
    plusTab_->setColour(juce::TextButton::textColourOffId, juce::Colour(0xffcccccc));
    plusTab_->setTooltip("New Deck or Load Deck...");
    plusTab_->onClick = [this] { showPlusMenu(); };
    addAndMakeVisible(plusTab_.get());
}

void DeckView::tabClicked(int deckIndex)
{
    // A click on the deck already showing changes nothing, so it does nothing: no switch, no rebuild (the clicked tab
    // must survive its own onClick to receive a double-click, ruling E-R3).
    if (composition_ == nullptr || deckIndex == composition_->activeDeckIndex)
        return;
    if (onDeckSwitched)
        onDeckSwitched(deckIndex);
}

juce::String DeckView::tabTooltipFor(const Deck& deck)
{
    return (deck.sourceFile == juce::File() ? juce::String("Not in the library yet - Save Deck As... adds it")
                                            : deck.sourceFile.getFullPathName())
         + "\nRight-click: Save / Rename / Duplicate / Remove";
}

void DeckView::showDeckTabMenu(int deckIndex)
{
    if (composition_ == nullptr || deckIndex < 0
        || deckIndex >= static_cast<int>(composition_->decks.size())
        || deckIndex >= static_cast<int>(deckTabs_.size()))
        return;

    // Headed by the deck's name so the performer sees WHICH deck the menu is about before choosing Remove.
    juce::PopupMenu menu;
    menu.addSectionHeader(juce::String(composition_->decks[static_cast<size_t>(deckIndex)].name));
    for (const auto& item : DeckTabRow::tabMenu(static_cast<int>(composition_->decks.size())))
    {
        if (item.separatorBefore)
            menu.addSeparator();
        menu.addItem(static_cast<int>(item.action), item.label, item.enabled);
    }
    menu.setLookAndFeel(&getLookAndFeel());   // the app LookAndFeel: a menu parented to the top-level window would draw stock
    menu.showMenuAsync(juce::PopupMenu::Options()
                           .withTargetComponent(deckTabs_[static_cast<size_t>(deckIndex)].get())
                           .withParentComponent(getTopLevelComponent()),
                       [this, deckIndex](int result) {
                           if (result > 0 && composition_ != nullptr
                               && deckIndex < static_cast<int>(composition_->decks.size()) && onDeckAction)
                               onDeckAction(deckIndex, static_cast<DeckTabRow::Action>(result));
                       });
}

juce::Rectangle<int> DeckView::getRoutinePadRowBounds() const
{
    juce::Rectangle<int> r;
    for (const auto& pad : routinePads_)
        if (pad != nullptr)
            r = r.isEmpty() ? pad->getBounds() : r.getUnion(pad->getBounds());
    return r;
}

void DeckView::setRoutineView(const RoutineDeckView& view)
{
    for (int i = 0; i < RoutineEngine::kBankSize; ++i)
        routinePads_[static_cast<size_t>(i)]->setSpec(view.pads[i]);
    if (view.cornerNote != routineCornerNote_)
    {
        routineCornerNote_ = view.cornerNote;
        repaint(0, 0, kLayerStripWidth, kRoutineRowHeight);
        uipaint::bump(uipaint::counters().deckCornerRepaints, uipaint::SrcCorner);   // s-rta-0929 g4cpu c1
    }
    lastRoutineView_ = view;
    fanRoutineBands();
}

void DeckView::fanRoutineBands()
{
    // Each strip knows its own layer index (the display rows are mirrored); a strip with no band gets none.
    for (auto& strip : layerStrips_)
    {
        const auto it = lastRoutineView_.bandsByLayer.find(strip->getLayerIndex());
        strip->setRoutineBands(it == lastRoutineView_.bandsByLayer.end() ? std::vector<RoutineDeckView::Band>{}
                                                                         : bandsToDraw(it->second));
    }
}

void DeckView::showRoutinePadMenu(int slot)
{
    if (slot < 0 || slot >= RoutineEngine::kBankSize)
        return;
    auto* pad = routinePads_[static_cast<size_t>(slot)].get();
    const auto spec = pad->getSpec();
    if (spec.state == RoutineDeckView::State::Empty)
        return;

    // Headed by the routine's name, the showDeckTabMenu idiom. No "Stop" row anywhere.
    const auto items = padMenu(spec);
    juce::PopupMenu quantize;
    for (const auto& item : items)
        if (item.inQuantizeSubmenu)
            quantize.addItem(static_cast<int>(item.id), item.label, item.enabled, item.ticked);

    juce::PopupMenu menu;
    menu.addSectionHeader(spec.name);
    bool quantizeAdded = false;
    for (const auto& item : items)
    {
        if (item.inQuantizeSubmenu)
        {
            if (!quantizeAdded)
            {
                if (item.separatorBefore)
                    menu.addSeparator();
                menu.addSubMenu("Quantize", quantize);
                quantizeAdded = true;
            }
            continue;
        }
        if (item.separatorBefore)
            menu.addSeparator();
        if (item.destructive)   // "Delete routine": the app's warning red (s-rta-0927 fix round)
            menu.addColouredItem(static_cast<int>(item.id), item.label, juce::Colour(AudioDNALookAndFeel::kMeterRed),
                                 item.enabled, item.ticked);
        else
            menu.addItem(static_cast<int>(item.id), item.label, item.enabled, item.ticked);
    }
    menu.setLookAndFeel(&getLookAndFeel());   // the app LookAndFeel: a menu parented to the top-level window would draw stock
    menu.showMenuAsync(juce::PopupMenu::Options()
                           .withTargetComponent(pad)
                           .withParentComponent(getTopLevelComponent()),
                       [this, slot](int result) {
                           if (result <= 0)
                               return;
                           using M = RoutineDeckView::PadMenu;
                           switch (static_cast<M>(result))
                           {
                               case M::Rename:           if (onRoutineRename) onRoutineRename(slot); break;
                               case M::RemoveFromLayers: if (onRoutineRemoved) onRoutineRemoved(slot); break;
                               case M::DeleteRoutine:    if (onRoutineDeleted) onRoutineDeleted(slot); break;
                               default:
                                   if (onRoutineSet) onRoutineSet(slot, settingsChangeFor(static_cast<M>(result)));
                                   break;
                           }
                       });
}

void DeckView::showPlusMenu()
{
    if (plusTab_ == nullptr)
        return;
    juce::PopupMenu menu;
    for (const auto& item : DeckTabRow::plusMenu())
    {
        if (item.separatorBefore)
            menu.addSeparator();
        menu.addItem(static_cast<int>(item.action), item.label, item.enabled);
    }
    menu.setLookAndFeel(&getLookAndFeel());   // the app LookAndFeel: a menu parented to the top-level window would draw stock
    menu.showMenuAsync(juce::PopupMenu::Options()
                           .withTargetComponent(plusTab_.get())
                           .withParentComponent(getTopLevelComponent()),
                       [this](int result) {
                           if (result > 0 && onDeckAction)
                               onDeckAction(-1, static_cast<DeckTabRow::Action>(result));
                       });
}

void DeckView::showUndoHint(const juce::String& text)
{
    undoHintBtn_->setButtonText(text);
    // Measured like the tab text (drawButtonText's 14 pt font) + 8 px each side.
    const int w = juce::GlyphArrangement::getStringWidthInt(juce::Font(juce::FontOptions(14.0f)), text) + 16;
    undoHintBtn_->setSize(w, kDeckTabHeight);
    undoHintBtn_->setVisible(true);
    resized();

    const int gen = ++undoHintGeneration_;
    juce::Timer::callAfterDelay(kUndoHintMs, [sp = juce::Component::SafePointer<DeckView>(this), gen] {
        if (sp != nullptr && sp->undoHintGeneration_ == gen)
            sp->hideUndoHint();
    });
}

void DeckView::hideUndoHint()
{
    ++undoHintGeneration_;
    if (undoHintBtn_->isVisible())
    {
        undoHintBtn_->setVisible(false);
        resized();
    }
}
