#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "ui/RoutineDeckView.h"
#include <functional>

// RoutinePad -- s-rta-0927 routine display, slice A (plan-routine-display-A.md 2.1): one of the eight pads in
// the deck's ROUTINES row, 90x22, directly above column N. Custom-painted (a TextButton would clip at the
// LookAndFeel's fixed 14 pt font): "1 Drop" on the left, a teal frame while waiting, a thick teal frame and a
// teal sweep with bar ticks and "5/8" while playing (a "back to the start" mark left of it while a press-again
// restart waits for its line), "LOOP" on an idle looping pad, a red "!" when something
// could not be restored, everything at 50 % when the routine plays on another deck.
//
// A pad has ONE press action, Fire (restart while playing, a no-op while waiting) -- there is deliberately no
// stop callback. Right-click (or Ctrl-click) opens the settings menu.
class RoutinePad : public juce::Component,
                   public juce::SettableTooltipClient
{
public:
    explicit RoutinePad(int slot);

    // Stores the pad's view; sets the tooltip when its text changed and repaints when what it paints changed.
    void setSpec(const RoutineDeckView::Pad& spec);
    const RoutineDeckView::Pad& getSpec() const { return spec_; }
    int getSlot() const { return slot_; }

    // s-rta-0929 g4cpu (Pitfall 57 rule 2): everything paint() draws, AS it draws it -- the sweep as a pixel width, never
    // progress01 (a new float every 30 Hz tick while a routine plays, which made the pad repaint 30 times a second and
    // its 90x22 rect union with the strips' and the wheel's repaints into one tall pass). setSpec() repaints only when
    // this changes at the pad's current width. Pure; public for tests/test_routine_pad_paint_key.cpp.
    struct PaintKey
    {
        int number = 0;
        juce::String name;
        RoutineDeckView::State state = RoutineDeckView::State::Empty;
        bool onShownDeck = true, loop = false, warning = false, restartPending = false;
        int sweepW = 0, bar = 0, barsTotal = 0;
        bool operator==(const PaintKey&) const = default;
    };
    static PaintKey paintKeyOf(const RoutineDeckView::Pad& spec, int width);

    std::function<void(int slot)> onFire;
    std::function<void(int slot)> onContextMenu;

    void paint(juce::Graphics& g) override;
    void mouseDown(const juce::MouseEvent& e) override;

private:
    void paintContent(juce::Graphics& g);

    int slot_ = -1;
    RoutineDeckView::Pad spec_;
    int lastPaintedSweepW_ = -1;   // s-rta-0929 g4cpu: the painted-sweep witness (UiPaintCounters routinePadSweepPaints)

    static constexpr juce::uint32 kPadEmpty  = 0xff2a2a2a;
    static constexpr juce::uint32 kPadIdle   = 0xff333333;
    static constexpr juce::uint32 kHairline  = 0xff1a1a1a;
    static constexpr juce::uint32 kTeal      = 0xff4a9a8a;
    static constexpr juce::uint32 kText      = 0xffe0e0e0;
    static constexpr juce::uint32 kLabel     = 0xff888888;
    static constexpr juce::uint32 kWarning   = 0xffcc3333;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RoutinePad)
};
