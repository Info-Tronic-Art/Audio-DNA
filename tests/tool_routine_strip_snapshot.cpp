// tool_routine_strip_snapshot -- s-rta-0926 lane 3 SHOTS requirement (plan-routines-s1-final.md
// section 7 LANE 3). A ctest-EXCLUDED headless snapshot tool: deliberately never registered with
// catch_discover_tests, so `ctest` never runs it and it never adds file I/O to the normal suite.
//
// The live app cannot be screenshotted showing the Record tab's Routines strip without a
// synthetic click (forbidden by the rig's screen-safety rules -- no AX/System Events), so this is
// the sanctioned substitute: it renders RecordPanel, with the app's own LookAndFeel, at its real
// size into a PNG for each of the three pad states (empty bank / one saved routine, idle / one
// routine playing) via juce::Component::createComponentSnapshot -- no window, no peer, same
// headless posture as test_right_click_reset's ScopedJuceInitialiser_GUI idiom.
//
// Usage: tool_routine_strip_snapshot [output-directory]  (defaults to ./routine-strip-shots)
#include <juce_gui_basics/juce_gui_basics.h>
#include "ui/RecordPanel.h"
#include "ui/LookAndFeel.h"
#include <iostream>

namespace
{
    bool writeSnapshot(RecordPanel& panel, const juce::File& out)
    {
        panel.setSize(380, 700);
        const auto image = panel.createComponentSnapshot(panel.getLocalBounds());
        juce::PNGImageFormat png;
        out.deleteFile();
        juce::FileOutputStream stream(out);
        if (!stream.openedOk())
            return false;
        return png.writeImageToStream(image, stream);
    }
}

int main(int argc, char* argv[])
{
    juce::ScopedJuceInitialiser_GUI juceInit;
    AudioDNALookAndFeel laf;
    juce::LookAndFeel::setDefaultLookAndFeel(&laf);

    const juce::File outDir = argc > 1
        ? juce::File::getCurrentWorkingDirectory().getChildFile(juce::String(argv[1]))
        : juce::File::getCurrentWorkingDirectory().getChildFile("routine-strip-shots");
    outDir.createDirectory();

    bool ok = true;

    // State 1: empty bank -- every pad reads "N: Empty", disabled.
    {
        RecordPanel panel;
        RoutineEngine::Status s;   // every slot defaults to state "empty"
        panel.onRoutineStatus = [s] { return s; };
        panel.refresh(RecorderHost::Status{}, 0.0);
        if (!writeSnapshot(panel, outDir.getChildFile("1-empty.png")))
        {
            std::cerr << "failed to write 1-empty.png\n";
            ok = false;
        }
    }

    // State 2: one saved routine, idle -- pad 1 reads "1: Drop 1".
    {
        RecordPanel panel;
        RoutineEngine::Status s;
        s.slots[0].state = "idle";
        s.slots[0].name = "Drop 1";
        panel.onRoutineStatus = [s] { return s; };
        panel.refresh(RecorderHost::Status{}, 0.0);
        if (!writeSnapshot(panel, outDir.getChildFile("2-saved.png")))
        {
            std::cerr << "failed to write 2-saved.png\n";
            ok = false;
        }
    }

    // State 3: that routine playing -- pad 1 reads "1: Drop 1 (bar 2)", Playing tone.
    {
        RecordPanel panel;
        RoutineEngine::Status s;
        s.slots[0].state = "running";
        s.slots[0].name = "Drop 1";
        s.slots[0].position = 5.0;   // beat 5 of the cycle -> bar 2 (4 beats/bar, 1-based)
        panel.onRoutineStatus = [s] { return s; };
        panel.refresh(RecorderHost::Status{}, 0.0);
        if (!writeSnapshot(panel, outDir.getChildFile("3-playing.png")))
        {
            std::cerr << "failed to write 3-playing.png\n";
            ok = false;
        }
    }

    juce::LookAndFeel::setDefaultLookAndFeel(nullptr);

    if (ok)
        std::cout << "wrote 3 PNGs to " << outDir.getFullPathName() << "\n";
    return ok ? 0 : 1;
}
