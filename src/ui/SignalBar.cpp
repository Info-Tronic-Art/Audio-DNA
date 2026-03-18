#include "SignalBar.h"

SignalBar::SignalBar(SignalRegistry& registry, FeatureBus& featureBus)
    : registry_(registry), featureBus_(featureBus)
{
    displaySnap_.clear();

    addAndMakeVisible(addButton_);
    addButton_.onClick = [this] { showAddSignalMenu(); };
    addButton_.setColour(juce::TextButton::buttonColourId,
                         juce::Colour(AudioDNALookAndFeel::kSurfaceLight));
    addButton_.setColour(juce::TextButton::textColourOffId,
                         juce::Colour(AudioDNALookAndFeel::kAccentCyan));

    // Size control buttons
    auto btnStyle = [](juce::TextButton& btn) {
        btn.setColour(juce::TextButton::buttonColourId,
                      juce::Colour(AudioDNALookAndFeel::kSurfaceLight));
        btn.setColour(juce::TextButton::textColourOffId,
                      juce::Colour(AudioDNALookAndFeel::kTextSecondary));
    };

    shrinkButton_.setButtonText(juce::CharPointer_UTF8("\xe2\x96\xb2"));  // up arrow = shrink
    growButton_.setButtonText(juce::CharPointer_UTF8("\xe2\x96\xbc"));    // down arrow = grow
    btnStyle(shrinkButton_);
    btnStyle(growButton_);
    addAndMakeVisible(shrinkButton_);
    addAndMakeVisible(growButton_);

    shrinkButton_.onClick = [this] { shrink(); };
    growButton_.onClick = [this] { grow(); };

    rebuildStrips();
    startTimerHz(30);
}

void SignalBar::rebuildStrips()
{
    strips_.clear();

    for (int i = 0; i < registry_.getNumSignals(); ++i)
    {
        auto* sig = registry_.getSignalAt(i);
        if (sig == nullptr || !sig->isVisible())
            continue;

        auto strip = std::make_unique<SignalStrip>(*sig, registry_);
        strip->setDisplaySize(displaySize_);
        strip->onSelected = [this](Signal& s) {
            if (onSignalSelected)
                onSignalSelected(s);
        };
        addAndMakeVisible(*strip);
        strips_.push_back(std::move(strip));
    }

    resized();
}

void SignalBar::setDisplaySize(SignalStrip::DisplaySize size)
{
    displaySize_ = size;
    for (auto& strip : strips_)
        strip->setDisplaySize(size);
    resized();
}

void SignalBar::shrink()
{
    switch (displaySize_)
    {
        case SignalStrip::DisplaySize::Expanded:
            setDisplaySize(SignalStrip::DisplaySize::Normal);
            break;
        case SignalStrip::DisplaySize::Normal:
            setDisplaySize(SignalStrip::DisplaySize::Minimized);
            break;
        case SignalStrip::DisplaySize::Minimized:
            break; // already smallest
    }
    if (onSizeChanged) onSizeChanged();
}

void SignalBar::grow()
{
    switch (displaySize_)
    {
        case SignalStrip::DisplaySize::Minimized:
            setDisplaySize(SignalStrip::DisplaySize::Normal);
            break;
        case SignalStrip::DisplaySize::Normal:
            setDisplaySize(SignalStrip::DisplaySize::Expanded);
            break;
        case SignalStrip::DisplaySize::Expanded:
            break; // already largest
    }
    if (onSizeChanged) onSizeChanged();
}

int SignalBar::getPreferredHeight() const
{
    switch (displaySize_)
    {
        case SignalStrip::DisplaySize::Minimized: return 26;
        case SignalStrip::DisplaySize::Normal:    return 84;
        case SignalStrip::DisplaySize::Expanded:  return -1; // -1 = fill all remaining space
    }
    return 84;
}

void SignalBar::timerCallback()
{
    // Read latest snapshot
    const FeatureSnapshot* newSnap = featureBus_.acquireRead();
    const FeatureSnapshot* snap = newSnap ? newSnap : featureBus_.getLatestRead();
    if (snap == nullptr)
        return;

    displaySnap_ = *snap;

    // Evaluate all signals with the latest snapshot
    registry_.evaluateAll(displaySnap_);

    // Update each strip with its cached value
    for (auto& strip : strips_)
    {
        float val = registry_.getCachedValue(strip->getSignal().getId());
        strip->updateValue(val);
    }

    repaint();
}

void SignalBar::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    // Background
    g.setColour(juce::Colour(AudioDNALookAndFeel::kSurface).darker(0.2f));
    g.fillRect(bounds);

    // Bottom border
    g.setColour(juce::Colour(AudioDNALookAndFeel::kPanelBorder));
    g.fillRect(bounds.getX(), bounds.getBottom() - 1.0f, bounds.getWidth(), 1.0f);
}

void SignalBar::resized()
{
    auto area = getLocalBounds().reduced(2);

    // Size control buttons at top-right
    int ctrlBtnW = 20;
    int ctrlBtnH = std::min(14, area.getHeight() / 2);
    int rightEdge = area.getRight();

    shrinkButton_.setBounds(rightEdge - ctrlBtnW, area.getY(), ctrlBtnW, ctrlBtnH);
    growButton_.setBounds(rightEdge - ctrlBtnW, area.getY() + ctrlBtnH, ctrlBtnW, ctrlBtnH);

    // Strip area (leave room for controls on the right)
    auto stripArea = area.withTrimmedRight(ctrlBtnW + 4);

    // Layout strips horizontally
    int x = stripArea.getX();
    int gap = 2;

    for (auto& strip : strips_)
    {
        int w = strip->getPreferredWidth();
        strip->setBounds(x, stripArea.getY(), w, stripArea.getHeight());
        x += w + gap;
    }

    // [+] button at the end of the strips
    int btnSize = std::min(stripArea.getHeight(), 24);
    addButton_.setBounds(x + 4, stripArea.getY() + (stripArea.getHeight() - btnSize) / 2,
                         btnSize, btnSize);
}

void SignalBar::showAddSignalMenu()
{
    juce::PopupMenu menu;
    int itemId = 1;

    // Show hidden signals that can be made visible
    for (int i = 0; i < registry_.getNumSignals(); ++i)
    {
        auto* sig = registry_.getSignalAt(i);
        if (sig == nullptr || sig->isVisible())
            continue;

        menu.addItem(itemId, juce::String(sig->getName()));
        ++itemId;
    }

    if (itemId == 1)
    {
        menu.addItem(-1, "All signals visible", false, false);
    }

    menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(&addButton_),
        [this](int result)
        {
            if (result <= 0) return;

            // Find the Nth hidden signal
            int count = 0;
            for (int i = 0; i < registry_.getNumSignals(); ++i)
            {
                auto* sig = registry_.getSignalAt(i);
                if (sig == nullptr || sig->isVisible())
                    continue;

                ++count;
                if (count == result)
                {
                    sig->setVisible(true);
                    rebuildStrips();
                    return;
                }
            }
        });
}
