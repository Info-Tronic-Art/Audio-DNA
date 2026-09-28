// test_output_menu_model -- s-rta-0927 outputs-c2 = plan5 slice C2 (.harmony/.reports/s-rta-0926b/plan5-final.md
// sections 5, 6, 7.3, 10.1): the ONE item list behind both doors to the output displays (the Output menu and the
// TopBar "Outputs" button), the button text, the output keys, and the menu bar's Output menu built from the list.
// outputs-c3 (plan5 C3) adds "Restore Last Outputs" and the Shift-up rule for Cmd+F / Cmd+` (Harmony ruling (b)).
// Pure/headless: no window, no GL, no Desktop -- the code under test is src/output/OutputMenuModel.h (what
// OutputManager::populateMenu and MainComponent::keyPressed run) and src/ui/MenuBarModel.cpp (case 6).
#include <catch2/catch_test_macros.hpp>
#include <juce_gui_basics/juce_gui_basics.h>
#include "output/OutputMenuModel.h"
#include "ui/MenuBarModel.h"

using output::DisplayInfo;
using output::OutputKey;
using C = AudioDNAMenuBar::CommandID;

namespace
{
const DisplayInfo kLaptop { 0, 0, 1728, 1117, 2.0, true };      // main first, as [NSScreen screens] orders it
const DisplayInfo kProjector { 1728, 0, 1920, 1080, 1.0, false };
const DisplayInfo kTv { -1920, 0, 1920, 1080, 1.0, false };     // same size as the projector, other position

struct Flat { juce::String text; int id; bool ticked, enabled, separator; juce::String shortcut; };

std::vector<Flat> flatten(const juce::PopupMenu& menu)
{
    std::vector<Flat> out;
    for (juce::PopupMenu::MenuItemIterator it(menu); it.next();)
    {
        const auto& i = it.getItem();
        out.push_back({ i.text, i.itemID, i.isTicked, i.isEnabled, i.isSeparator, i.shortcutKeyDescription });
    }
    return out;
}
} // namespace

TEST_CASE("buildOutputMenu: one tickable item per display, then All Outputs Off", "[output_menu_model]")
{
    SECTION("two displays (main first), the second live")
    {
        const auto items = output::buildOutputMenu({ kLaptop, kProjector }, { false, true }, 1,
                                                   C::kOutputFullscreenBase, C::kOutputDisabled);
        REQUIRE(items.size() == 3);
        CHECK(items[0].label == "Display 1 (1728x1117, main)");
        CHECK(items[0].id == C::kOutputFullscreenBase + 0);
        CHECK_FALSE(items[0].ticked);
        CHECK(items[0].enabled);
        CHECK(items[1].label == "Display 2 (1920x1080)");
        CHECK(items[1].id == C::kOutputFullscreenBase + 1);
        CHECK(items[1].ticked);
        CHECK(items[2].label == "All Outputs Off");
        CHECK(items[2].id == C::kOutputDisabled);
        CHECK(items[2].enabled);
        CHECK_FALSE(items[2].ticked);
        CHECK(items[2].shortcut == "Cmd+Shift+Esc");
    }

    SECTION("nothing live -> All Outputs Off is disabled")
    {
        const auto items = output::buildOutputMenu({ kLaptop, kProjector }, {}, 0,
                                                   C::kOutputFullscreenBase, C::kOutputDisabled);
        REQUIRE(items.size() == 3);
        CHECK_FALSE(items[0].ticked);
        CHECK_FALSE(items[1].ticked);
        CHECK_FALSE(items[2].enabled);
    }

    SECTION("zero displays -> only the (disabled) off item")
    {
        const auto items = output::buildOutputMenu({}, {}, 0, C::kOutputFullscreenBase, C::kOutputDisabled);
        REQUIRE(items.size() == 1);
        CHECK(items[0].label == "All Outputs Off");
        CHECK_FALSE(items[0].enabled);
    }

    SECTION("three displays, two live -- the main screen included")
    {
        const auto items = output::buildOutputMenu({ kLaptop, kProjector, kTv }, { true, false, true }, 2,
                                                   C::kOutputFullscreenBase, C::kOutputDisabled);
        REQUIRE(items.size() == 4);
        CHECK(items[0].ticked);
        CHECK_FALSE(items[1].ticked);
        CHECK(items[2].ticked);
        CHECK(items[2].label == "Display 3 (1920x1080)");
        CHECK(items[3].enabled);
    }

    SECTION("a live window whose display went away ticks nothing but keeps All Outputs Off enabled")
    {
        const auto items = output::buildOutputMenu({ kLaptop }, { false }, 1,
                                                   C::kOutputFullscreenBase, C::kOutputDisabled);
        REQUIRE(items.size() == 2);
        CHECK_FALSE(items[0].ticked);
        CHECK(items[1].enabled);
    }

    SECTION("labels are ASCII (AX scripts click menu items by title)")
    {
        for (const auto& it : output::buildOutputMenu({ kLaptop, kProjector }, {}, 0, C::kOutputFullscreenBase,
                                                      C::kOutputDisabled))
            for (auto p = it.label.getCharPointer(); !p.isEmpty(); ++p)
                CHECK(static_cast<int>(*p) < 128);
    }
}

