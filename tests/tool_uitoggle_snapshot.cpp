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
#include "output/OutputMenuModel.h"
#include "ui/MenuBarModel.h"
#include <algorithm>
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

    // s-rta-0927 outputs-c2: a PopupMenu drawn row by row with the app LookAndFeel's own popup painters
    // (drawPopupMenuBackground / drawPopupMenuItem -- what JUCE's PopupMenu window paints for the TopBar
    // "Outputs" button). The menu-bar door is the native macOS NSMenu: same titles, ids and ticks, drawn by macOS.
    bool writeMenu(const juce::PopupMenu& menu, juce::LookAndFeel& laf, const juce::File& out)
    {
        std::vector<juce::PopupMenu::Item> items;
        std::vector<int> heights;
        int width = 0;
        for (juce::PopupMenu::MenuItemIterator it(menu); it.next();)
        {
            const auto& i = it.getItem();
            int w = 0, h = 0;
            laf.getIdealPopupMenuItemSize(i.text, i.isSeparator, -1, w, h);
            width = std::max(width, w + (i.shortcutKeyDescription.isNotEmpty() ? 110 : 0));
            items.push_back(i);
            heights.push_back(h);
        }
        int total = 8;
        for (int h : heights)
            total += h;
        width += 24;
        juce::Image image(juce::Image::ARGB, width, total, true);
        {
            juce::Graphics g(image);
            laf.drawPopupMenuBackground(g, width, total);
            int y = 4;
            for (size_t k = 0; k < items.size(); ++k)
            {
                const auto& i = items[k];
                laf.drawPopupMenuItem(g, { 0, y, width, heights[k] }, i.isSeparator, i.isEnabled, false, i.isTicked,
                                      false, i.text, i.shortcutKeyDescription, nullptr, nullptr);
                y += heights[k];
            }
        }
        juce::PNGImageFormat png;
        out.deleteFile();
        juce::FileOutputStream stream(out);
        return stream.openedOk() && png.writeImageToStream(image, stream);
    }

    void printMenu(const juce::String& title, const juce::PopupMenu& menu)
    {
        std::cout << title << "\n";
        for (juce::PopupMenu::MenuItemIterator it(menu); it.next();)
        {
            const auto& i = it.getItem();
            if (i.isSeparator)
                std::cout << "  ----\n";
            else
                std::cout << "  [" << (i.isTicked ? "x" : " ") << "] " << i.text << "  (id " << i.itemID
                          << (i.isEnabled ? "" : ", disabled")
                          << (i.shortcutKeyDescription.isNotEmpty() ? ", " + i.shortcutKeyDescription : juce::String())
                          << ")\n";
        }
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

    // s-rta-0927 outputs-c2: the TopBar "Outputs" button with 0 / 1 / 2 live outputs, set through
    // TopBar::setLiveOutputCount -- the call OutputManager::onLiveCountChanged makes. No window is opened.
    for (int n : { 0, 1, 2 })
    {
        Composition comp;
        comp.initDefault();
        FeatureBus bus;
        TopBar bar(bus, comp);
        bar.setSize(1728, 40);
        bar.setLiveOutputCount(n);
        const auto name = "topbar-outputs-" + juce::String(n) + "-headless.png";
        if (!writeSnapshot(bar, outDir.getChildFile(name)))
        {
            std::cerr << "failed to write " << name << "\n";
            ok = false;
        }
    }

    // The output item list the model builds for a FAKE 3-display set (main flagged, Display 2 live), as the TopBar
    // button's PopupMenu, and the whole Output menu of the menu bar built from the same list. s-rta-0927 outputs-c3:
    // the list ends with "Restore Last Outputs" -- rendered DISABLED (nothing restorable: the first launch, an empty
    // or absent saved set) and ENABLED (a saved output whose display is connected and free).
    {
        using C = AudioDNAMenuBar::CommandID;
        const std::vector<output::DisplayInfo> fake { { 0, 0, 1728, 1117, 2.0, true },
                                                      { 1728, 0, 1920, 1080, 1.0, false },
                                                      { -3840, 0, 3840, 2160, 1.0, false } };
        for (const bool canRestore : { false, true })
        {
            const juce::String state = canRestore ? "restore-on" : "restore-off";
            const auto items = output::buildOutputMenu(fake, { false, true, false }, 1, C::kOutputFullscreenBase,
                                                       C::kOutputDisabled, C::kOutputRestoreLast, canRestore);
            juce::PopupMenu topBarMenu;
            output::addOutputMenuItems(topBarMenu, items, C::kOutputDisabled);
            printMenu("TopBar Outputs button menu (fake 3-display set, " + state + "):", topBarMenu);
            const auto topName = "outputs-menu-fake3-" + state + "-headless.png";
            if (!writeMenu(topBarMenu, laf, outDir.getChildFile(topName)))
            {
                std::cerr << "failed to write " << topName << "\n";
                ok = false;
            }
            AudioDNAMenuBar bar;
            bar.populateOutputItems = [&items](juce::PopupMenu& m) { output::addOutputMenuItems(m, items, C::kOutputDisabled); };
            const auto outputMenu = bar.getMenuForIndex(6, "Output");
            printMenu("Menu bar > Output (fake 3-display set, " + state + "):", outputMenu);
            const auto barName = "output-menubar-fake3-" + state + "-headless.png";
            if (!writeMenu(outputMenu, laf, outDir.getChildFile(barName)))
            {
                std::cerr << "failed to write " << barName << "\n";
                ok = false;
            }
        }

        // This machine's REAL displays (juce::Desktop -- a read, no window), nothing live: the titles the app's
        // Output menu shows on this rig.
        std::vector<output::DisplayInfo> real;
        for (const auto& d : juce::Desktop::getInstance().getDisplays().displays)
            real.push_back({ d.totalArea.getX(), d.totalArea.getY(), d.totalArea.getWidth(), d.totalArea.getHeight(),
                             d.scale, d.isMain });
        const auto realItems = output::buildOutputMenu(real, {}, 0, C::kOutputFullscreenBase, C::kOutputDisabled,
                                                       C::kOutputRestoreLast, false);
        AudioDNAMenuBar realBar;
        realBar.populateOutputItems = [&realItems](juce::PopupMenu& m) {
            output::addOutputMenuItems(m, realItems, C::kOutputDisabled);
        };
        printMenu("Menu bar > Output (this machine's displays, nothing live):", realBar.getMenuForIndex(6, "Output"));
    }

    juce::LookAndFeel::setDefaultLookAndFeel(nullptr);

    if (ok)
        std::cout << "wrote 12 PNGs to " << outDir.getFullPathName() << "\n";
    return ok ? 0 : 1;
}
