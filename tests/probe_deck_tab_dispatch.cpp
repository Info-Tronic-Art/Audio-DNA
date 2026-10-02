// probe_deck_tab_dispatch -- s-rta-1002b ui U3.5 (ruling-ui.md AM8; gate G1b). REAL JUCE mouse and key dispatch on the
// deck tab row and its in-place rename box (BF8 "Need to be able to rename each deck with double click").
//
// NOT A CTEST. It opens a small ON-SCREEN, always-on-top, borderless window (700 x 124) near the top-left of the main
// display for about 5 s: the mac peer drops every mouse event aimed at a window that is not the topmost one on screen
// (ruling E-R1), so a headless test cannot exercise this path. It never activates the process, never calls
// toFront(true) and never touches another app. Harmony runs it, once, in Boris's logged-in session, after asking him
// (HARMONY ADOPTION 3) -- never a lane, never at the same time as another gate.
//
//   usage: probe_deck_tab_dispatch <OUT dir> [--offset X,Y]
//   exit 0 = P1-P9 all PASS; 1 = a FAIL; 3 = INCONCLUSIVE (the precheck found the tab strip covered / off screen:
//   re-run once with --offset 40,400; a second 3 is reported, never counted as a pass).
//
// Input goes in through ComponentPeer::handleMouseEvent / handleKeyPress -- the entry points the mac peer calls --
// with synthetic times (a click = button down for 40 ms; the two clicks of a double-click 100 ms apart; separate
// gestures 1 s apart). After every step one CFRunLoopRunInMode turn lets posted clicks and focus losses land.
#include <juce_gui_basics/juce_gui_basics.h>
#include "ui/DeckView.h"
#include "ui/DeckTabRow.h"
#include "ui/LookAndFeel.h"
#include "render/PngWrite.h"

#include <CoreFoundation/CoreFoundation.h>
#include <cctype>
#include <cstdio>
#include <functional>
#include <vector>

namespace
{
void pump(double seconds) { CFRunLoopRunInMode(kCFRunLoopDefaultMode, seconds, false); }

// The window's root: wants keyboard focus (MainComponent does) and records every key that reaches it.
struct Home : juce::Component
{
    Home() { setWantsKeyboardFocus(true); setOpaque(true); }
    void paint(juce::Graphics& g) override { g.fillAll(juce::Colours::black); }
    bool keyPressed(const juce::KeyPress& k) override { keys.push_back(k); return true; }
    bool received(std::function<bool(const juce::KeyPress&)> pred) const
    {
        for (const auto& k : keys)
            if (pred(k))
                return true;
        return false;
    }
    std::vector<juce::KeyPress> keys;
};

struct Counters
{
    int switches = 0, columnFires = 0, layerClears = 0, clipCallbacks = 0, renames = 0;
};

struct Driver
{
    juce::ComponentPeer* peer = nullptr;
    juce::int64 t = 0;

