#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "features/FeatureBus.h"
#include "analysis/FeatureSnapshot.h"
#include "model/Composition.h"
#include "ui/LookAndFeel.h"

// TopBar: the main application toolbar below the menu bar.
// Contains: audio source, gain, transport, tempo display,
// tap/resync, BPM multiplier, quantize, fade, FPS/DSP stats.
class TopBar : public juce::Component, private juce::Timer
{
public:
    TopBar(FeatureBus& featureBus, Composition& composition);

    void paint(juce::Graphics& g) override;
    void resized() override;

    // Callbacks for actions that MainComponent handles
    std::function<void(float bpm)> onTapTempo;  // Called with computed BPM from taps
    std::function<void()> onResync;
    std::function<void(int)> onBpmMultiplierChanged;
    std::function<void(Composition::QuantizeMode)> onQuantizeChanged;
    std::function<void(bool manualMode, float bpm)> onManualBpmChanged;

    // Update stats
    void setFps(float fps);
    void setDspLoad(float percent);

    // Access audio controls
    juce::ComboBox& getAudioSourceSelector() { return audioSourceSelector_; }
    juce::Slider& getInputGainSlider() { return inputGainSlider_; }
    juce::Slider& getMasterLevelSlider() { return masterLevelSlider_; }

    // Access display selector
    juce::ComboBox& getDisplaySelector() { return displaySelector_; }

private:
    void timerCallback() override;
    void updateBpmDisplay();
    void handleMultiplierButton(int multiplier);

    FeatureBus& featureBus_;
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
    juce::Slider inputGainSlider_;

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
    juce::Slider fadeSlider_;

    // === Master Level ===
    juce::Label masterLabel_{"", "Master:"};
    juce::Slider masterLevelSlider_;

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

    // Tap tempo state
    std::array<double, 8> tapTimes_{};
    int tapCount_ = 0;
    double lastTapTime_ = 0.0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TopBar)
};
