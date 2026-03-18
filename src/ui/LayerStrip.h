#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "model/Layer.h"
#include "ui/LookAndFeel.h"

// LayerStrip: Resolume-style layer header — flat, dense, machine-like.
//
// Layout:
//   ┌───┬───┬───┬──────┬───┬───┬───────────┬──┐
//   │ X │ B │ S │      │ K │ V │           │ F│
//   ├───┴───┴───┤      │ ▓ │ ▓ │ thumbnail │ ▓│
//   │ Layer 1 ●A│      │ ▓ │ ▓ │           │ ▓│
//   └───────────┘──────┴───┼───┼───────────┼──┤
//                          │Add│           │Dis│
//                          │ ▼ │           │ ▼ │
//                          └───┘           └──┘
//
// K = keying threshold slider only (no dropdown)
// V = opacity slider + dropdown with keying types + mix modes (unified)
// F = fade speed slider + transition mix mode dropdown
// ●A = green badge when clip has alpha channel
class LayerStrip : public juce::Component
{
public:
    LayerStrip();

    void paint(juce::Graphics& g) override;
    void resized() override;

    void setLayer(Layer* layer, int index);
    Layer* getLayer() const { return layer_; }
    int getLayerIndex() const { return layerIndex_; }

    void refresh();

    // Callbacks
    std::function<void(int layerIndex)> onSelect;
    std::function<void(int layerIndex)> onClearClip;
    std::function<void(int layerIndex, bool)> onBypass;
    std::function<void(int layerIndex, bool)> onSolo;
    std::function<void(int layerIndex, Layer::MixMode)> onBlendModeChanged;

private:
    void mouseDown(const juce::MouseEvent& event) override;

    Layer* layer_ = nullptr;
    int layerIndex_ = 0;

    // Left: X B S buttons (square)
    juce::TextButton clearBtn_{"X"};
    juce::TextButton bypassBtn_{"B"};
    juce::TextButton soloBtn_{"S"};

    // K = keying threshold slider (no dropdown)
    juce::Slider keyingSlider_;

    // V = opacity slider + blend/keying mode dropdown
    juce::Slider opacitySlider_;
    juce::ComboBox blendDropdown_;

    // Thumbnail (painted manually)
    juce::Image thumbnail_;
    juce::Rectangle<int> thumbnailBounds_;

    // Name + clip name (painted manually for pixel-perfect alignment)
    juce::Rectangle<int> nameBounds_;
    juce::Rectangle<int> clipNameBounds_;
    juce::String layerName_;
    juce::String clipName_;

    // F = fade speed slider + transition mode dropdown
    juce::Slider fadeTimeSlider_;
    juce::ComboBox transitionDropdown_;

    void setupFlatButton(juce::TextButton& btn);
    void updateButtonStates();
    void updateThumbnail();
    void updateClipName();
    void populateBlendDropdown();
    void populateTransitionDropdown();

    // Colors — Resolume style
    static constexpr juce::uint32 kStripBg     = 0xff1e1e1e;
    static constexpr juce::uint32 kBtnBg       = 0xff333333;
    static constexpr juce::uint32 kBtnBorder   = 0xff1a1a1a;
    static constexpr juce::uint32 kSoloActive  = 0xff7a7a4a;
    static constexpr juce::uint32 kBypassActive= 0xff6a3a3a;
    static constexpr juce::uint32 kTextDim     = 0xffe0e0e0;
    static constexpr juce::uint32 kThumbBg     = 0xff111111;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LayerStrip)
};
