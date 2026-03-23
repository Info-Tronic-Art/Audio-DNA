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
            menu.addItem(kAbout, "About Audio-DNA");
            menu.addSeparator();
            menu.addItem(kQuit, "Quit", true, false);
            break;
        }

        case 1: // Composition
        {
            menu.addItem(kCompNew,      "New Composition",            true, false);
            menu.addItem(kCompOpen,     "Open...",                    true, false);
            menu.addSeparator();
            menu.addItem(kCompSave,     "Save",                      true, false);
            menu.addItem(kCompSaveAs,   "Save As...",                true, false);
            menu.addSeparator();
            menu.addItem(kCompCopyEffects,  "Copy Global Effects",   true, false);
            menu.addItem(kCompPasteEffects, "Paste Global Effects",  true, false);
            break;
        }

        case 2: // Deck
        {
            menu.addItem(kDeckNew,          "New Deck",              true, false);
            menu.addItem(kDeckInsertBefore, "Insert Before",         true, false);
            menu.addItem(kDeckInsertAfter,  "Insert After",          true, false);
            menu.addItem(kDeckDuplicate,    "Duplicate",             true, false);
            menu.addSeparator();
            menu.addItem(kDeckRename,       "Rename...",             true, false);
            menu.addItem(kDeckClearClips,   "Clear Clips",           true, false);
            menu.addSeparator();
            menu.addItem(kDeckClose,        "Close Deck",            true, false);
            menu.addItem(kDeckRemove,       "Remove Deck",           true, false);
            break;
        }

        case 3: // Layer
        {
            menu.addItem(kLayerNew,            "New Layer",                  true, false);
            menu.addItem(kLayerInsertAbove,    "Insert Above",              true, false);
            menu.addItem(kLayerInsertBelow,    "Insert Below",              true, false);
            menu.addItem(kLayerDuplicate,      "Duplicate",                 true, false);
            menu.addSeparator();
            menu.addItem(kLayerRename,         "Rename...",                 true, false);
            menu.addItem(kLayerCopyEffects,    "Copy Effects",              true, false);
            menu.addItem(kLayerPasteEffects,   "Paste Effects",             true, false);
            menu.addSeparator();
            menu.addItem(kLayerClearClips,     "Clear Clips",               true, false);
            menu.addItem(kLayerRemove,         "Remove Layer",              true, false);
            menu.addSeparator();
            menu.addItem(kLayerIgnoreColumnTrigger, "Ignore Column Trigger", true, false);
            menu.addItem(kLayerLockContent,    "Lock Content",              true, false);
            break;
        }

        case 4: // Column
        {
            menu.addItem(kColumnNew,            "New Column",           true, false);
            menu.addItem(kColumnInsertBefore,   "Insert Before",        true, false);
            menu.addItem(kColumnInsertAfter,    "Insert After",         true, false);
            menu.addItem(kColumnDuplicate,      "Duplicate",            true, false);
            menu.addSeparator();
            menu.addItem(kColumnClearClips,     "Clear Clips",          true, false);
            menu.addItem(kColumnRemove,         "Remove Column",        true, false);
            menu.addSeparator();
            menu.addItem(kColumnRemoveAllBefore, "Remove All Before",   true, false);
            menu.addItem(kColumnRemoveAllAfter,  "Remove All After",    true, false);
            break;
        }

        case 5: // Clip
        {
            menu.addItem(kClipSelectAll,    "Select All",              true, false);
            menu.addSeparator();
            menu.addItem(kClipCut,          "Cut",                     true, false);
            menu.addItem(kClipCopy,         "Copy",                    true, false);
            menu.addItem(kClipPaste,        "Paste",                   true, false);
            menu.addSeparator();
            menu.addItem(kClipCopyEffects,  "Copy Effects",            true, false);
            menu.addItem(kClipPasteEffects, "Paste Effects",           true, false);
            menu.addSeparator();
            menu.addItem(kClipRename,       "Rename...",               true, false);
            menu.addItem(kClipClear,        "Clear",                   true, false);
            menu.addItem(kClipShowInFinder, "Show in Finder",          true, false);
            menu.addSeparator();
            menu.addItem(kClipNewSource,    "New Procedural Source",   true, false);
            menu.addItem(kClipNewEffect,    "New Effect Clip",         true, false);
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
            menu.addItem(kOutputWindowed,         "Windowed",            true, false);
            menu.addSeparator();
            menu.addItem(kOutputIdentifyDisplays, "Identify Displays",   true, false);
            menu.addItem(kOutputTestCard,         "Test Card",           true, false);
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
            break;
        }

        case 8: // View
        {
            menu.addItem(kViewSignalBar,       "Signal Bar",        true, false);
            menu.addItem(kViewDeck,            "Deck",              true, false);
            menu.addItem(kViewPreview,         "Preview",           true, false);
            menu.addItem(kViewInspector,       "Inspector",         true, false);
            menu.addItem(kViewBrowser,         "Browser",           true, false);
            menu.addItem(kViewTimingWindow,    "Timing Window",     true, false);
            menu.addSeparator();
            menu.addItem(kViewFpsStats,        "FPS and Stats",     true, false);
            menu.addItem(kViewProgrammingMode, "Programming Mode",  true, false);
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
