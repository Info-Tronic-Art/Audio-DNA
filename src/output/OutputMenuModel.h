#pragma once

// OutputMenuModel: the ONE item list behind both doors to the output displays -- the Output menu of the menu
// bar and the TopBar "Outputs" button (s-rta-0927 outputs-c2 = plan5 slice C2,
// .harmony/.reports/s-rta-0926b/plan5-final.md sections 5, 6, 7.3). Pure: data in, data out -- no window, no GL,
// no Desktop access -- unit-tested headless (tests/test_output_menu_model.cpp). OutputManager::populateMenu() and
// its /api/state view are built from buildOutputMenu(), so the two doors and the REST view cannot disagree.

#include <juce_gui_basics/juce_gui_basics.h>
#include "output/OutputTargets.h"   // DisplayInfo (one definition, plan5 C3)
#include <vector>

namespace output
{
struct OutputMenuItem
{
    juce::String label;
    int id = 0;
    bool ticked = false;
    bool enabled = true;
    juce::String shortcut;   // display-only (PopupMenu::Item::shortcutKeyDescription); the key lives in keyPressed
};

// "Display 2 (1920x1080)" / "Display 1 (1728x1117, main)". ASCII only: AX scripts click menu items by title.
inline juce::String displayLabel(int index, const DisplayInfo& d)
{
    return "Display " + juce::String(index + 1) + " (" + juce::String(d.w) + "x" + juce::String(d.h)
         + (d.isMain ? ", main)" : ")");
}

inline const char* allOutputsOffLabel() { return "All Outputs Off"; }
inline const char* allOutputsOffShortcut() { return "Cmd+Shift+Esc"; }

// One tickable item per connected display (tick = an output is live on it; id = fullscreenBase + index), then
// "All Outputs Off" (id = allOffId). `live` may be shorter than `displays` (missing entries read as not live).
// "All Outputs Off" is enabled iff any output window is live: a ticked display OR liveWindows > 0 -- the
// window count, because a window whose display went away (no reconcile before plan5 C3) ticks no item but
// must still be closable from the panic item.
inline std::vector<OutputMenuItem> buildOutputMenu(const std::vector<DisplayInfo>& displays, const std::vector<bool>& live,
                                                   int liveWindows, int fullscreenBase, int allOffId)
{
    std::vector<OutputMenuItem> items;
    bool anyLive = liveWindows > 0;
    for (size_t i = 0; i < displays.size(); ++i)
    {
        const bool on = i < live.size() && live[i];
        anyLive = anyLive || on;
        items.push_back({ displayLabel(static_cast<int>(i), displays[i]), fullscreenBase + static_cast<int>(i), on, true, {} });
    }
    items.push_back({ allOutputsOffLabel(), allOffId, false, anyLive, allOutputsOffShortcut() });
    return items;
}

// The items into a PopupMenu (both doors): the display items, a separator, then "All Outputs Off".
inline void addOutputMenuItems(juce::PopupMenu& menu, const std::vector<OutputMenuItem>& items, int allOffId)
{
    for (const auto& it : items)
    {
        if (it.id == allOffId)
            menu.addSeparator();
        juce::PopupMenu::Item item(it.label);
        item.itemID = it.id;
        item.isTicked = it.ticked;
        item.isEnabled = it.enabled;
        item.shortcutKeyDescription = it.shortcut;
        menu.addItem(std::move(item));
    }
}

// The TopBar button's text: "Outputs: Off" with none live, else "Outputs: N".
inline juce::String outputsButtonText(int liveCount)
{
    return liveCount <= 0 ? juce::String("Outputs: Off") : "Outputs: " + juce::String(liveCount);
}

// The keys of plan5 7.2-7.3, classified in ONE place (MainComponent::keyPressed acts on the result).
//   Cmd+Shift+Esc -> CloseAll (the panic chord). It MUST win over the bare-Escape case: KeyPress::isKeyCode
//                    compares the key code only, so a modifier-blind Escape test would swallow it.
//   Cmd+`         -> RaiseApp (bring the app window back above an output that covers it).
//   Cmd+F         -> ToggleMain (the output on the main display; Boris: "leave Cmd+F as-is").
//   Esc           -> SwallowEscape: plain Esc no longer touches outputs (plan5 Q2) but is still consumed.
// KeyPress key codes: letters upper-case; '`' = 0x60 (the mac peer upper-cases charactersIgnoringModifiers, and
// falls back to kVK_ANSI_Grave -> '`'; '`' has no case -- juce_NSViewComponentPeer_mac.mm getKeyCodeFromEvent).
enum class OutputKey { None, CloseAll, RaiseApp, ToggleMain, SwallowEscape };

inline OutputKey classifyOutputKey(const juce::KeyPress& key)
{
    const auto mod = key.getModifiers();
    if (key.isKeyCode(juce::KeyPress::escapeKey))
        return (mod.isCommandDown() && mod.isShiftDown()) ? OutputKey::CloseAll : OutputKey::SwallowEscape;
    if (key.isKeyCode('`') && mod.isCommandDown())
        return OutputKey::RaiseApp;
    if (key.isKeyCode('F') && mod.isCommandDown())
        return OutputKey::ToggleMain;
    return OutputKey::None;
}
} // namespace output
