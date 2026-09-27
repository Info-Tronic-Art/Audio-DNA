// test_lookandfeel_square -- s-rta-0926b plan6 fix round: the app draws no rounded corners, anywhere
// (BORIS_DECISIONS "Rejected: Rounded corners (anywhere, ever)"). Two surfaces the deck tab row and the
// library brought to the front broke that rule:
//   * every PopupMenu (the "+" menu, a deck tab's menu, a library row's menu) -- drawPopupMenuBackground
//     filled and outlined a 4-px rounded rectangle;
//   * the dialogs (Rename Deck, the replace confirm, Delete from Library) -- an AlertWindow is its own
//     top-level window, so it never inherits MainComponent's LookAndFeel and drew with the stock
//     LookAndFeel_V4 (rounded panel, rounded buttons, JUCE's alert icons).
// Headless: each surface is painted into an image with the app's real AudioDNALookAndFeel and the corner
// pixel is sampled -- a square panel paints its 1-px border there, a rounded one leaves it unpainted.
#include <catch2/catch_test_macros.hpp>
#include <juce_gui_basics/juce_gui_basics.h>
#include "ui/LookAndFeel.h"
#include <memory>

namespace
{
    juce::String argb(juce::Colour c) { return c.toDisplayString(true); }
}

TEST_CASE("PopupMenu background is square: the border reaches the corner pixel", "[lookandfeel][s-rta-0926b]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    AudioDNALookAndFeel laf;

    juce::Image img(juce::Image::ARGB, 120, 60, true);
    {
        juce::Graphics g(img);
        laf.drawPopupMenuBackground(g, img.getWidth(), img.getHeight());
    }

    const juce::Colour border(AudioDNALookAndFeel::kPanelBorder);
    for (const auto& p : { juce::Point<int>(0, 0), juce::Point<int>(119, 0),
                           juce::Point<int>(0, 59), juce::Point<int>(119, 59) })
    {
        INFO("corner " << p.toString() << " = " << argb(img.getPixelAt(p.x, p.y)));
        CHECK(img.getPixelAt(p.x, p.y) == border);   // RED pre-fix: ~transparent (4-px rounded corner)
    }
    // The fill is the menu surface colour, unchanged.
    CHECK(img.getPixelAt(60, 30) == juce::Colour(AudioDNALookAndFeel::kSurface));
}

TEST_CASE("an AlertWindow opened from a component that carries the app LookAndFeel draws with it: "
          "square panel, app colours, no icon",
          "[lookandfeel][s-rta-0926b]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    AudioDNALookAndFeel laf;           // NOT the default LookAndFeel -- the app only sets it on MainComponent
    juce::Component owner;             // stands in for MainComponent / CompDecksBrowser
    owner.setLookAndFeel(&laf);

    // Exactly what AlertWindow::showOkCancelBox(..., associatedComponent = owner, ...) does
    // (juce detail/juce_AlertWindowHelpers.h setUpAlert): the owner's LookAndFeel creates the window.
    std::unique_ptr<juce::AlertWindow> aw(owner.getLookAndFeel().createAlertWindow(
        "Delete from Library", "Delete \"x\" from the library?\n\nThe file will be moved to the Trash.",
        "Delete", "Cancel", {}, juce::MessageBoxIconType::NoIcon, 2, &owner));
    REQUIRE(aw != nullptr);

    CHECK(&aw->getLookAndFeel() == &laf);   // RED pre-fix: the stock default LookAndFeel_V4

    const auto shot = aw->createComponentSnapshot(aw->getLocalBounds());
    const int w = shot.getWidth(), h = shot.getHeight();
    INFO("size " << w << "x" << h << ", corners " << argb(shot.getPixelAt(0, 0)) << " "
         << argb(shot.getPixelAt(w - 1, h - 1)) << ", left edge " << argb(shot.getPixelAt(4, h / 2)));
    CHECK(shot.getPixelAt(0, 0) == juce::Colour(AudioDNALookAndFeel::kPanelBorder));           // RED: rounded
    CHECK(shot.getPixelAt(w - 1, h - 1) == juce::Colour(AudioDNALookAndFeel::kPanelBorder));   // RED: rounded
    CHECK(shot.getPixelAt(4, h / 2) == juce::Colour(AudioDNALookAndFeel::kBackground));        // RED: stock grey

    // Its buttons draw with the app LookAndFeel too: square (the corner pixel is the button border).
    auto* del = aw->getButton("Delete");
    REQUIRE(del != nullptr);
    const auto b = del->createComponentSnapshot(del->getLocalBounds());
    CHECK(b.getPixelAt(0, 0) == juce::Colour(AudioDNALookAndFeel::kPanelBorder));
}