TEST_CASE("DisplayInfo identifies a display exactly", "[output_menu_model]")
{
    CHECK(kProjector == DisplayInfo { 1728, 0, 1920, 1080, 1.0, false });
    CHECK(kProjector != kTv);                                            // same size, other position
    CHECK(kProjector != DisplayInfo { 1728, 0, 1920, 1080, 2.0, false });   // scale differs
}

TEST_CASE("the TopBar button text", "[output_menu_model]")
{
    CHECK(output::outputsButtonText(0) == "Outputs: Off");
    CHECK(output::outputsButtonText(1) == "Outputs: 1");
    CHECK(output::outputsButtonText(2) == "Outputs: 2");
    CHECK(output::outputsButtonText(3) == "Outputs: 3");
}

TEST_CASE("addOutputMenuItems: the PopupMenu both doors show", "[output_menu_model]")
{
    juce::ScopedJuceInitialiser_GUI gui;   // local, never a function-static (notebook: s-rta-0925 rclick2)
    juce::PopupMenu menu;
    output::addOutputMenuItems(menu, output::buildOutputMenu({ kLaptop, kProjector }, { false, true }, 1,
                                                             C::kOutputFullscreenBase, C::kOutputDisabled),
                               C::kOutputDisabled);
    const auto f = flatten(menu);
    REQUIRE(f.size() == 4);   // two displays, a separator, All Outputs Off
    CHECK(f[0].text == "Display 1 (1728x1117, main)");
    CHECK_FALSE(f[0].ticked);
    CHECK(f[1].text == "Display 2 (1920x1080)");
    CHECK(f[1].ticked);
    CHECK(f[2].separator);
    CHECK(f[3].text == "All Outputs Off");
    CHECK(f[3].id == C::kOutputDisabled);
    CHECK(f[3].enabled);
    CHECK(f[3].shortcut == "Cmd+Shift+Esc");
}

TEST_CASE("the menu bar's Output menu is built from the item list (All Outputs Off, no Fullscreen items)",
          "[output_menu_model]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    AudioDNAMenuBar bar;
    bar.populateOutputItems = [](juce::PopupMenu& m) {
        output::addOutputMenuItems(m, output::buildOutputMenu({ kLaptop, kProjector, kTv }, { false, true, false }, 1,
                                                              C::kOutputFullscreenBase, C::kOutputDisabled),
                                   C::kOutputDisabled);
    };
    const auto names = bar.getMenuBarNames();
    const int outputIndex = names.indexOf("Output");
    REQUIRE(outputIndex == 6);
    const auto f = flatten(bar.getMenuForIndex(outputIndex, "Output"));

    std::vector<Flat> items;   // separators dropped
    for (const auto& x : f)
        if (!x.separator)
            items.push_back(x);
    REQUIRE(items.size() == 8);
    CHECK(items[0].text == "Display 1 (1728x1117, main)");
    CHECK(items[0].id == C::kOutputFullscreenBase);
    CHECK(items[1].text == "Display 2 (1920x1080)");
    CHECK(items[1].ticked);
    CHECK(items[2].text == "Display 3 (1920x1080)");
    CHECK(items[3].text == "All Outputs Off");
    CHECK(items[3].id == C::kOutputDisabled);
    // Snapshot / Recording / Syphon unchanged, ids unchanged.
    CHECK(items[4].text == "Snapshot");
    CHECK(items[4].id == C::kOutputSnapshot);
    CHECK(items[5].text == "Start Recording");
    CHECK(items[6].text == "Stop Recording");
    CHECK(items[7].text == "Syphon Output");
    CHECK(items[7].id == C::kOutputSyphon);
    // The exact list above already rules out the retired close item (ids unchanged, the label now "All Outputs Off");
    // no retired per-display "Fullscreen: WxH" item either.
    for (const auto& x : items)
        CHECK_FALSE(x.text.startsWith("Fullscreen: "));
    // The enum is untouched in C2 (plan5 section 11): no existing id moved.
    CHECK(static_cast<int>(C::kOutputDisabled) == 1600);
    CHECK(static_cast<int>(C::kOutputFullscreenBase) == 1601);
    CHECK(static_cast<int>(C::kOutputWindowed) == 1690);
    CHECK(static_cast<int>(C::kOutputSyphon) == 1696);
}

