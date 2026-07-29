#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "model/Clip.h"
#include "routing/MacroBank.h"
#include "effects/EffectLibrary.h"
#include "signal/SignalRegistry.h"
#include "ui/MacroPanel.h"
#include "ui/EffectStackView.h"
#include "ui/UniversalParamControl.h"
#include "ui/LookAndFeel.h"

// ClipInspector: Resolume-style clip properties panel.
//
// Sections (matching Resolume Clip tab):
//   [Name + Thumbnail]        "Metalive"  search + gear icons
//   [Dashboard]               8 link knobs
//   [Transport]               Mode dropdown, playhead, ◀ ⏸ ▶, loop/trigger, Speed, Duration ½/×2
//   [Cuepoints]               6-8 cue buttons
//   [Autopilot]               Direction, Duration, Action
//   [Source Parameters]       (Source clips only) UniversalParamControls with triangles
//   [Video]                   Opacity, Width, Height, Blend Mode, Alpha Type
//   [RGBA Toggles]            R G B A channel toggle buttons
//   [Transform]               Position X/Y, Scale %, Rotation °, Anchor
//   [Effects]                 Effect stack
class ClipInspector : public juce::Component,
                      public juce::DragAndDropTarget
{
public:
    ClipInspector();

    void paint(juce::Graphics& g) override;
    void resized() override;

    // scope: the effect-chain coordinate for this clip (deck, layer, column), so
    // effect-stack edits become undo commands that re-resolve by coordinate. A
    // None scope leaves effect edits un-undoable but never mis-targets.
    void setClip(Clip* clip, EffectScope scope = EffectScope::none());
    Clip* getClip() const { return clip_; }

    void setEffectLibrary(EffectLibrary* lib);
    void setSignalRegistry(SignalRegistry* reg);
    void setMacroBank(MacroBank* bank);

    // Undo v1 step 7: hand the effect stack the host's performEdit hook.
    void setEffectPerformEdit(EffectStackView::PerformEditFn cb)
    {
        effectStackView_.onPerformEdit = std::move(cb);
    }

    // Family-fence fix round 2 (2026-07-28): hand the effect stack the GL
    // fence hook (structural push_back/erase — see EffectStackView.h).
    void setEffectFenceHook(EffectStackView::EffectFenceHook hook)
    {
        effectStackView_.setFenceHook(std::move(hook));
    }

    void refresh();
    int getPreferredHeight() const;

    std::function<void(Clip* clip)> onSourceParamsChanged;

    // Called when user wants to jump to a cuepoint position [0,1]
    std::function<void(Clip* clip, double normalizedPosition)> onCuepointJump;

    // Called when a cuepoint is set (for potential undo tracking)
    std::function<void(Clip* clip, int cuepointIndex)> onCuepointSet;

private:
    Clip* clip_ = nullptr;
    SignalRegistry* signalRegistry_ = nullptr;
    MacroBank* macroBank_ = nullptr;

    // --- Dashboard ---
    MacroPanel macroPanel_;

    // --- Transport ---
    juce::ComboBox transportModeSelector_;
    ResettableSlider speedSlider_;
    juce::TextButton reverseBtn_{"Reverse"};
    juce::TextButton halfSpeedBtn_;
    juce::TextButton doubleSpeedBtn_;
    juce::ComboBox loopModeSelector_;
    // Transport control buttons
    juce::TextButton playBackBtn_;
    juce::TextButton pauseBtn_;
    juce::TextButton playBtn_;
    juce::ComboBox loopDropdown_;
    juce::ComboBox triggerDropdown_;
    // Duration
    ResettableSlider durationSlider_;
    juce::TextButton durHalfBtn_;
    juce::TextButton durDoubleBtn_;

    // --- Cuepoints ---
    static constexpr int kNumCuepoints = 8;
    std::array<std::unique_ptr<juce::TextButton>, kNumCuepoints> cuepointBtns_;    // trigger (jump)
    std::array<std::unique_ptr<juce::TextButton>, kNumCuepoints> cuepointSetBtns_; // set at playhead

    // --- Autopilot ---
    juce::ComboBox autopilotActionSelector_;
    juce::ComboBox autopilotDurationSelector_;

    // --- Beat Snap ---
    juce::ComboBox beatSnapSelector_;

    // --- Image Sequence FPS ---
    ResettableSlider sequenceFpsSlider_;
    juce::Label sequenceFpsLabel_;

    // --- Beat Division (BPM Sync mode) ---
    juce::ComboBox beatDivisionSelector_;
    juce::Label beatDivisionLabel_;

    // --- Content Beats (BPM Sync mode) ---
    ResettableSlider videoBeatsSlider_;
    juce::Label videoBeatsLabel_;

    // --- Source Parameters ---
    std::vector<std::unique_ptr<UniversalParamControl>> sourceParamControls_;
    void buildSourceParamControls();

    // --- Video ---
    UniversalParamControl clipOpacityControl_;
    ResettableSlider clipWidthSlider_;
    ResettableSlider clipHeightSlider_;
    juce::ComboBox clipBlendModeSelector_;
    juce::ComboBox clipAlphaTypeSelector_;
    // RGBA toggles
    juce::ToggleButton channelRBtn_{"R"};
    juce::ToggleButton channelGBtn_{"G"};
    juce::ToggleButton channelBBtn_{"B"};
    juce::ToggleButton channelABtn_{"A"};

    // --- Transform ---
    UniversalParamControl posXControl_;
    UniversalParamControl posYControl_;
    UniversalParamControl scaleControl_;
    UniversalParamControl rotationControl_;
    UniversalParamControl anchorControl_;

    // --- Effects ---
    EffectStackView effectStackView_;

    // --- Timeline bar (in/out points, playhead, beat markers) ---
    juce::Rectangle<int> timelineBounds_;  // bounds of the timeline track
    enum class DragTarget { None, InPoint, OutPoint, Playhead };
    DragTarget currentDrag_ = DragTarget::None;

    void mouseDown(const juce::MouseEvent& event) override;
    void mouseDrag(const juce::MouseEvent& event) override;
    void mouseUp(const juce::MouseEvent& event) override;
    float timelineXToNormalized(int x) const;
    int normalizedToTimelineX(float norm) const;
    void paintTimeline(juce::Graphics& g, const juce::Rectangle<int>& bounds) const;

    // DragAndDropTarget for FX drops
    bool isInterestedInDragSource(const SourceDetails& details) override;
    void itemDragEnter(const SourceDetails& details) override;
    void itemDragExit(const SourceDetails& details) override;
    void itemDropped(const SourceDetails& details) override;
    bool fxDropHighlight_ = false;

    void paintSectionHeader(juce::Graphics& g, const juce::Rectangle<int>& bounds,
                            const juce::String& title, bool hasPButton = false);
    void populateDropdowns();
    void syncFromClip();
    void updateTransportHighlights();

    static constexpr int kSectionHeaderHeight = 20;
    static constexpr int kSectionGap = 8;
    static constexpr int kRowHeight = 24;
    static constexpr int kNameBarHeight = 28;
    static constexpr int kTimelineHeight = 36; // timeline bar with in/out markers + handles
    static constexpr int kInset = 6;           // left/right padding within sections

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ClipInspector)
};