TEST_CASE("a hand-built AlertWindow (Rename Deck) given the app LookAndFeel is square", "[lookandfeel][s-rta-0926b]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    AudioDNALookAndFeel laf;

    // The Rename Deck dialog's own construction (MainComponent::renameDeck).
    juce::AlertWindow w("Rename Deck", "", juce::MessageBoxIconType::NoIcon);
    w.setLookAndFeel(&laf);
    w.addTextEditor("name", "A");
    w.addButton("Rename", 1, juce::KeyPress(juce::KeyPress::returnKey));
    w.addButton("Cancel", 0, juce::KeyPress(juce::KeyPress::escapeKey));

    const auto shot = w.createComponentSnapshot(w.getLocalBounds());
    INFO("size " << shot.getWidth() << "x" << shot.getHeight() << ", corner " << argb(shot.getPixelAt(0, 0)));
    CHECK(shot.getPixelAt(0, 0) == juce::Colour(AudioDNALookAndFeel::kPanelBorder));   // RED pre-fix: rounded
    CHECK(shot.getPixelAt(4, shot.getHeight() / 2) == juce::Colour(AudioDNALookAndFeel::kBackground));

    w.setLookAndFeel(nullptr);
}

TEST_CASE("AudioDNALookAndFeel::installAsDefault makes an owner-less AlertWindow/menu window draw with "
          "the app LookAndFeel (the 12 showMessageBoxAsync calls with no associated component, and any "
          "PopupMenu with no in-app parent -- MacroPanel/UniversalParamControl/SignalBar)",
          "[lookandfeel][s-rta-0926b]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    AudioDNALookAndFeel laf;

    // Before install: an orphan Component (no explicit LookAndFeel, no parent) -- exactly what a
    // top-level AlertWindow/MenuWindow is at construction before it is given one -- falls to
    // whatever the process-wide JUCE default happens to be, never this app's LookAndFeel.
    {
        juce::Component orphan;
        CHECK(&orphan.getLookAndFeel() != static_cast<juce::LookAndFeel*>(&laf));
    }

    laf.installAsDefault();

    juce::Component orphan;
    CHECK(&orphan.getLookAndFeel() == static_cast<juce::LookAndFeel*>(&laf));

    // Exactly the path AlertWindow::showMessageBoxAsync/showOkCancelBox take with no associated
    // component (juce_AlertWindowHelpers.h setUpAlert: lf = LookAndFeel::getDefaultLookAndFeel()).
    std::unique_ptr<juce::AlertWindow> aw(juce::LookAndFeel::getDefaultLookAndFeel().createAlertWindow(
        "Save Deck", "Save failed: x.deck.json", "OK", {}, {},
        juce::MessageBoxIconType::WarningIcon, 1, nullptr));
    REQUIRE(aw != nullptr);
    CHECK(&aw->getLookAndFeel() == static_cast<juce::LookAndFeel*>(&laf));

    const auto shot = aw->createComponentSnapshot(aw->getLocalBounds());
    CHECK(shot.getPixelAt(0, 0) == juce::Colour(AudioDNALookAndFeel::kPanelBorder));   // RED pre-fix: rounded/absent

    laf.uninstallAsDefault();
    juce::Component after;
    CHECK(&after.getLookAndFeel() != static_cast<juce::LookAndFeel*>(&laf));   // cleared before laf goes out of scope
}

TEST_CASE("a Label whose text colour is transparent draws no text (the AlertWindow's hidden accessibility label)",
          "[lookandfeel][s-rta-0926b]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    AudioDNALookAndFeel laf;

    // juce::AlertWindow keeps a copy of its message in an invisible Label for screen readers
    // (accessibleMessageLabel: text, background and outline colours all transparentBlack). Drawn
    // visibly, it doubled every dialog's message as a second, overlapping block of text.
    juce::Label hidden("hidden", "Open Composition: Open \"Example\"?");
    hidden.setLookAndFeel(&laf);
    hidden.setColour(juce::Label::textColourId, juce::Colours::transparentBlack);
    hidden.setColour(juce::Label::backgroundColourId, juce::Colours::transparentBlack);
    hidden.setBounds(0, 0, 300, 24);

    const auto shot = hidden.createComponentSnapshot(hidden.getLocalBounds());
    int painted = 0;
    for (int y = 0; y < shot.getHeight(); ++y)
        for (int x = 0; x < shot.getWidth(); ++x)
            if (shot.getPixelAt(x, y).getAlpha() != 0)
                ++painted;
    CHECK(painted == 0);   // RED pre-fix: the text drawn in kTextPrimary

    // A label with the default colour still draws its text.
    juce::Label shown("shown", "Open Composition: Open \"Example\"?");
    shown.setLookAndFeel(&laf);
    shown.setBounds(0, 0, 300, 24);
    const auto shot2 = shown.createComponentSnapshot(shown.getLocalBounds());
    int painted2 = 0;
    for (int y = 0; y < shot2.getHeight(); ++y)
        for (int x = 0; x < shot2.getWidth(); ++x)
            if (shot2.getPixelAt(x, y).getAlpha() != 0)
                ++painted2;
    CHECK(painted2 > 0);

    hidden.setLookAndFeel(nullptr);
    shown.setLookAndFeel(nullptr);
}
