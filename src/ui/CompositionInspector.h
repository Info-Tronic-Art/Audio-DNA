#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "model/Composition.h"
#include "routing/MacroBank.h"
#include "effects/EffectLibrary.h"
#include "signal/SignalRegistry.h"
#include "ui/MacroPanel.h"
#include "ui/EffectStackView.h"
#include "ui/UniversalParamControl.h"
#include "ui/LookAndFeel.h"

// CompositionInspector: Resolume-style composition properties panel.
//
// Sections (top to bottom, matching Resolume Composition tab):
//   [Name + Resolution]       "Example (1280 x 720)"  search + gear icons
//   [Dashboard]               8 link knobs
//   [Autopilot]               Direction (◀◀ OFF ▶▶ ⤮), Duration, Clip Loops, Loop, Master Layer
//   [Composition]             ▶ Master slider (with signal triangle), Speed slider
//   [Video]                   ▶ Opacity slider
//   [Transform]               Position X/Y, Scale %, Rotation °, Anchor
//   [Global Effects]          Effect stack
//   [Output Settings]         Resolution dropdown
class CompositionInspector : public juce::Component
{
public:
    CompositionInspector();

    void paint(juce::Graphics& g) override;
    void resized() override;

    void setComposition(Composition* comp);
    Composition* getComposition() const { return composition_; }

    void setEffectLibrary(EffectLibrary* lib);
    void setSignalRegistry(SignalRegistry* reg);
    void setMacroBank(MacroBank* bank);

    void refresh();
    int getPreferredHeight() const;

    // Undo v1 step 7: hand the effect stack the host's performEdit hook.
    void setEffectPerformEdit(EffectStackView::PerformEditFn cb)
    {
        effectStackView_.onPerformEdit = std::move(cb);
    }

    // Rebuild the global-effects stack from the live vector — used after an
    // undo/redo of a global effect add/remove/bypass (the selected-cell re-points
    // in refreshAfterUndoRedo don't reach the composition's own chain).
    void rebuildEffectStack();

private:
    Composition* composition_ = nullptr;
    SignalRegistry* signalRegistry_ = nullptr;

    // --- Dashboard ---
    MacroPanel macroPanel_;

    // --- Autopilot ---
    juce::TextButton apRewindBtn_;
    juce::TextButton apOffBtn_{"OFF"};
    juce::TextButton apForwardBtn_;
    juce::TextButton apRandomBtn_;
    juce::ComboBox apDurationSelector_;
    ResettableSlider apClipLoopsSlider_;
    juce::ToggleButton apLoopToggle_{"Loop"};
    juce::ComboBox apMasterLayerSelector_;

    // --- Per-Type Autopilot (P20) ---
    juce::ToggleButton perTypeEnabledToggle_{"Per-Type"};
    ResettableSlider opaqueCycleSlider_;
    ResettableSlider transparentCycleSlider_;
    ResettableSlider effectCycleSlider_;
    juce::ToggleButton transparentRandomToggle_{"Randomize"};
    juce::ToggleButton effectRandomToggle_{"Randomize"};

    // --- Composition (Master + Speed) ---
    UniversalParamControl masterControl_;
    UniversalParamControl speedControl_;

    // --- Video ---
    UniversalParamControl opacityControl_;

    // --- Transform ---
    UniversalParamControl posXControl_;
    UniversalParamControl posYControl_;
    UniversalParamControl scaleControl_;
    UniversalParamControl rotationControl_;
    UniversalParamControl anchorControl_;

    // --- Global Effects ---
    EffectStackView effectStackView_;

    // --- Output Settings ---
    juce::ComboBox resolutionSelector_;

    void paintSectionHeader(juce::Graphics& g, const juce::Rectangle<int>& bounds,
                            const juce::String& title, bool hasPButton = false);
    void syncFromComposition();

    static constexpr int kSectionHeaderHeight = 18;
    static constexpr int kSectionGap = 4;
    static constexpr int kRowHeight = 22;
    static constexpr int kNameBarHeight = 26;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CompositionInspector)
};
