#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "features/FeatureBus.h"
#include "analysis/FeatureSnapshot.h"
#include "model/Composition.h"
#include "ui/LookAndFeel.h"
#include "ui/UniversalParamControl.h" // for ResettableSlider

// TopBar: the main application toolbar below the menu bar.
// Contains: audio source, gain, transport, tempo display,
// tap/resync, BPM multiplier, quantize, fade, FPS/DSP stats.
class TopBar : public juce::Component, private juce::Timer
{
public:
    TopBar(const FeatureBus& featureBus, Composition& composition);

    void paint(juce::Graphics& g) override;
    void resized() override;

    // Callbacks for actions that MainComponent handles
    std::function<void(float bpm)> onTapTempo;  // Called with computed BPM from taps
    std::function<void()> onResync;
    // Global transport: play/pause/stop the active deck's layers (see MainComponent).
    std::function<void()> onPlay;
    std::function<void()> onPause;
    std::function<void()> onStop;
    std::function<void(int)> onBpmMultiplierChanged;
    std::function<void(Composition::QuantizeMode)> onQuantizeChanged;
    std::function<void(bool manualMode, float bpm)> onManualBpmChanged;
    std::function<void(bool enabled)> onLinkToggled;

    // Update stats
    void setFps(float fps);
    void setDspLoad(float percent);

    // Access audio controls
    juce::ComboBox& getAudioSourceSelector() { return audioSourceSelector_; }
    juce::Slider& getInputGainSlider() { return inputGainSlider_; }
    // Test seam (tests/test_master_opacity_link.cpp); production wiring
    // lives in TopBar.cpp.
    ResettableSlider& getMasterLevelSlider() { return masterLevelSlider_; }
    // Test seam (tests/test_master_signal_link.cpp); production wiring
    // lives in TopBar.cpp. Master Signal (s-rta-0925 mastersignal Step 1).
    ResettableSlider& getMasterSignalSlider() { return masterSignalSlider_; }

    // s-rta-0925 link: pull the fader from the model -- manual field when
    // not connected, toNorm(eff()) when a signal drives it (same rule as
    // CompositionInspector::syncFromComposition). Skipped mid-drag. Called
    // from timerCallback (15 Hz) and directly by tests
    // (tests/test_master_opacity_link.cpp).
    void syncMasterFromComposition();
    // Same rule, for the Signal fader / CompScalar::Signal (tests/
    // test_master_signal_link.cpp).
    void syncMasterSignalFromComposition();

    // Layout test seams (tests/test_master_signal_link.cpp): confirm the
    // "Signal:" label sits to the right of the existing Fade slider (no
    // overlap with the widget immediately to its left in the bar).
    juce::Rectangle<int> masterSignalLabelBoundsForTest() const { return masterSignalLabel_.getBounds(); }
    juce::Rectangle<int> fadeSliderBoundsForTest() const { return fadeSlider_.getBounds(); }

    // Access display selector
    juce::ComboBox& getDisplaySelector() { return displaySelector_; }

private:
    void timerCallback() override;
    void updateBpmDisplay();
    void handleMultiplierButton(int multiplier);

    const FeatureBus& featureBus_;
    Composition& composition_;

    // Latest snapshot for tempo display + beat wheel
    FeatureSnapshot displaySnap_{};

    // Beat wheel state
    juce::Rectangle<int> beatWheelBounds_;
    juce::Rectangle<int> barPhraseBounds_; // area for bar/phrase text display
    void paintBeatWheel(juce::Graphics& g) const;
    void paintBarPhraseDisplay(juce::Graphics& g) const;

    // === Audio Source Section ===
    juce::Label audioSourceLabel_{"", "Audio:"};
    juce::ComboBox audioSourceSelector_;
    juce::Label inputGainLabel_{"", "Gain:"};
    ResettableSlider inputGainSlider_;

    // === Transport Section ===
    juce::TextButton playButton_{">"};
    juce::TextButton pauseButton_{"||"};
    juce::TextButton stopButton_{"[]"};

    // === Tempo Section ===
    juce::Label tempoLabel_;
    juce::Label trackerStateLabel_;
    juce::TextButton tapButton_{"Tap"};
    juce::TextButton resyncButton_{"Resync"};

    // BPM Multiplier buttons
    juce::TextButton multDiv4Button_{"/4"};
    juce::TextButton multDiv2Button_{"/2"};
    juce::TextButton multX1Button_{"x1"};
    juce::TextButton multX2Button_{"x2"};
    juce::TextButton multX4Button_{"x4"};

    // === Quantize Section ===
    juce::Label quantizeLabel_{"", "Quantize:"};
    juce::ComboBox quantizeSelector_;

    // === Fade Section ===
    juce::Label fadeLabel_{"", "Fade:"};
    ResettableSlider fadeSlider_;

    // === Master Signal (s-rta-0925 mastersignal Step 1) ===
    // Sits immediately left of Master: a second widget-grip view of
    // CompScalar::Signal / Composition::masterSignal, built exactly like
    // Master's own fader below.
    juce::Label masterSignalLabel_{"", "Signal:"};
    ResettableSlider masterSignalSlider_;
    bool signalDragging_ = false;   // sync skips while dragging

    // === Master Level ===
    juce::Label masterLabel_{"", "Master:"};
    ResettableSlider masterLevelSlider_;
    bool masterDragging_ = false;   // s-rta-0925 link: sync skips while dragging

    // === Output ===
    juce::Label outputLabel_{"", "Output:"};
    juce::ComboBox displaySelector_;

    // === Stats ===
    juce::Label fpsLabel_;
    juce::Label dspLabel_;
    float currentFps_ = 0.0f;
    float currentDspLoad_ = 0.0f;

    // Manual BPM mode
    juce::ToggleButton manualModeBtn_{"Manual"};
    juce::TextEditor bpmEditField_;
    bool manualMode_ = false;

    // Ableton Link toggle (L7-JUKE). Not seeded from Composition — LinkSync's
    // own enabled_ defaults false and nothing else calls setEnabled(), so
    // this toggle starts unchecked in step with linkSync_'s real state; it
    // has no post-composition-load staleness exposure like TopBar's other
    // composition-seeded widgets (see TopBar's known stale-widget issue).
    juce::ToggleButton linkToggleBtn_{"Link"};

    // Tap tempo state
    std::array<double, 8> tapTimes_{};
    int tapCount_ = 0;
    double lastTapTime_ = 0.0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TopBar)
};
