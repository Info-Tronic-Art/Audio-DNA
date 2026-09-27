// tool_uitoggle_snapshot -- s-rta-0926b uitoggle lane, HEADLESS supplementary evidence.
// A ctest-EXCLUDED tool (deliberately never registered with catch_discover_tests), same
// idiom as tool_routine_strip_snapshot.cpp: renders real production widgets, with the app's
// own AudioDNALookAndFeel, into PNGs via juce::Component::createComponentSnapshot -- no
// window, no peer, no live app, no port 7070, no lock needed.
//
// This does NOT replace the rig-mandated live-app window screenshots (Quartz window id,
// screencapture -l<id> -o -x) -- it is supplementary pixel evidence of the same LookAndFeel
// dimming fix, captured while the live-app lock may be held by another concurrent lane.
//
// Usage: tool_uitoggle_snapshot [output-directory]  (defaults to ./uitoggle-shots)
#include <juce_gui_basics/juce_gui_basics.h>
#include "ui/LookAndFeel.h"
#include "ui/TopBar.h"
#include "ui/LayerInspector.h"
#include "ui/RecordPanel.h"
#include "features/FeatureBus.h"
#include "model/Composition.h"
#include <iostream>

namespace
{
    bool writeSnapshot(juce::Component& comp, const juce::File& out)
    {
        const auto image = comp.createComponentSnapshot(comp.getLocalBounds());
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
        : juce::File::getCurrentWorkingDirectory().getChildFile("uitoggle-shots");
    outDir.createDirectory();

    bool ok = true;

    // TopBar: default build has no Link, so this is always the disabled/dimmed state
    // (matches the real app's default-build Link toggle).
    {
        Composition comp;
        comp.initDefault();
        FeatureBus bus;
        TopBar bar(bus, comp);
        bar.setSize(1728, 40);
        if (!writeSnapshot(bar, outDir.getChildFile("topbar-link-headless.png")))
        {
            std::cerr << "failed to write topbar-link-headless.png\n";
            ok = false;
        }
    }

    // LayerInspector: Opaque layer, flag off -- Persistent enabled (unchanged look).
    {
        LayerInspector inspector;
        inspector.setSize(300, 900);
        Layer layer;
        layer.type = Layer::Type::Opaque;
        layer.persistent = false;
        inspector.setLayer(&layer);
        if (!writeSnapshot(inspector, outDir.getChildFile("layerinspector-opaque-headless.png")))
        {
            std::cerr << "failed to write layerinspector-opaque-headless.png\n";
            ok = false;
        }
    }

    // LayerInspector: Mask layer, flag cleared -- Persistent disabled + dimmed.
    {
        LayerInspector inspector;
        inspector.setSize(300, 900);
        Layer layer;
        layer.type = Layer::Type::Mask;
        layer.persistent = false;
        inspector.setLayer(&layer);
        if (!writeSnapshot(inspector, outDir.getChildFile("layerinspector-mask-cleared-headless.png")))
        {
            std::cerr << "failed to write layerinspector-mask-cleared-headless.png\n";
            ok = false;
        }
    }

    // LayerInspector: Mask layer, stale flag still set -- Persistent enabled so it CAN be
    // cleared (the actual bug fixed this lane; not one of the two shots the packet named,
    // but the clearest single frame of the fix).
    {
        LayerInspector inspector;
        inspector.setSize(300, 900);
        Layer layer;
        layer.type = Layer::Type::Mask;
        layer.persistent = true;
        inspector.setLayer(&layer);
        if (!writeSnapshot(inspector, outDir.getChildFile("layerinspector-mask-stale-headless.png")))
        {
            std::cerr << "failed to write layerinspector-mask-stale-headless.png\n";
            ok = false;
        }
    }

    // RecordPanel: default status -- Record audio enabled, Play with audio disabled+dimmed
    // (no take loaded yet), both toggles visible in one frame.
    {
        RecordPanel panel;
        panel.setSize(380, 700);
        panel.refresh(RecorderHost::Status{}, 0.0);
        if (!writeSnapshot(panel, outDir.getChildFile("recordpanel-default-headless.png")))
        {
            std::cerr << "failed to write recordpanel-default-headless.png\n";
            ok = false;
        }
    }

    juce::LookAndFeel::setDefaultLookAndFeel(nullptr);

    if (ok)
        std::cout << "wrote 5 PNGs to " << outDir.getFullPathName() << "\n";
    return ok ? 0 : 1;
}
