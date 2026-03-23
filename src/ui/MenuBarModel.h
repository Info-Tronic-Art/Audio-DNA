#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "model/Composition.h"
#include <functional>

// AudioDNAMenuBar: Implements the 9-menu application menu bar.
// Menus: Audio-DNA, Composition, Deck, Layer, Column, Clip, Output, Shortcuts, View
class AudioDNAMenuBar : public juce::MenuBarModel
{
public:
    AudioDNAMenuBar();

    juce::StringArray getMenuBarNames() override;
    juce::PopupMenu getMenuForIndex(int menuIndex, const juce::String& menuName) override;
    void menuItemSelected(int menuItemID, int topLevelMenuIndex) override;

    // Command IDs
    enum CommandID
    {
        // Audio-DNA menu
        kPreferences = 1000,
        kAbout,
        kQuit,
        kImportISF,

        // Composition menu
        kCompNew = 1100,
        kCompOpen,
        kCompSave,
        kCompSaveAs,
        kCompCopyEffects,
        kCompPasteEffects,

        // Deck menu
        kDeckNew = 1200,
        kDeckInsertBefore,
        kDeckInsertAfter,
        kDeckDuplicate,
        kDeckRename,
        kDeckClose,
        kDeckClearClips,
        kDeckRemove,

        // Layer menu
        kLayerNew = 1300,
        kLayerInsertAbove,
        kLayerInsertBelow,
        kLayerDuplicate,
        kLayerRename,
        kLayerCopyEffects,
        kLayerPasteEffects,
        kLayerClearClips,
        kLayerRemove,
        kLayerIgnoreColumnTrigger,
        kLayerLockContent,

        // Column menu
        kColumnNew = 1400,
        kColumnInsertBefore,
        kColumnInsertAfter,
        kColumnDuplicate,
        kColumnClearClips,
        kColumnRemove,
        kColumnRemoveAllBefore,
        kColumnRemoveAllAfter,

        // Clip menu
        kClipSelectAll = 1500,
        kClipCut,
        kClipCopy,
        kClipPaste,
        kClipCopyEffects,
        kClipPasteEffects,
        kClipRename,
        kClipClear,
        kClipShowInFinder,
        kClipNewSource,
        kClipNewEffect,

        // Output menu
        kOutputDisabled = 1600,
        kOutputFullscreenBase,  // +displayIndex for each display
        kOutputWindowed = 1690,
        kOutputIdentifyDisplays,
        kOutputTestCard,
        kOutputSnapshot,
        kOutputStartRecording,
        kOutputStopRecording,

        // Shortcuts menu
        kShortcutsEditKeyboard = 1700,
        kShortcutsEditMIDI,
        kShortcutsStop,

        // View menu
        kViewSignalBar = 1800,
        kViewDeck,
        kViewPreview,
        kViewInspector,
        kViewBrowser,
        kViewTimingWindow,
        kViewFpsStats,
        kViewProgrammingMode,
    };

    // Callbacks — MainComponent wires these
    std::function<void(int)> onMenuCommand;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioDNAMenuBar)
};
