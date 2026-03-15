#include "KeyboardPanel.h"
#include "ui/LookAndFeel.h"

// ============================================================
// KeyButton
// ============================================================

KeyboardPanel::KeyButton::KeyButton(KeySlot& slot, KeyboardPanel& parent)
    : slot_(slot), parent_(parent)
{
}

void KeyboardPanel::KeyButton::updateThumbnail()
{
    juce::String currentPath;
    if (slot_.mediaType == KeySlot::MediaType::Image && slot_.mediaFile.existsAsFile())
        currentPath = slot_.mediaFile.getFullPathName();

    if (currentPath == lastMediaPath_)
        return; // Already up to date

    lastMediaPath_ = currentPath;
    if (currentPath.isEmpty())
    {
        thumbnail_ = juce::Image();
        return;
    }

    auto img = juce::ImageFileFormat::loadFrom(slot_.mediaFile);
    if (img.isValid())
    {
        // Scale to thumbnail size (max 80x60)
        int thumbW = 80;
        int thumbH = 60;
        float aspect = static_cast<float>(img.getWidth()) / static_cast<float>(img.getHeight());
        if (aspect > static_cast<float>(thumbW) / static_cast<float>(thumbH))
            thumbH = static_cast<int>(static_cast<float>(thumbW) / aspect);
        else
            thumbW = static_cast<int>(static_cast<float>(thumbH) * aspect);
        thumbnail_ = img.rescaled(thumbW, thumbH, juce::Graphics::mediumResamplingQuality);
    }
    else
    {
        thumbnail_ = juce::Image();
    }
}

void KeyboardPanel::KeyButton::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat().reduced(1.5f);

    // Update thumbnail if needed
    updateThumbnail();

    // Background color based on state
    juce::Colour bgColor;
    if (slot_.active)
        bgColor = juce::Colour(AudioDNALookAndFeel::kAccentCyan).withAlpha(0.3f);
    else if (slot_.hasMedia() || slot_.hasEffects())
        bgColor = juce::Colour(0xff2a2a3e);
    else
        bgColor = juce::Colour(AudioDNALookAndFeel::kSurface);

    g.setColour(bgColor);
    g.fillRoundedRectangle(bounds, 4.0f);

    // Draw thumbnail — zoom to fill 90% of key area (crop, don't letterbox)
    if (thumbnail_.isValid())
    {
        auto imgArea = bounds.reduced(bounds.getWidth() * 0.05f, bounds.getHeight() * 0.05f);
        float imgAspect = static_cast<float>(thumbnail_.getWidth()) /
                          static_cast<float>(thumbnail_.getHeight());
        float areaAspect = imgArea.getWidth() / imgArea.getHeight();

        // Zoom to fill: use the LARGER scale factor so image covers the area
        int srcX, srcY, srcW, srcH;
        if (imgAspect > areaAspect)
        {
            // Image is wider — crop sides
            srcH = thumbnail_.getHeight();
            srcW = static_cast<int>(static_cast<float>(srcH) * areaAspect);
            srcX = (thumbnail_.getWidth() - srcW) / 2;
            srcY = 0;
        }
        else
        {
            // Image is taller — crop top/bottom
            srcW = thumbnail_.getWidth();
            srcH = static_cast<int>(static_cast<float>(srcW) / areaAspect);
            srcX = 0;
            srcY = (thumbnail_.getHeight() - srcH) / 2;
        }

        g.setOpacity(slot_.active ? 1.0f : 0.5f);
        g.drawImage(thumbnail_,
                     static_cast<int>(imgArea.getX()), static_cast<int>(imgArea.getY()),
                     static_cast<int>(imgArea.getWidth()), static_cast<int>(imgArea.getHeight()),
                     srcX, srcY, srcW, srcH);
        g.setOpacity(1.0f);
    }

    // Border — brighter when active
    if (slot_.active)
        g.setColour(juce::Colour(AudioDNALookAndFeel::kAccentCyan));
    else if (slot_.hasMedia() || slot_.hasEffects())
        g.setColour(juce::Colour(AudioDNALookAndFeel::kAccentMagenta).withAlpha(0.4f));
    else
        g.setColour(juce::Colour(AudioDNALookAndFeel::kPanelBorder));

    g.drawRoundedRectangle(bounds, 4.0f, 1.0f);

    // Key character (small, top-left, with background for readability)
    {
        auto charBounds = bounds.reduced(3.0f, 2.0f).removeFromTop(12.0f).removeFromLeft(12.0f);
        g.setColour(juce::Colour(0x88000000)); // semi-transparent black
        g.fillRoundedRectangle(charBounds, 2.0f);
        g.setColour(juce::Colour(AudioDNALookAndFeel::kTextSecondary).withAlpha(0.8f));
        g.setFont(9.0f);
        g.drawText(juce::String::charToString(slot_.keyChar), charBounds,
                   juce::Justification::centred);
    }

    // Effects count indicator (bottom-right)
    if (slot_.hasEffects())
    {
        auto fxBounds = bounds.reduced(3.0f).removeFromBottom(11.0f).removeFromRight(25.0f);
        g.setColour(juce::Colour(0x88000000));
        g.fillRoundedRectangle(fxBounds, 2.0f);
        g.setColour(juce::Colour(AudioDNALookAndFeel::kAccentMagenta).withAlpha(0.8f));
        g.setFont(8.0f);
        g.drawText("FX:" + juce::String(static_cast<int>(slot_.effects.size())),
                   fxBounds, juce::Justification::centred);
    }

    // Latch indicator (bottom-left)
    if (slot_.latched)
    {
        auto lBounds = bounds.reduced(3.0f).removeFromBottom(11.0f).removeFromLeft(12.0f);
        g.setColour(juce::Colour(0x88000000));
        g.fillRoundedRectangle(lBounds, 2.0f);
        g.setColour(juce::Colour(AudioDNALookAndFeel::kMeterYellow).withAlpha(0.8f));
        g.setFont(8.0f);
        g.drawText("L", lBounds, juce::Justification::centred);
    }
}

