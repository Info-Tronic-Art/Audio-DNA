#include "ui/RoutinePad.h"
#include <cmath>

namespace
{
    bool samePad(const RoutineDeckView::Pad& a, const RoutineDeckView::Pad& b)
    {
        return a.number == b.number && a.name == b.name && a.state == b.state && a.onShownDeck == b.onShownDeck
            && a.loop == b.loop && a.progress01 == b.progress01 && a.bar == b.bar && a.barsTotal == b.barsTotal
            && a.warning == b.warning && a.restartPending == b.restartPending;
    }
}

RoutinePad::RoutinePad(int slot) : slot_(slot)
{
    spec_.number = slot + 1;
    setComponentID("routinePad" + juce::String(slot));
    setOpaque(true);
}

void RoutinePad::setSpec(const RoutineDeckView::Pad& spec)
{
    const bool changed = !samePad(spec_, spec);
    if (spec.tooltip != spec_.tooltip)
        setTooltip(spec.tooltip);
    spec_ = spec;
    if (changed)
        repaint();
}

void RoutinePad::paint(juce::Graphics& g)
{
    using State = RoutineDeckView::State;
    const bool dimmed = !spec_.onShownDeck && (spec_.state == State::Waiting || spec_.state == State::Playing);

    // The row behind a dimmed pad is the deck's background, so the 50 % layer reads as "elsewhere".
    g.fillAll(juce::Colour(kHairline));
    if (dimmed)
    {
        g.beginTransparencyLayer(0.5f);
        paintContent(g);
        g.endTransparencyLayer();
    }
    else
    {
        paintContent(g);
    }
}

void RoutinePad::paintContent(juce::Graphics& g)
{
    using State = RoutineDeckView::State;
    const auto bounds = getLocalBounds();
    const int w = bounds.getWidth(), h = bounds.getHeight();

    // 1. fill
    g.setColour(juce::Colour(spec_.state == State::Empty ? kPadEmpty : kPadIdle));
    g.fillRect(bounds);

    // 2. the sweep and its bar ticks
    if (spec_.state == State::Playing)
    {
        const int sweepW = juce::roundToInt(static_cast<float>(w) * juce::jlimit(0.0f, 1.0f, spec_.progress01));
        g.setColour(juce::Colour(kTeal).withAlpha(0.3f));
        g.fillRect(0, 0, sweepW, h);
        if (spec_.barsTotal > 1 && spec_.barsTotal <= 8)
        {
            g.setColour(juce::Colour(kHairline));
            for (int k = 1; k < spec_.barsTotal; ++k)
                g.fillRect(juce::roundToInt(static_cast<float>(w * k) / static_cast<float>(spec_.barsTotal)), 0, 1, h);
        }
    }

    // 3. frame
    if (spec_.state == State::Playing)
    {
        g.setColour(juce::Colour(kTeal));
        g.drawRect(bounds, 2);
    }
    else
    {
        g.setColour(juce::Colour(spec_.state == State::Waiting ? kTeal : kHairline));
        g.drawRect(bounds, 1);
    }

    // 4. left: "!" (warning), the number, the name
    int x = 4;
    if (spec_.warning)
    {
        g.setColour(juce::Colour(kWarning));
        g.setFont(juce::Font(juce::FontOptions(10.0f, juce::Font::bold)));
        g.drawText("!", juce::Rectangle<int>(x, (h - 12) / 2, 12, 12), juce::Justification::centred, false);
        x = 20;
    }
    const juce::String number(spec_.number);
    const juce::Font numberFont(juce::FontOptions(9.0f, juce::Font::bold));
    const int numberW = std::max(8, juce::GlyphArrangement::getStringWidthInt(numberFont, number));
    g.setColour(juce::Colour(kLabel).withAlpha(spec_.state == State::Empty ? 0.4f : 1.0f));
    g.setFont(numberFont);
    g.drawText(number, juce::Rectangle<int>(x, 0, numberW, h), juce::Justification::centredLeft, false);
    const int nameX = x + numberW + 4;

    // 5. right slot (inset 4): "5/8" while playing, "LOOP" on an idle looping pad
    int rightX = w - 4;
    if (spec_.state == State::Playing && spec_.barsTotal > 0)
    {
        const juce::String digits = juce::String(spec_.bar) + "/" + juce::String(spec_.barsTotal);
        const juce::Font mono(juce::FontOptions(juce::Font::getDefaultMonospacedFontName(), 9.0f, juce::Font::plain));
        const int dw = juce::GlyphArrangement::getStringWidthInt(mono, digits);
        rightX = w - 4 - dw;
        g.setColour(juce::Colour(kText));
        g.setFont(mono);
        g.drawText(digits, juce::Rectangle<int>(rightX, 0, dw, h), juce::Justification::centredRight, false);

        if (spec_.restartPending)
        {
            // s-rta-0927 fix round: pressed again -- a "back to the start" mark (a bar + a left-pointing
            // triangle, drawn: no glyph, Pitfall 6) left of the digits until the restart lands on its line.
            const float right = static_cast<float>(rightX - 3), cy = static_cast<float>(h) * 0.5f;
            juce::Path mark;
            mark.addRectangle(right - 8.0f, cy - 4.0f, 1.5f, 8.0f);
            mark.addTriangle(right - 6.0f, cy, right, cy - 4.0f, right, cy + 4.0f);
            g.fillPath(mark);
            rightX -= 12;
        }
    }
    else if (spec_.state == State::Idle && spec_.loop)
    {
        const juce::Font tag(juce::FontOptions(8.0f));
        const int tw = juce::GlyphArrangement::getStringWidthInt(tag, "LOOP");
        rightX = w - 4 - tw;
        g.setColour(juce::Colour(kLabel));
        g.setFont(tag);
        g.drawText("LOOP", juce::Rectangle<int>(rightX, 0, tw, h), juce::Justification::centredRight, false);
    }

    if (spec_.state != State::Empty && spec_.name.isNotEmpty())
    {
        const int nameW = rightX - 4 - nameX;
        if (nameW > 0)
        {
            g.setColour(juce::Colour(kText));
            g.setFont(juce::Font(juce::FontOptions(10.0f)));
            g.drawText(spec_.name, juce::Rectangle<int>(nameX, 0, nameW, h), juce::Justification::centredLeft, true);
        }
    }
}

void RoutinePad::mouseDown(const juce::MouseEvent& e)
{
    if (spec_.state == RoutineDeckView::State::Empty)
        return;
    // A JUCE Component gets mouseDown for ANY button: a right-click (or Ctrl-click) is the settings menu and
    // never fires (the DeckTabButton idiom). Fire on the press, not the release -- a pad, lowest latency.
    if (e.mods.isPopupMenu())
    {
        if (onContextMenu) onContextMenu(slot_);
        return;
    }
    if (e.mods.isLeftButtonDown() && onFire)
        onFire(slot_);
}