TEST_CASE("Restore Last Outputs (plan5 C3): right below All Outputs Off, enabled iff restoring would open an output",
          "[output_menu_model]")
{
    // Its id is APPENDED after the last existing Output id: no existing id moves.
    CHECK(static_cast<int>(C::kOutputRestoreLast) == static_cast<int>(C::kOutputSyphon) + 1);
    CHECK(static_cast<int>(C::kOutputRestoreLast) == 1697);
    CHECK(static_cast<int>(C::kOutputRestoreLast) < static_cast<int>(C::kShortcutsEditKeyboard));

    SECTION("something to restore -> enabled, never ticked, no shortcut")
    {
        const auto items = output::buildOutputMenu({ kLaptop, kProjector }, {}, 0, C::kOutputFullscreenBase,
                                                   C::kOutputDisabled, C::kOutputRestoreLast, true);
        REQUIRE(items.size() == 4);
        CHECK(items[2].label == "All Outputs Off");
        CHECK(items[3].label == "Restore Last Outputs");
        CHECK(items[3].id == C::kOutputRestoreLast);
        CHECK(items[3].enabled);
        CHECK_FALSE(items[3].ticked);
        CHECK(items[3].shortcut.isEmpty());
    }

    SECTION("nothing to restore (an empty or absent saved set, or none of its displays free) -> disabled")
    {
        const auto items = output::buildOutputMenu({ kLaptop, kProjector }, { false, true }, 1, C::kOutputFullscreenBase,
                                                   C::kOutputDisabled, C::kOutputRestoreLast, false);
        REQUIRE(items.size() == 4);
        CHECK(items[3].label == "Restore Last Outputs");
        CHECK_FALSE(items[3].enabled);
        CHECK(items[2].enabled);   // All Outputs Off keeps its own rule
    }

    SECTION("both doors: the PopupMenu has it right below All Outputs Off, and so does the menu bar's Output menu")
    {
        juce::ScopedJuceInitialiser_GUI gui;
        const auto items = output::buildOutputMenu({ kLaptop, kProjector, kTv }, { false, true, false }, 1,
                                                   C::kOutputFullscreenBase, C::kOutputDisabled,
                                                   C::kOutputRestoreLast, false);
        juce::PopupMenu menu;
        output::addOutputMenuItems(menu, items, C::kOutputDisabled);
        const auto f = flatten(menu);
        REQUIRE(f.size() == 6);   // three displays, a separator, All Outputs Off, Restore Last Outputs
        CHECK(f[3].separator);
        CHECK(f[4].text == "All Outputs Off");
        CHECK(f[5].text == "Restore Last Outputs");
        CHECK(f[5].id == C::kOutputRestoreLast);
        CHECK_FALSE(f[5].enabled);

        AudioDNAMenuBar bar;
        bar.populateOutputItems = [&items](juce::PopupMenu& m) { output::addOutputMenuItems(m, items, C::kOutputDisabled); };
        std::vector<Flat> bi;
        for (const auto& x : flatten(bar.getMenuForIndex(6, "Output")))
            if (!x.separator)
                bi.push_back(x);
        REQUIRE(bi.size() == 9);
        CHECK(bi[3].text == "All Outputs Off");
        CHECK(bi[4].text == "Restore Last Outputs");
        CHECK(bi[5].text == "Snapshot");
        CHECK(bi[8].text == "Syphon Output");
    }
}