void KeyboardPanel::KeyButton::mouseDown(const juce::MouseEvent& e)
{
    // Right-click always opens editor
    if (e.mods.isRightButtonDown())
    {
        if (parent_.onKeyClicked)
            parent_.onKeyClicked(slot_);
        return;
    }

    if (slot_.isEmpty())
    {
        // Empty key — open editor on click
        if (parent_.onKeyClicked)
            parent_.onKeyClicked(slot_);
        return;
    }

    // Shift+click = latch toggle (even if latch checkbox is off)
    if (e.mods.isShiftDown())
    {
        if (slot_.active)
        {
            parent_.layout_.deactivateKey(slot_);
        }
        else
        {
            parent_.layout_.activateKey(slot_);
            slot_.shiftLatched = true;
        }
        repaint();
        return;
    }

    if (slot_.latched)
    {
        // Toggle mode
        parent_.layout_.toggleKey(slot_);
    }
    else
    {
        // Momentary mode — activate on press
        parent_.layout_.activateKey(slot_);
    }
    repaint();
}

void KeyboardPanel::KeyButton::mouseUp(const juce::MouseEvent&)
{
    if (!slot_.latched && slot_.active && !slot_.isEmpty())
    {
        // Momentary mode — deactivate on release
        parent_.layout_.deactivateKey(slot_);
        repaint();
    }
}

// ============================================================
// KeyboardPanel
// ============================================================

KeyboardPanel::KeyboardPanel(KeyboardLayout& layout)
    : layout_(layout)
{
    for (int i = 0; i < KeyboardLayout::kNumKeys; ++i)
    {
        buttons_[static_cast<size_t>(i)] =
            std::make_unique<KeyButton>(layout_.keys[static_cast<size_t>(i)], *this);
        addAndMakeVisible(buttons_[static_cast<size_t>(i)].get());
    }
}

void KeyboardPanel::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    g.setColour(juce::Colour(AudioDNALookAndFeel::kBackground));
    g.fillRect(bounds);

    // Top border
    g.setColour(juce::Colour(AudioDNALookAndFeel::kPanelBorder));
    g.drawHorizontalLine(0, bounds.getX(), bounds.getRight());
}

void KeyboardPanel::resized()
{
    auto area = getLocalBounds().reduced(4);

    const int rowHeight = area.getHeight() / KeyboardLayout::kNumRows;
    const float keyWidth = static_cast<float>(area.getWidth()) / static_cast<float>(KeyboardLayout::kNumCols);

    // Row offsets to mimic staggered keyboard layout
    const float rowOffsets[KeyboardLayout::kNumRows] = { 0.0f, 8.0f, 16.0f, 24.0f };

    for (int r = 0; r < KeyboardLayout::kNumRows; ++r)
    {
        for (int c = 0; c < KeyboardLayout::kNumCols; ++c)
        {
            int idx = r * KeyboardLayout::kNumCols + c;
            float x = area.getX() + rowOffsets[r] + static_cast<float>(c) * keyWidth;
            float y = static_cast<float>(area.getY() + r * rowHeight);
            float w = keyWidth;

            // Last key in row takes remaining space (account for offset)
            if (c == KeyboardLayout::kNumCols - 1)
                w = static_cast<float>(area.getRight()) - x;

            buttons_[static_cast<size_t>(idx)]->setBounds(
                static_cast<int>(x), static_cast<int>(y),
                static_cast<int>(w), rowHeight);
        }
    }
}

void KeyboardPanel::keyActivated(KeySlot& key)
{
    int idx = key.row * KeyboardLayout::kNumCols + key.col;
    if (idx >= 0 && idx < KeyboardLayout::kNumKeys)
        buttons_[static_cast<size_t>(idx)]->repaint();
}

void KeyboardPanel::keyDeactivated(KeySlot& key)
{
    int idx = key.row * KeyboardLayout::kNumCols + key.col;
    if (idx >= 0 && idx < KeyboardLayout::kNumKeys)
        buttons_[static_cast<size_t>(idx)]->repaint();
}

void KeyboardPanel::refresh()
{
    for (auto& btn : buttons_)
        btn->repaint();
}
