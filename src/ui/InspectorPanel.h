#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "model/Composition.h"
#include "model/Layer.h"
#include "model/Clip.h"
#include "effects/EffectLibrary.h"
#include "signal/Signal.h"
#include "signal/SignalRegistry.h"
#include "routing/MacroBank.h"
#include "ui/ClipInspector.h"
#include "ui/LayerInspector.h"
#include "ui/CompositionInspector.h"
#include "ui/SignalInspector.h"
#include "ui/LookAndFeel.h"
#include <memory>

// InspectorPanel: 4-tab inspector — Clip, Layer, Composition, Signal.
// Auto-switches based on what the user last clicked:
//   - Click a clip → Clip tab
//   - Click a layer strip → Layer tab
//   - Click a signal strip → Signal tab
//   - Composition tab always available manually
class InspectorPanel : public juce::Component
{
public:
    InspectorPanel();

    void paint(juce::Graphics& g) override;
    void resized() override;

    // Set dependencies
    void setComposition(Composition* comp);
    void setEffectLibrary(EffectLibrary* lib);
    void setSignalRegistry(SignalRegistry* reg);
    void setMacroBank(MacroBank* bank);

    // Selection callbacks — auto-switch tab. The EffectScope (default None) is the
    // effect-chain coordinate for the inspected clip/layer, so effect-stack edits
    // become undo commands that re-resolve by coordinate.
    void inspectClip(Clip* clip, EffectScope scope = EffectScope::none());
    void inspectLayer(Layer* layer, EffectScope scope = EffectScope::none());
    void inspectSignal(Signal* signal);
    void showCompositionTab();

    // Undo v1 step 7: install the shared effect-stack performEdit hook on all
    // three effect hosts (clip / layer / composition).
    void setEffectPerformEdit(EffectStackView::PerformEditFn cb);

    // Family-fence fix round 2 (2026-07-28): install the shared GL fence hook
    // on all three effect hosts (clip / layer / composition).
    void setEffectFenceHook(EffectStackView::EffectFenceHook cb);

    // Rebuild the composition (global) effect stack after an undo/redo — clip and
    // layer stacks re-point via inspect* in refreshAfterUndoRedo, but the global
    // chain has no selected cell to re-point through.
    void rebuildCompositionEffects() { compInspector_.rebuildEffectStack(); }

    // Refresh the currently visible tab
    void refresh();

    // L9 (modulation-freeze fix, 2026-09-05): tick EVERY inspector's
    // signal/macro-driven effect-param modulation, unconditionally — NOT
    // gated by activeTab_ like refresh() above. This is the line that
    // actually closes the freeze: previously the compute+write for a
    // connected param only ran while its owning tab was the active one, so a
    // "connected" param silently stopped modulating the instant the operator
    // looked away. Painting/display-sync still only happens for the active
    // tab via refresh().
    void tickModulation();

    enum class Tab : int { Clip = 0, Layer = 1, Composition = 2, Signal = 3 };
    void setActiveTab(Tab tab);
    Tab getActiveTab() const { return activeTab_; }

    // Access individual inspectors
    ClipInspector& getClipInspector() { return clipInspector_; }
    LayerInspector& getLayerInspector() { return layerInspector_; }

    // P24.11: Pin inspector — prevents auto-switching tabs during performance
    bool isPinned() const { return pinned_; }
    void setPinned(bool p) { pinned_ = p; updatePinButton(); }

private:
    Tab activeTab_ = Tab::Clip;

    // Tab buttons
    juce::TextButton clipTabBtn_{"Clip"};
    juce::TextButton layerTabBtn_{"Layer"};
    juce::TextButton compTabBtn_{"Composition"};
    juce::TextButton signalTabBtn_{"Signal"};

    // Scrollable content viewports
    juce::Viewport clipViewport_;
    juce::Viewport layerViewport_;
    juce::Viewport compViewport_;
    juce::Viewport signalViewport_;

    // Tab content
    ClipInspector clipInspector_;
    LayerInspector layerInspector_;
    CompositionInspector compInspector_;
    SignalInspector signalInspector_;

    Composition* composition_ = nullptr;
    bool pinned_ = false;
    juce::TextButton pinBtn_{"Pin"};

    static constexpr int kTabBarHeight = 26;

    void updateTabButtonColors();
    void showActiveTab();
    void updatePinButton();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(InspectorPanel)
};
