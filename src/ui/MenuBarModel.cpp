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
            // Dynamic Undo/Redo: "Undo <description>", enabled flag, and the
            // Cmd+Z / Cmd+Shift+Z shortcut text, driven by injected providers.
            {
                juce::PopupMenu::Item undoItem;
                undoItem.itemID = kCompUndo;
                undoItem.shortcutKeyDescription = "Cmd+Z";
                if (getUndoState)
                {
                    auto state = getUndoState();
                    undoItem.text = state.first.isNotEmpty() ? "Undo " + state.first : "Undo";
                    undoItem.isEnabled = state.second;
                }
                else
                {
                    undoItem.text = "Undo";
                    undoItem.isEnabled = false;
                }
                menu.addItem(undoItem);

                juce::PopupMenu::Item redoItem;
                redoItem.itemID = kCompRedo;
                redoItem.shortcutKeyDescription = "Cmd+Shift+Z";
                if (getRedoState)
                {
                    auto state = getRedoState();
                    redoItem.text = state.first.isNotEmpty() ? "Redo " + state.first : "Redo";
                    redoItem.isEnabled = state.second;
                }
                else
                {
                    redoItem.text = "Redo";
                    redoItem.isEnabled = false;
                }
                menu.addItem(redoItem);
            }
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
            // plan6 §6.3: mirrors the deck tab row ("+" and the tab right-click menu); every item acts on the
            // ACTIVE deck (a menu cannot know a tab). Remove Deck stays enabled here -- its handler guards.
            menu.addItem(kDeckNew,          "New Deck",              true, false);
            menu.addItem(kDeckLoad,         "Load Deck...",          true, false);
            menu.addSeparator();
            menu.addItem(kDeckSave,         "Save Deck",             true, false);
            menu.addItem(kDeckSaveAs,       "Save Deck As...",       true, false);
            menu.addSeparator();
            menu.addItem(kDeckRename,       "Rename Deck...",        true, false);
            menu.addItem(kDeckDuplicate,    "Duplicate Deck",        true, false);
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
            const bool hasSelection = hasClipSelection && hasClipSelection();
            menu.addItem(kClipClear,        "Clear",                   hasSelection, false);
            menu.addSeparator();
            menu.addItem(kClipReplaceContent, "Replace Content...",    hasSelection, false);
            menu.addItem(kClipLockContent,    "Lock Content",          hasSelection, false);
            break;
        }

        case 6: // Output
        {
            // plan5 C2: one tickable item per connected display (kOutputFullscreenBase + i), then "All Outputs Off"
            // (kOutputDisabled) -- from OutputManager, the same list as the TopBar "Outputs" button.
            if (populateOutputItems)
                populateOutputItems(menu);

            menu.addSeparator();
            menu.addItem(kOutputSnapshot,         "Snapshot",            true, false);
            menu.addSeparator();
            menu.addItem(kOutputStartRecording,   "Start Recording",     true, false);
            menu.addItem(kOutputStopRecording,    "Stop Recording",      true, false);
            menu.addSeparator();
            const bool syphonOn = isSyphonOutputEnabled && isSyphonOutputEnabled();
            menu.addItem(kOutputSyphon,           "Syphon Output",       true, syphonOn);
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
