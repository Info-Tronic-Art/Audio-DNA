#include "MenuBarModel.h"

AudioDNAMenuBar::AudioDNAMenuBar()
{
}

juce::StringArray AudioDNAMenuBar::getMenuBarNames()
{
    return { "Audio-DNA", "Composition", "Deck", "Layer",
             "Column", "Clip", "Output", "Shortcuts", "View" };
}

juce::PopupMenu AudioDNAMenuBar::getMenuForIndex(int menuIndex,
                                                   const juce::String& /*menuName*/)
{
    juce::PopupMenu menu;

    switch (menuIndex)
    {
        case 0: // Audio-DNA
        {
            menu.addItem(kPreferences, "Preferences...",  true, false);
            menu.addSeparator();
            menu.addItem(kImportISF, "Import ISF Shader...", true, false);
            menu.addSeparator();
            menu.addItem(kAbout, "About Audio-DNA");
            menu.addSeparator();
            menu.addItem(kQuit, "Quit", true, false);
            break;
        }

        case 1: // Composition
        {
            menu.addItem(kCompUndo,     "Undo",                       true, false);
            menu.addItem(kCompRedo,     "Redo",                       true, false);
            menu.addSeparator();
            menu.addItem(kCompNew,      "New Composition",            true, false);
            menu.addItem(kCompOpen,     "Open...",                    true, false);
            menu.addSeparator();
            menu.addItem(kCompSave,     "Save",                      true, false);
            menu.addItem(kCompSaveAs,   "Save As...",                true, false);
            menu.addSeparator();
            menu.addItem(kCompCollectMedia,   "Collect Media...",        true, false);
            menu.addItem(kCompRelocateFiles,  "Relocate Missing Files...", true, false);
            break;
        }

        case 2: // Deck
        {
            menu.addItem(kDeckNew,          "New Deck",              true, false);
            menu.addSeparator();
            menu.addItem(kDeckClearClips,   "Clear Clips",           true, false);
            menu.addItem(kDeckRemove,       "Remove Deck",           true, false);
            break;
        }

        case 3: // Layer
        {
            menu.addItem(kLayerNew,            "New Layer",                  true, false);
            menu.addItem(kLayerInsertAbove,    "Insert Above",              true, false);
            menu.addItem(kLayerInsertBelow,    "Insert Below",              true, false);
            menu.addSeparator();
            menu.addItem(kLayerClearClips,     "Clear Clips",               true, false);
            menu.addItem(kLayerRemove,         "Remove Layer",              true, false);
            menu.addSeparator();
            menu.addItem(kLayerFold,           "Fold/Unfold Layer",         true, false);
            menu.addItem(kLayerMoveUp,         "Move Layer Up",             true, false);
            menu.addItem(kLayerMoveDown,       "Move Layer Down",           true, false);
            break;
        }

        case 4: // Column
        {
            menu.addItem(kColumnNew,            "New Column",           true, false);
            menu.addItem(kColumnInsertBefore,   "Insert Before",        true, false);
            menu.addItem(kColumnInsertAfter,    "Insert After",         true, false);
            menu.addSeparator();
            menu.addItem(kColumnRemove,         "Remove Column",        true, false);
            break;
        }

        case 5: // Clip
        {
            menu.addItem(kClipClear,        "Clear",                   true, false);
            menu.addSeparator();
            menu.addItem(kClipReplaceContent, "Replace Content...",    true, false);
            menu.addItem(kClipLockContent,    "Lock Content",          true, false);
            break;
        }

        case 6: // Output
        {
            menu.addItem(kOutputDisabled, "Disabled", true, false);
            menu.addSeparator();

            // Add available displays
            const auto& displays = juce::Desktop::getInstance().getDisplays().displays;
            for (int i = 0; i < static_cast<int>(displays.size()); ++i)
            {
                const auto& d = displays[static_cast<size_t>(i)];
                juce::String label = "Fullscreen: "
                    + juce::String(d.totalArea.getWidth()) + "x"
                    + juce::String(d.totalArea.getHeight());
                if (d.isMain)
                    label += " (main)";
                else
                    label += " (display " + juce::String(i + 1) + ")";
                menu.addItem(kOutputFullscreenBase + i, label, true, false);
            }

            menu.addSeparator();
            menu.addItem(kOutputSnapshot,         "Snapshot",            true, false);
            menu.addSeparator();
            menu.addItem(kOutputStartRecording,   "Start Recording",     true, false);
            menu.addItem(kOutputStopRecording,    "Stop Recording",      true, false);
            break;
        }

        case 7: // Shortcuts
        {
            menu.addItem(kShortcutsEditKeyboard, "Edit Keyboard Shortcuts...", true, false);
            menu.addItem(kShortcutsEditMIDI,     "Edit MIDI Mappings...",      true, false);
            menu.addSeparator();
            menu.addItem(kShortcutsStop,         "Stop All",                   true, false);
            menu.addSeparator();
            menu.addItem(kShortcutsExportBindings, "Export Bindings...",        true, false);
            menu.addItem(kShortcutsImportBindings, "Import Bindings...",        true, false);
            break;
        }

        case 8: // View
        {
            menu.addItem(kViewSaveLayout,      "Save Layout...",    true, false);
            menu.addItem(kViewLoadLayout,      "Load Layout...",    true, false);
            menu.addItem(kViewResetLayout,     "Reset Layout",      true, false);
            break;
        }

        default:
            break;
    }

    return menu;
}

void AudioDNAMenuBar::menuItemSelected(int menuItemID, int /*topLevelMenuIndex*/)
{
    if (onMenuCommand)
        onMenuCommand(menuItemID);
}