    void mouse(juce::Point<float> p, juce::ModifierKeys mods)
    {
        peer->handleMouseEvent(juce::MouseInputSource::InputSourceType::mouse, p, mods,
                               juce::MouseInputSource::defaultPressure, juce::MouseInputSource::defaultOrientation, t);
    }
    // One left click, gapMs after the previous event: move, down, 40 ms, up.
    void click(juce::Point<float> p, int gapMs)
    {
        t += gapMs;
        mouse(p, juce::ModifierKeys());
        mouse(p, juce::ModifierKeys(juce::ModifierKeys::leftButtonModifier));
        t += 40;
        mouse(p, juce::ModifierKeys());
        pump(0.3);
    }
    void doubleClick(juce::Point<float> p)
    {
        click(p, 1000);
        click(p, 100);
    }
    void key(const juce::KeyPress& k)
    {
        peer->handleKeyUpOrDown(true);
        peer->handleKeyPress(k);
        peer->handleKeyUpOrDown(false);
        pump(0.3);
    }
    void type(const juce::String& text)
    {
        for (auto c : text)
        {
            const bool upper = std::isupper(static_cast<int>(c)) != 0;
            key(juce::KeyPress(std::toupper(static_cast<int>(c)),
                               upper ? juce::ModifierKeys(juce::ModifierKeys::shiftModifier) : juce::ModifierKeys(),
                               c));
        }
    }
};

juce::TextEditor* visibleEditor(DeckView& dv)
{
    for (int i = 0; i < dv.getNumChildComponents(); ++i)
        if (auto* ed = dynamic_cast<juce::TextEditor*>(dv.getChildComponent(i)))
            if (ed->isVisible())
                return ed;
    return nullptr;
}

int failures = 0;
void row(const char* name, bool ok, const juce::String& info)
{
    if (!ok)
        ++failures;
    std::printf("%s %s | %s\n", name, ok ? "PASS" : "FAIL", info.toRawUTF8());
    std::fflush(stdout);
}
} // namespace

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        std::printf("usage: probe_deck_tab_dispatch <OUT dir> [--offset X,Y]\n");
        return 1;
    }
    const juce::File out(juce::File::getCurrentWorkingDirectory().getChildFile(argv[1]));
    out.createDirectory();
    juce::Point<int> offset(40, 60);
    for (int i = 2; i + 1 < argc; ++i)
        if (juce::String(argv[i]) == "--offset")
        {
            const auto xy = juce::StringArray::fromTokens(argv[i + 1], ",", "");
            if (xy.size() == 2)
                offset = { xy[0].getIntValue(), xy[1].getIntValue() };
        }

    int exitCode = 0;
    {
        juce::ScopedJuceInitialiser_GUI gui;
        AudioDNALookAndFeel laf;
        juce::LookAndFeel::setDefaultLookAndFeel(&laf);
        {
            // Decks A B C, 3 layers x 12 columns, A showing.
            Composition comp;
            comp.initDefault();
            comp.decks[0].name = "A";
            comp.addDeck("B");
            comp.addDeck("C");
            comp.activeDeckIndex = 0;

            Home home;
            DeckView dv;
            Counters n;
            // MainComponent's shapes: a switch rebuilds the grid (MainComponent.cpp handleDeckSwitch); a rename writes the
            // name and relabels; every close hands the keyboard home.
            dv.onDeckSwitched = [&](int i) { ++n.switches; comp.activeDeckIndex = i; dv.rebuildGrid(); };
            dv.onDeckRenamed = [&](int i, const juce::String& name) {
                ++n.renames;
                comp.decks[static_cast<size_t>(i)].name = name.toStdString();
                dv.refresh();
            };
            dv.onRenameClosed = [&] { home.grabKeyboardFocus(); };
            dv.onColumnTriggered = [&](int) { ++n.columnFires; };
            dv.onLayerClearClip = [&](int) { ++n.layerClears; };
            dv.onClipTriggered = [&](int, int) { ++n.clipCallbacks; };
            dv.onClipSelected = [&](int, int, bool) { ++n.clipCallbacks; };

            // Only the bottom layer row (layer 0) and the tab row show: DeckView at (0, -236) inside a 700 x 124 Home.
            home.setSize(700, 124);
            dv.setBounds(0, -236, 700, 356);
            dv.setComposition(&comp);
            home.addAndMakeVisible(dv);

            const auto* primary = juce::Desktop::getInstance().getDisplays().getPrimaryDisplay();
            const auto area = primary != nullptr ? primary->userArea : juce::Rectangle<int>(0, 0, 1280, 800);
            home.setTopLeftPosition(area.getPosition() + offset);
            home.setAlwaysOnTop(true);
            home.addToDesktop(0);   // borderless; never toFront(true), never activated
            home.setVisible(true);
            pump(0.4);

            auto* peer = home.getPeer();
            Driver d;
            d.peer = peer;
            d.t = juce::Time::currentTimeMillis();

            // Tab centres in Home (= peer) coordinates, from the live layout.
            auto tabCentre = [&](int i) {
                const auto L = DeckTabRow::layout(dv.getWidth(), static_cast<int>(comp.decks.size()));
                const int y = dv.getHeight() - 12 + dv.getY();   // the 24-px tab row is DeckView's last row
                return juce::Point<float>(static_cast<float>(L.tabs[static_cast<size_t>(i)].x + L.tabs[static_cast<size_t>(i)].w / 2),
                                          static_cast<float>(y));
            };

            bool visibleOk = peer != nullptr;
            for (int i = 0; i < 3 && visibleOk; ++i)
                visibleOk = peer->contains(tabCentre(i).roundToInt(), true);
            if (!visibleOk)
            {
                std::printf("PRECHECK INCONCLUSIVE: the tab strip is not the topmost window at offset %d,%d\n",
                            offset.x, offset.y);
                exitCode = 3;
            }
            else
            {
                auto focusedIsHome = [&] { return juce::Component::getCurrentlyFocusedComponent() == &home; };
                auto expectBox = [&](int i) {
                    const auto tab = DeckTabRow::layout(dv.getWidth(), static_cast<int>(comp.decks.size())).tabs[static_cast<size_t>(i)];
                    const auto r = DeckTabRow::editorRect(tab, dv.getWidth());
                    return juce::Rectangle<int>(r.x, dv.getHeight() - 24, r.w, 24);
                };

                // P1 click C (not showing) -> 1 switch, C showing, 1 rebuild.
                int b = dv.tabRowBuilds();
                d.click(tabCentre(2), 1000);
                row("P1", n.switches == 1 && comp.activeDeckIndex == 2 && dv.tabRowBuilds() == b + 1,
                    "switches " + juce::String(n.switches) + " active " + juce::String(comp.activeDeckIndex.load())
                        + " builds +" + juce::String(dv.tabRowBuilds() - b));

                // P2 click C again (showing) -> nothing.
                b = dv.tabRowBuilds();
                d.click(tabCentre(2), 1000);
                row("P2", n.switches == 1 && dv.tabRowBuilds() == b,
                    "switches " + juce::String(n.switches) + " builds +" + juce::String(dv.tabRowBuilds() - b));

                // P3 double-click A (not showing) -> A showing, no box; then 'q' reaches Home.
                d.doubleClick(tabCentre(0));
                const bool noBox = visibleEditor(dv) == nullptr;
                home.keys.clear();
                d.type("q");
                row("P3", comp.activeDeckIndex == 0 && noBox
                              && home.received([](const juce::KeyPress& k) { return k.getTextCharacter() == 'q'; }),
                    "active " + juce::String(comp.activeDeckIndex.load()) + " box " + juce::String(noBox ? "none" : "OPEN")
                        + " home keys " + juce::String(static_cast<int>(home.keys.size())));

                // P4 double-click A (showing) -> the box, "A" all selected, over tab A. Capture C15.
                d.doubleClick(tabCentre(0));
                auto* ed = visibleEditor(dv);
                const bool p4 = ed != nullptr && ed->getText() == "A" && ed->getHighlightedRegion() == juce::Range<int>(0, 1)
                             && ed->getBounds() == expectBox(0);
                row("P4", p4, ed == nullptr ? juce::String("no box")
                                            : "text '" + ed->getText() + "' bounds " + ed->getBounds().toString()
                                                  + " expected " + expectBox(0).toString() + " focused "
                                                  + juce::String(ed->hasKeyboardFocus(true) ? "box" : "elsewhere"));
                const auto shot = out.getChildFile("probe-dispatch-P4.png");
                const bool saved = PngWrite::writeReplacing(home.createComponentSnapshot(home.getLocalBounds()), shot);
                std::printf("C15 %s %s\n", saved ? "saved" : "NOT SAVED", shot.getFullPathName().toRawUTF8());

                // P5 type "Intro", Return -> renamed, box hidden, focus home.
                d.type("Intro");
                d.key(juce::KeyPress(juce::KeyPress::returnKey));
                row("P5", comp.decks[0].name == "Intro" && visibleEditor(dv) == nullptr && focusedIsHome(),
                    "name '" + juce::String(comp.decks[0].name) + "' box " + juce::String(visibleEditor(dv) ? "OPEN" : "hidden")
                        + " focus home " + juce::String(focusedIsHome() ? "yes" : "no"));

                // P6 Return again -> no column fires, no layer clears, no switch; Home receives it.
                const auto before = n;
                home.keys.clear();
                d.key(juce::KeyPress(juce::KeyPress::returnKey));
                row("P6", n.columnFires == before.columnFires && n.layerClears == before.layerClears
                              && n.switches == before.switches
                              && home.received([](const juce::KeyPress& k) { return k.isKeyCode(juce::KeyPress::returnKey); }),
                    "column fires +" + juce::String(n.columnFires - before.columnFires) + " layer clears +"
                        + juce::String(n.layerClears - before.layerClears) + " home keys "
                        + juce::String(static_cast<int>(home.keys.size())));

                // P7 double-click A, 'K', then a real click on the visible cell (layer 0, column 3) -> kept, focus home.
                d.doubleClick(tabCentre(0));
                d.type("K");
                const int clipsBefore = n.clipCallbacks;
                d.click(juce::Point<float>(250.0f + 3 * 90 + 45, 60.0f), 1000);
                row("P7", n.clipCallbacks == clipsBefore + 1 && comp.decks[0].name == "K" && focusedIsHome(),
                    "clip callbacks +" + juce::String(n.clipCallbacks - clipsBefore) + " name '"
                        + juce::String(comp.decks[0].name) + "' focus home " + juce::String(focusedIsHome() ? "yes" : "no"));

                // P8 double-click A, 'X', Tab -> kept; Tab never reaches Home; focus home.
                d.doubleClick(tabCentre(0));
                d.type("X");
                home.keys.clear();
                d.key(juce::KeyPress(juce::KeyPress::tabKey));
                const bool tabReachedHome = home.received([](const juce::KeyPress& k) { return k.isKeyCode(juce::KeyPress::tabKey); });
                row("P8", comp.decks[0].name == "X" && !tabReachedHome && focusedIsHome(),
                    "name '" + juce::String(comp.decks[0].name) + "' tab reached home " + juce::String(tabReachedHome ? "YES" : "no")
                        + " focus home " + juce::String(focusedIsHome() ? "yes" : "no"));

                // P9 double-click A, 'Y', Esc -> unchanged; focus home.
                d.doubleClick(tabCentre(0));
                d.type("Y");
                d.key(juce::KeyPress(juce::KeyPress::escapeKey));
                row("P9", comp.decks[0].name == "X" && visibleEditor(dv) == nullptr && focusedIsHome(),
                    "name '" + juce::String(comp.decks[0].name) + "' focus home " + juce::String(focusedIsHome() ? "yes" : "no"));

                exitCode = failures == 0 ? 0 : 1;
                std::printf("probe_deck_tab_dispatch: %d FAIL of 9 rows\n", failures);
            }

            home.setVisible(false);
            home.removeFromDesktop();
            dv.setComposition(nullptr);
        }
        juce::LookAndFeel::setDefaultLookAndFeel(nullptr);
    }
    return exitCode;
}