TEST_CASE("the output keys (plan5 7.2-7.3), from KeyPress descriptions -- no key is ever pressed",
          "[output_menu_model]")
{
    juce::ScopedJuceInitialiser_GUI gui;
    using juce::KeyPress;

    SECTION("Cmd+Shift+Esc is the panic chord and wins over the bare-Escape case")
    {
        const auto k = KeyPress::createFromDescription("command + shift + escape");
        CHECK(k.isKeyCode(KeyPress::escapeKey));   // the modifier-blind test a bare-Escape branch makes
        CHECK(output::classifyOutputKey(k) == OutputKey::CloseAll);
    }

    SECTION("plain Esc (and Esc with only one of the two modifiers) never touches outputs, but is swallowed")
    {
        CHECK(output::classifyOutputKey(KeyPress(KeyPress::escapeKey)) == OutputKey::SwallowEscape);
        CHECK(output::classifyOutputKey(KeyPress::createFromDescription("command + escape")) == OutputKey::SwallowEscape);
        CHECK(output::classifyOutputKey(KeyPress::createFromDescription("shift + escape")) == OutputKey::SwallowEscape);
    }

    SECTION("Cmd+` raises the app: the backtick key code is 0x60, as the mac peer derives it")
    {
        const auto k = KeyPress::createFromDescription(juce::String("command + ") + juce::String::charToString('`'));
        CHECK(k.getKeyCode() == 0x60);
        CHECK(k.getModifiers().isCommandDown());
        // juce_NSViewComponentPeer_mac.mm getKeyCodeFromEvent: keyCode = toUpperCase(charactersIgnoringModifiers[0]);
        // the grave key's character is '`', which has no upper case -> 0x60 (its kVK_ANSI_Grave fallback is '`' too).
        CHECK(static_cast<int>(juce::CharacterFunctions::toUpperCase(static_cast<juce::juce_wchar>('`'))) == 0x60);
        CHECK(output::classifyOutputKey(k) == OutputKey::RaiseApp);
        CHECK(output::classifyOutputKey(KeyPress('`')) == OutputKey::None);   // bare backtick is not ours
    }

    SECTION("Cmd+F toggles the output on the main display (Boris: leave Cmd+F as-is)")
    {
        CHECK(output::classifyOutputKey(KeyPress::createFromDescription("command + F")) == OutputKey::ToggleMain);
        CHECK(output::classifyOutputKey(KeyPress::createFromDescription("command + f")) == OutputKey::ToggleMain);
        CHECK(output::classifyOutputKey(KeyPress('F')) == OutputKey::None);
    }

    SECTION("Cmd+Shift+F and Cmd+Shift+` are NOT output keys: ToggleMain and RaiseApp need Shift up "
            "(Harmony ruling (b) on the C2 review, s-rta-0927 outputs-c3)")
    {
        CHECK(output::classifyOutputKey(KeyPress::createFromDescription("command + shift + F")) == OutputKey::None);
        CHECK(output::classifyOutputKey(KeyPress::createFromDescription("command + shift + f")) == OutputKey::None);
        const auto k = KeyPress::createFromDescription(juce::String("command + shift + ") + juce::String::charToString('`'));
        CHECK(k.getKeyCode() == 0x60);
        CHECK(k.getModifiers().isShiftDown());
        CHECK(output::classifyOutputKey(k) == OutputKey::None);
    }

    SECTION("other chords are not output keys")
    {
        CHECK(output::classifyOutputKey(KeyPress::createFromDescription("command + Z")) == OutputKey::None);
        CHECK(output::classifyOutputKey(KeyPress::createFromDescription("command + shift + Z")) == OutputKey::None);
        CHECK(output::classifyOutputKey(KeyPress::createFromDescription("command + S")) == OutputKey::None);
    }
}
