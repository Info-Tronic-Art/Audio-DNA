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
#include "ui/ClipMediaText.h"
#include <optional>

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

    // s-rta-0928b idlepaint (Pitfall 57): EVERY input paint() / paintTimeline() / paintSectionHeader() read. refresh()
    // (10 Hz) repaints only when it changes; the child widgets repaint themselves. A superset is fine; a missing field
    // is a stale inspector -- add, never remove. Public for tests/test_clip_inspector_paint_key.cpp.
    struct PaintKey
    {
        const Clip* clip = nullptr;
        bool fxDrop = false;
        std::string name;
        int mediaType = 0, transportMode = 0;
        bool playable = false;
        double playhead = 0.0;
        float inPoint = 0.0f, outPoint = 0.0f, beatDivision = 0.0f;
        int sourceParamControls = 0, sectionHeights = 0, width = 0, height = 0;
        int infoRows = 0;               // s-rta-1002b ui U2.4: the file-info rows under the name bar (paint offsets)
        bool revealShown = false;       // s-rta-1002b ui U2.4: the name is trimmed for the Show in Finder button
        bool operator==(const PaintKey&) const = default;
    };
    PaintKey paintKeyNow() const;

    // L9 (modulation-freeze fix, 2026-09-05): compute+apply signal/macro-
    // driven values for the effect stack AND this inspector's own Source-clip
    // param loop — the two things refresh() used to compute only while the
    // Clip tab was active. Driven unconditionally from InspectorPanel::
    // tickModulation() / MainComponent::tickFeaturePipeline()'s 120Hz timer.
    void tickModulation();

    int getPreferredHeight() const;

    std::function<void(Clip* clip)> onSourceParamsChanged;

    // Called when user wants to jump to a cuepoint position [0,1]
    std::function<void(Clip* clip, double normalizedPosition)> onCuepointJump;

    // Called when a cuepoint is set (for potential undo tracking)
    std::function<void(Clip* clip, int cuepointIndex)> onCuepointSet;

    // s-rta-1002b ui U2.4 (BF3 "Codec display for each video and easy access to that video in finder"; ruling-ui.md
    // AM9): for a clip with a file (video, picture, image sequence) a "Show in Finder" button at the right of the name
    // bar and one or two info rows under it (a video: its codec, then its size + frame rate; clipmedia::describe).
    // A Video clip's info comes from this source (MainComponent::videoInfoFor: the player open for the clip).
    void setVideoInfoSource(VideoInfoSource source);
    std::function<void(Clip* clip)> onRevealInFinder;
    static constexpr int kRevealButtonWidth = 104;   // "Show in Finder" is 84.7 px at the 14-px button font (AM9 (h))
    static constexpr int kInfoRowHeight = 18;
    // Pure layout (tests): where the name text goes in a `width`-wide name bar, and the button's place in it.
    static juce::Rectangle<int> nameTextBounds(int width, bool buttonShown);
    static juce::Rectangle<int> revealButtonBounds(int width);
    // What the info rows and the button show now (tests and /api/debug/clip_media).
    juce::StringArray mediaInfoLinesShown() const;
    bool revealButtonShown() const { return revealShown_; }
    juce::TextButton& revealButtonForTests() { return revealBtn_; }

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

    // L9 fix-round (blocking review finding): the last value actually pushed
    // (to sourceParamControls_[i]'s display AND to onSourceParamsChanged,
    // which feeds Renderer::updateActiveSourceParams — the standalone-source
    // render path) — parallel to sourceParamControls_, resized alongside it
    // in buildSourceParamControls(). nullopt means never pushed yet.
    // Deliberately NOT clip_->sourceParams[i].value — that field is
    // overwritten unconditionally every tick by tickModulation() itself.
    std::vector<std::optional<float>> lastPushedSourceParam_;

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
    // s-rta-0926b plan-fitmode: Stretch / Bars / Crop (first row of the Transform section)
    juce::Label fitLabel_;
    juce::ComboBox fitSelector_;
    juce::Label fitCaption_;            // fix round: what the selected mode does (not hover-only)
    void updateFitCaption();

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
    PaintKey lastPaintKey_;               // s-rta-0928b idlepaint: what the last refresh() repaint showed
    bool paintKeyValid_ = false;

    // --- s-rta-1002b ui U2.4: file info + Show in Finder ---
    VideoInfoSource videoInfoSource_;
    juce::TextButton revealBtn_ { "Show in Finder" };
    juce::Label mediaInfoLabel1_, mediaInfoLabel2_;
    int infoRows_ = 0;                    // labels shown (0, 1 or 2)
    bool revealShown_ = false;
    // What the rows were made from: re-described only when this changes, or while a Video clip's player has not
    // answered yet (a player that opens later is picked up by the next refresh; bounded to that case).
    struct MediaKey
    {
        const Clip* clip = nullptr;
        int mediaType = 0;
        juce::String path;
        bool missing = false;
        bool operator==(const MediaKey&) const = default;
    };
    MediaKey mediaKey_;
    bool mediaKeyValid_ = false;
    bool videoPending_ = false;
    void updateMediaInfo(bool force);
    int infoRowHeight() const { return kInfoRowHeight * infoRows_; }

    void paintSectionHeader(juce::Graphics& g, const juce::Rectangle<int>& bounds,
                            const juce::String& title, bool hasPButton = false);
    void populateDropdowns();
    void syncFromClip();
    void updateTransportHighlights();
    // s-rta-0923 lane 3 plan section 4.2: bind the 6 scalar controls to
    // clip_->scalarConns/scalarLive (null-safe when clip_ is nullptr).
    void bindScalarControls();
    // Lane bf9b fix (AM-1): drops the 6 scalar bindings WITHOUT reading them -- setClip's first call.
    void forgetScalarBindings();

    static constexpr int kSectionHeaderHeight = 20;
    static constexpr int kSectionGap = 8;
    static constexpr int kRowHeight = 24;
    static constexpr int kFitCaptionHeight = 16;   // the Fit caption line under the Fit combo
    static constexpr int kNameBarHeight = 28;
    static constexpr int kTimelineHeight = 36; // timeline bar with in/out markers + handles
    static constexpr int kInset = 6;           // left/right padding within sections
    // Room reserved below the (always-live) Dashboard for the "No clip selected" message -- paired with
    // getPreferredHeight()'s no-clip branch so the message's reserved area in paint() is never zero-height.
    static constexpr int kEmptyStateHeight = 60;

    // L9 cost trap: tickModulation() now runs unconditionally at 120Hz
    // (previously ~10Hz and only while the Clip tab was active). Only push a
    // new sourceValue (and pay its unconditional repaint(), and the
    // onSourceParamsChanged render-callback) when it moved by more than this
    // SINCE THE LAST VALUE ACTUALLY PUSHED (lastPushedSourceParam_) — not
    // since the last tick. A tick-to-tick epsilon check would suppress a
    // slow modulator's push forever (fix-round finding); this one only
    // suppresses a truly-static repeat.
    static constexpr float kModulationChangeEpsilon = 0.0005f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ClipInspector)
};
