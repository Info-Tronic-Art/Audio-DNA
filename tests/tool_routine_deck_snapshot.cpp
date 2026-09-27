// tool_routine_deck_snapshot -- s-rta-0927 routine display, slice A (plan-routine-display-A.md 4, "Headless
// complement"): a ctest-EXCLUDED manual tool (own main(), never registered with catch_discover_tests) that
// renders, with the app's AudioDNALookAndFeel, (a) the ROUTINES row -- a 970x22 strip holding the eight real
// RoutinePads at the deck's x (250 + i*90) -- and (b) one 250x96 LayerStrip with its routine bands, for
// synthetic RoutineEngine::Status states run through the REAL deriveRoutineDeckView, to PNGs via
// juce::Component::createComponentSnapshot -- no window, no peer. The row's corner label is painted here
// only to frame the pads (DeckView paints the real one). Run by hand:
//   ./tool_routine_deck_snapshot <output-directory>
#include <juce_gui_basics/juce_gui_basics.h>
#include "ui/LayerStrip.h"
#include "ui/LookAndFeel.h"
#include "ui/RoutinePad.h"
#include "ui/RoutineDeckView.h"
#include <array>
#include <iostream>
#include <memory>

namespace
{
    struct Row : juce::Component
    {
        std::array<std::unique_ptr<RoutinePad>, RoutineEngine::kBankSize> pads;
        juce::String note;
        Row()
        {
            for (int i = 0; i < RoutineEngine::kBankSize; ++i)
            {
                pads[static_cast<size_t>(i)] = std::make_unique<RoutinePad>(i);
                addAndMakeVisible(pads[static_cast<size_t>(i)].get());
                pads[static_cast<size_t>(i)]->setBounds(250 + i * 90, 0, 90, 22);
            }
        }
        void paint(juce::Graphics& g) override
        {
            g.fillAll(juce::Colour(0xff1a1a1a));
            const juce::Font label(juce::FontOptions(10.0f, juce::Font::bold));
            g.setColour(juce::Colour(0xff888888));
            g.setFont(label);
            g.drawText("ROUTINES", juce::Rectangle<int>(6, 0, 244, 22), juce::Justification::centredLeft, false);
            const int noteX = 6 + juce::GlyphArrangement::getStringWidthInt(label, "ROUTINES") + 4;
            g.setFont(juce::Font(juce::FontOptions(9.0f)));
            g.drawText(note, juce::Rectangle<int>(noteX, 0, 246 - noteX, 22), juce::Justification::centredLeft, true);
        }
    };

    RoutineEngine::Status::Slot idle(int slot, const char* name, double lengthBeats, bool loop = false)
    {
        RoutineEngine::Status::Slot s;
        s.slot = slot;
        s.uuid = std::string("u") + std::to_string(slot);
        s.name = name;
        s.state = "idle";
        s.lengthBeats = lengthBeats;
        s.loop = loop;
        s.quantize = "bar";
        return s;
    }

    void run(RoutineEngine::Status::Slot& s, const char* state, int deck, std::vector<int> layers, uint32_t seq, double pos)
    {
        s.state = state;
        s.deck = deck;
        s.layers = std::move(layers);
        s.fireSeq = seq;
        s.position = pos;
        s.startsOn = std::string(state) == "pending" ? "bar" : "";
    }

    RoutineEngine::Status bank()
    {
        RoutineEngine::Status st;
        st.slots[0] = idle(0, "Drop", 32.0);
        st.slots[1] = idle(1, "Build", 16.0, true);
        st.slots[2] = idle(2, "Wash", 4.0);
        return st;
    }

    bool save(juce::Component& c, const juce::File& file)
    {
        const auto img = c.createComponentSnapshot(c.getLocalBounds(), true, 2.0f);
        file.deleteFile();
        juce::FileOutputStream out(file);
        juce::PNGImageFormat png;
        return out.openedOk() && png.writeImageToStream(img, out);
    }

    void shoot(const juce::File& dir, const juce::String& name, const RoutineEngine::Status& st, int shownDeck,
               int stripLayer)
    {
        const std::vector<juce::String> decks{ "Deck 1", "Deck 2" };
        const std::vector<juce::String> layers{ "Layer 1", "Layer 2", "Layer 3" };
        const auto v = deriveRoutineDeckView(st, shownDeck, decks, layers);

        Row row;
        row.setSize(970, 22);
        for (int i = 0; i < RoutineEngine::kBankSize; ++i)
            row.pads[static_cast<size_t>(i)]->setSpec(v.pads[i]);
        row.note = v.cornerNote;
        const bool a = save(row, dir.getChildFile(name + "-row.png"));

        Layer layer;
        layer.name = "Layer " + std::to_string(stripLayer + 1);
        LayerStrip strip;
        strip.setLayer(&layer, stripLayer);
        strip.setSize(250, 96);
        const auto it = v.bandsByLayer.find(stripLayer);
        strip.setRoutineBands(it == v.bandsByLayer.end() ? std::vector<RoutineDeckView::Band>{} : bandsToDraw(it->second));
        const bool b = save(strip, dir.getChildFile(name + "-strip.png"));
        std::cout << name << ": row " << (a ? "ok" : "FAILED") << ", strip " << (b ? "ok" : "FAILED") << "\n";
    }
}

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        std::cerr << "usage: tool_routine_deck_snapshot <output-directory>\n";
        return 2;
    }
    juce::ScopedJuceInitialiser_GUI gui;
    AudioDNALookAndFeel laf;
    juce::LookAndFeel::setDefaultLookAndFeel(&laf);
    const juce::File dir { juce::String(argv[1]) };
    dir.createDirectory();

    shoot(dir, "t1-empty", RoutineEngine::Status{}, 0, 0);
    shoot(dir, "t2-idle", bank(), 0, 0);
    {
        auto st = bank();
        run(st.slots[0], "pending", 0, { 0, 2 }, 1, 0.0);
        shoot(dir, "t3-waiting", st, 0, 0);
    }
    {
        auto st = bank();
        run(st.slots[0], "running", 0, { 0, 2 }, 1, 17.0);
        shoot(dir, "t4-playing-5of8", st, 0, 2);
    }
    {
        auto st = bank();
        run(st.slots[0], "running", 0, { 0, 2 }, 1, 21.0);
        run(st.slots[1], "running", 0, { 0 }, 2, 5.0);
        shoot(dir, "t5-two-on-one-layer", st, 0, 0);
    }
    {
        auto st = bank();
        run(st.slots[0], "running", 0, { 0, 2 }, 1, 17.0);
        run(st.slots[1], "running", 0, { 0 }, 2, 5.0);
        shoot(dir, "t6-offdeck", st, 1, 0);
    }
    {
        auto st = bank();
        st.slots[2].preambleUnresolved = 1;
        shoot(dir, "t7-warning", st, 0, 0);
    }
    {
        auto st = bank();   // s-rta-0927 fix round: pad 1 pressed again while playing -- the restart mark
        run(st.slots[0], "running", 0, { 0, 2 }, 1, 17.0);
        st.slots[0].restartPending = true;
        st.slots[0].startsOn = "bar";
        shoot(dir, "t8-restart-pending", st, 0, 2);
    }
    juce::LookAndFeel::setDefaultLookAndFeel(nullptr);
    return 0;
}
