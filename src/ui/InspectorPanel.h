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

    // Selection callbacks — auto-switch tab
    void inspectClip(Clip* clip);
    void inspectLayer(Layer* layer);
    void inspectSignal(Signal* signal);
    void showCompositionTab();

    // Refresh the currently visible tab
    void refresh();

    enum class Tab : int { Clip = 0, Layer = 1, Composition = 2, Signal = 3 };
    void setActiveTab(Tab tab);
    Tab getActiveTab() const { return activeTab_; }

    // Access individual inspectors
    ClipInspector& getClipInspector() { return clipInspector_; }
    LayerInspector& getLayerInspector() { return layerInspector_; }

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

    static constexpr int kTabBarHeight = 26;

    void updateTabButtonColors();
    void showActiveTab();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(InspectorPanel)
};
