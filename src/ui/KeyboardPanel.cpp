#include "KeyboardPanel.h"
#include "ui/LookAndFeel.h"

// ============================================================
// KeyButton
// ============================================================

KeyboardPanel::KeyButton::KeyButton(KeySlot& slot, KeyboardPanel& parent)
    : slot_(slot), parent_(parent)
{
}

void KeyboardPanel::KeyButton::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat().reduced(1.5f);

    // Background color based on state
    juce::Colour bgColor;
    if (slot_.active)
        bgColor = juce::Colour(AudioDNALookAndFeel::kAccentCyan).withAlpha(0.3f);
    else if (slot_.hasMedia() || slot_.hasEffects())
        bgColor = juce::Colour(0xff2a2a3e); // Assigned but inactive
    else
        bgColor = juce::Colour(AudioDNALookAndFeel::kSurface);

    g.setColour(bgColor);
    g.fillRoundedRectangle(bounds, 4.0f);

    // Border — brighter when active
    if (slot_.active)
        g.setColour(juce::Colour(AudioDNALookAndFeel::kAccentCyan));
    else if (slot_.hasMedia() || slot_.hasEffects())
        g.setColour(juce::Colour(AudioDNALookAndFeel::kAccentMagenta).withAlpha(0.4f));
    else
        g.setColour(juce::Colour(AudioDNALookAndFeel::kPanelBorder));

    g.drawRoundedRectangle(bounds, 4.0f, 1.0f);

    // Key character (small, top-left)
    g.setColour(juce::Colour(AudioDNALookAndFeel::kTextSecondary).withAlpha(0.5f));
    g.setFont(10.0f);
    g.drawText(juce::String::charToString(slot_.keyChar),
               bounds.reduced(3.0f, 2.0f).removeFromTop(12.0f),
               juce::Justification::topLeft);

    // Media indicator
    if (slot_.hasMedia())
    {
        g.setColour(juce::Colour(AudioDNALookAndFeel::kTextPrimary).withAlpha(0.6f));
        g.setFont(9.0f);
        juce::String mediaLabel;
        switch (slot_.mediaType)
        {
            case KeySlot::MediaType::Image:     mediaLabel = "IMG"; break;
            case KeySlot::MediaType::VideoFile:  mediaLabel = "VID"; break;
            case KeySlot::MediaType::Camera:     mediaLabel = "CAM"; break;
            default: break;
        }
        g.drawText(mediaLabel, bounds.reduced(3.0f), juce::Justification::centred);
    }

    // Effects count indicator (bottom-right)
    if (slot_.hasEffects())
    {
        g.setColour(juce::Colour(AudioDNALookAndFeel::kAccentMagenta).withAlpha(0.7f));
        g.setFont(8.0f);
        g.drawText("FX:" + juce::String(static_cast<int>(slot_.effects.size())),
                   bounds.reduced(3.0f).removeFromBottom(10.0f),
                   juce::Justification::bottomRight);
    }

    // Latch indicator (bottom-left)
    if (slot_.latched)
    {
        g.setColour(juce::Colour(AudioDNALookAndFeel::kMeterYellow).withAlpha(0.6f));
        g.setFont(8.0f);
        g.drawText("L", bounds.reduced(3.0f).removeFromBottom(10.0f),
                   juce::Justification::bottomLeft);
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
