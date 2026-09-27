#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "model/Layer.h"
#include "ui/LookAndFeel.h"
#include "ui/UniversalParamControl.h" // for ResettableSlider
#include "ui/RoutineDeckView.h"
#include <vector>

// LayerStrip: Resolume-style layer header — flat, dense, machine-like.
//
// Layout:
//   ┌───┬───┬───┬──────┬───┬───┬───┬───────────┬──┐
//   │ X │ B │ S │      │ S │ K │ V │           │ F│
//   ├───┴───┴───┤      │ ▓ │ ▓ │ ▓ │ thumbnail │ ▓│
//   │ < || > >| │      │ ▓ │ ▓ │ ▓ │           │ ▓│
//   └───────────┘──────┴───┴───┼───┼─playhead──┼──┤
//   │  Layer Name  │  Blend▼   │ClipName + ▏  │T▼│
//   └──────────────┴───────────┴──────────────┴──┘
//
// S = speed slider (0-4x, default 1x)
// K = keying threshold slider only (no dropdown)
// V = opacity slider + dropdown with keying types + mix modes (unified)
// F = fade speed slider + transition mix mode dropdown
// playhead = cyan vertical line over clip name area
class LayerStrip : public juce::Component,
                   public juce::DragAndDropTarget,
                   private juce::Timer
{
public:
    LayerStrip();

    void paint(juce::Graphics& g) override;
    void resized() override;

    void setLayer(Layer* layer, int index);
    Layer* getLayer() const { return layer_; }
    int getLayerIndex() const { return layerIndex_; }

    void refresh();

    // s-rta-0927 routine display (plan-routine-display-A.md 2.5): pull the V fader (opacity; eff() when
    // connected) and the S fader (the active clip's speed) from the model -- a routine, REST, MIDI or OSC
    // write moves them -- skipping a fader under the mouse; the V fill turns kRoutineCue while a lane-rank hand
    // (a routine or a take replay) grips opacity. Called by the strip's own 30 Hz timer; public for tests.
    void syncFromModel();

    // s-rta-0927 routine display (2.2): the routines playing / waiting on this layer, newest first, at most
    // two (RoutineDeckView bandsToDraw). Painted over the top of the thumbnail with the name, a progress
    // hairline and an x that takes the WHOLE routine off every layer it plays on (onRoutineRemove).
    void setRoutineBands(std::vector<RoutineDeckView::Band> bands);
    std::function<void(int slot)> onRoutineRemove;

    void setSelected(bool sel) { if (selected_ != sel) { selected_ = sel; repaint(); } }
    bool isSelected() const { return selected_; }

    // Callbacks
    std::function<void(int layerIndex)> onSelect;
    std::function<void(int layerIndex)> onClearClip;
    std::function<void(int layerIndex, bool)> onBypass;
    std::function<void(int layerIndex, bool)> onSolo;
    std::function<void(int layerIndex, Layer::MixMode)> onBlendModeChanged;
    std::function<void(int layerIndex)> onTransportPlay;
    std::function<void(int layerIndex)> onTransportPause;
    std::function<void(int layerIndex)> onTransportBack;
    std::function<void(int layerIndex)> onTransportForward;
    std::function<void(int layerIndex)> onFoldToggle;        // P24.12
    std::function<void(int fromIndex, int toIndex)> onLayerDragReorder; // P24.13
    // FX-drop-target (2026-07-30): fired when an fx: drag is dropped anywhere on
    // the strip, comma-separated names for a multi-select drop. The host owns the
    // effect library lookup + undo command — this class only forwards the raw
    // description string, mirroring ClipCell's onEffectDrop shape.
    std::function<void(int layerIndex, const juce::String& effectDesc)> onEffectDropped;

private:
    void mouseDown(const juce::MouseEvent& event) override;
    void mouseDrag(const juce::MouseEvent& event) override;
    void timerCallback() override;

    // DragAndDropTarget for FX drops (mirrors LayerInspector/CompositionInspector,
    // 9c316e6 pattern: whole component accepts fx: drags)
    bool isInterestedInDragSource(const SourceDetails& details) override;
    void itemDragEnter(const SourceDetails& details) override;
    void itemDragExit(const SourceDetails& details) override;
    void itemDropped(const SourceDetails& details) override;
    bool fxDropHighlight_ = false;

    void scrubPlayhead(juce::Point<int> pos);

    // s-rta-0927 routine bands: band k = (thumb x, thumb y + 16k, thumb w, 16); its x = the rightmost 16x16.
    static constexpr int kBandHeight = 16;
    bool bandsShown() const { return thumbnailBounds_.getHeight() >= 48; }   // a folded row shows none
    juce::Rectangle<int> bandBounds(int k) const;
    juce::Rectangle<int> bandXBounds(int k) const;
    void paintRoutineBands(juce::Graphics& g);
    std::vector<RoutineDeckView::Band> routineBands_;

    Layer* layer_ = nullptr;
    int layerIndex_ = 0;
    bool selected_ = false;

    // Left: X B S buttons (square)
    juce::TextButton clearBtn_{"X"};
    juce::TextButton bypassBtn_{"B"};
    juce::TextButton soloBtn_{"S"};

    // Transport controls (play/pause/forward/back)
    juce::TextButton transportBackBtn_;
    juce::TextButton transportPauseBtn_;
    juce::TextButton transportPlayBtn_;
    juce::TextButton transportForwardBtn_;

    // S = speed slider
    ResettableSlider speedSlider_;

    // K = keying threshold slider (no dropdown)
    ResettableSlider keyingSlider_;

    // V = opacity slider + blend/keying mode dropdown
    ResettableSlider opacitySlider_;
    juce::ComboBox blendDropdown_;

    // Thumbnail (painted manually)
    juce::Image thumbnail_;
    juce::Rectangle<int> thumbnailBounds_;

    // Name + clip name + transport display (painted manually)
    juce::Rectangle<int> nameBounds_;
    juce::Rectangle<int> clipNameBounds_;
    juce::Rectangle<int> transportBounds_; // playhead display above the name
    juce::String layerName_;
    juce::String clipName_;

    // F = fade speed slider + transition mode dropdown
    ResettableSlider fadeTimeSlider_;
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
