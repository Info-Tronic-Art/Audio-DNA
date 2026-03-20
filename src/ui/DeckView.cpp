#include "ui/DeckView.h"

DeckView::DeckView()
{
    setOpaque(false);

    // Grid viewport for scrollable content
    gridContent_ = std::make_unique<juce::Component>();
    gridViewport_.setViewedComponent(gridContent_.get(), false);
    gridViewport_.setScrollBarsShown(true, true);
    addAndMakeVisible(gridViewport_);
}

void DeckView::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff1a1a1a));
}

void DeckView::resized()
{
    auto area = getLocalBounds();

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

    // Deck tabs immediately after the grid (attached to bottom of last layer)
    auto tabArea = area.removeFromTop(kDeckTabHeight);
    int tabX = 0;
    for (auto& tab : deckTabs_)
    {
        tab->setBounds(tabX, tabArea.getY(), 100, kDeckTabHeight);
        tabX += 102;
    }

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
        strip->setLayer(layer, layerIdx);

        // Wire callbacks
        strip->onSelect = [this](int idx) {
            selectLayer(idx);
            if (onLayerSelected) onLayerSelected(idx);
        };
        strip->onClearClip = [this](int idx) {
            if (!composition_) return;
            auto* d = composition_->getActiveDeck();
            if (!d) return;
            if (auto* l = d->getLayer(idx))
            {
                l->clearActiveClip();
                refresh();
            }
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
            cell->setClip(layer->getClipAt(col));
            cell->setActive(layer->activeClipColumn == col);

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

            gridContent_->addAndMakeVisible(cell.get());
            layerCells[static_cast<size_t>(col)] = std::move(cell);
        }
    }

    // Column trigger buttons
    setupColumnTriggers();

    // Deck tabs
    setupDeckTabs();

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
                layerCells[col]->setActive(layer->activeClipColumn == static_cast<int>(col));
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

    // Update deck tabs
    for (size_t i = 0; i < deckTabs_.size(); ++i)
    {
        bool isActive = static_cast<int>(i) == composition_->activeDeckIndex;
        deckTabs_[i]->setColour(
            juce::TextButton::buttonColourId,
            isActive ? juce::Colour(0xff3a5a4a) : juce::Colour(0xff2a2a2a));
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

    int numLayers = deck->getNumLayers();
    int rowHeight = kCellHeight + kCellGap;
    return kColumnTriggerHeight + rowHeight * numLayers + kDeckTabHeight;
}

void DeckView::layoutGrid()
{
    if (!composition_) return;
    auto* deck = composition_->getActiveDeck();
    if (!deck) return;

    int numLayers = deck->getNumLayers();
    int numCols = deck->numColumns;
    int rowHeight = kCellHeight + kCellGap;

    // Total content size
    int contentWidth = kLayerStripWidth + (kCellWidth + kCellGap) * numCols + 30;
    int contentHeight = rowHeight * numLayers;
    gridContent_->setSize(contentWidth, contentHeight);

    for (int displayRow = 0; displayRow < static_cast<int>(layerStrips_.size()); ++displayRow)
    {
        int y = displayRow * rowHeight;

        // Layer strip on the left
        layerStrips_[static_cast<size_t>(displayRow)]->setBounds(
            0, y, kLayerStripWidth, kCellHeight);

        // Clip cells
        auto& layerCells = clipCells_[static_cast<size_t>(displayRow)];
        for (int col = 0; col < static_cast<int>(layerCells.size()); ++col)
        {
            int x = kLayerStripWidth + col * (kCellWidth + kCellGap);
            if (layerCells[static_cast<size_t>(col)])
                layerCells[static_cast<size_t>(col)]->setBounds(x, y, kCellWidth, kCellHeight);
        }
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

    if (!composition_) return;

    for (size_t i = 0; i < composition_->decks.size(); ++i)
    {
        auto& deck = composition_->decks[i];
        auto btn = std::make_unique<juce::TextButton>(juce::String(deck.name));
        btn->setColour(juce::TextButton::buttonColourId, juce::Colour(0xff2a2a2a));
        btn->setColour(juce::TextButton::textColourOffId, juce::Colour(0xffcccccc));

        int capturedIdx = static_cast<int>(i);
        btn->onClick = [this, capturedIdx] {
            if (onDeckSwitched)
                onDeckSwitched(capturedIdx);
        };

        addAndMakeVisible(btn.get());
        deckTabs_.push_back(std::move(btn));
    }
}
