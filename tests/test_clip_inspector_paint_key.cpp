// test_clip_inspector_paint_key -- s-rta-0928b idlepaint (plan-idlepaint.md 2.6 / 3.1 test 4). ClipInspector::refresh() runs
// at ~10 Hz and used to end in an unconditional repaint() of the whole inspector, which joined JUCE's mac peer union every
// time (Pitfall NN). Now it repaints only when something paint() shows changed: ClipInspector::paintKeyNow() carries every
// painted input. Headless JUCE widgets under ScopedJuceInitialiser_GUI; refresh() repaints are counted by
// uipaint::counters().clipInspectorRepaints.
#include <catch2/catch_test_macros.hpp>
#include <juce_gui_basics/juce_gui_basics.h>
#include "ui/ClipInspector.h"
#include "ui/UiPaintCounters.h"

namespace
{
uint64_t repaints() { return uipaint::counters().clipInspectorRepaints.load(); }

struct Rig
{
    Clip clip;
    ClipInspector insp;
    Rig()
    {
        clip.name = "seq";
        clip.mediaType = Clip::MediaType::ImageSequence;
        insp.setSize(413, 900);
        insp.setClip(&clip);
        insp.refresh();                                  // the first refresh after setClip repaints
    }
};
}

TEST_CASE("ClipInspector::paintKeyNow: equal across refreshes when nothing changed", "[idlepaint][inspector]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    Rig r;
    const auto k0 = r.insp.paintKeyNow();
    const auto base = repaints();
    r.insp.refresh();
    r.insp.refresh();
    CHECK(r.insp.paintKeyNow() == k0);
    CHECK(repaints() == base);                           // idle: silent
}

TEST_CASE("ClipInspector::paintKeyNow: every painted input changes the key and makes refresh() repaint",
          "[idlepaint][inspector]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    Rig r;
    auto expectRepaint = [&r](const char* what, auto&& change) {
        const auto k0 = r.insp.paintKeyNow();
        const auto base = repaints();
        change();
        INFO(what);
        CHECK_FALSE(r.insp.paintKeyNow() == k0);
        r.insp.refresh();
        CHECK(repaints() == base + 1);
        r.insp.refresh();
        CHECK(repaints() == base + 1);
    };
    expectRepaint("playhead", [&] { r.clip.playheadPosition = 0.25; });
    expectRepaint("in point", [&] { r.clip.inPoint = 0.1f; });
    expectRepaint("out point", [&] { r.clip.outPoint = 0.9f; });
    expectRepaint("transport mode", [&] { r.clip.transportMode = Clip::TransportMode::BPMSync; });
    expectRepaint("beat division", [&] { r.clip.beatDivision = 8.0f; });
    expectRepaint("name", [&] { r.clip.name = "renamed"; });
    expectRepaint("media type", [&] { r.clip.mediaType = Clip::MediaType::Video; });
    expectRepaint("size", [&] { r.insp.setSize(420, 900); });
    expectRepaint("fx drop highlight", [&] {
        juce::DragAndDropTarget::SourceDetails d(juce::var("fx:Ripple"), nullptr, {});
        static_cast<juce::DragAndDropTarget&>(r.insp).itemDragEnter(d);
    });
}

TEST_CASE("ClipInspector::paintKeyNow: setClip(nullptr) gives a null-clip key and one repaint", "[idlepaint][inspector]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    Rig r;
    const auto base = repaints();
    r.insp.setClip(nullptr);
    const auto k = r.insp.paintKeyNow();
    CHECK(k.clip == nullptr);
    CHECK(k.name.empty());
    CHECK_FALSE(k.playable);
    r.insp.refresh();
    CHECK(repaints() == base + 1);
    r.insp.refresh();
    CHECK(repaints() == base + 1);
}
