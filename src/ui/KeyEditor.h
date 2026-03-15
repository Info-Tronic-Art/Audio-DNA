#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "keyboard/KeySlot.h"
#include "effects/EffectLibrary.h"

// KeyEditor: panel for configuring a single keyboard slot.
// Shows media selector, transparency controls, effects list,
// and playback behavior settings. Opens when a key is clicked.
class KeyEditor : public juce::Component
{
public:
    KeyEditor(EffectLibrary& effectLibrary);

    void setKey(KeySlot* key);
    KeySlot* getKey() const { return currentKey_; }

    void paint(juce::Graphics& g) override;
    void resized() override;

    // Callback to request an image file chooser
    std::function<void(KeySlot& key)> onRequestImage;
    // Callback when editor is closed
    std::function<void()> onClose;

private:
    void refreshFromKey();
    void applyToKey();

    EffectLibrary& effectLibrary_;
    KeySlot* currentKey_ = nullptr;

    // Header
    juce::Label titleLabel_;
    juce::TextButton closeButton_{"X"};

    // Media section
    juce::TextButton openImageBtn_{"Open Image"};
    juce::TextButton clearMediaBtn_{"Clear"};
    juce::Label mediaInfoLabel_;

    // Keying + Blend section
    juce::Label keyingLabel_{"", "Keying"};
    juce::ComboBox keyingModeSelector_;
    juce::Label blendLabel_{"", "Blend"};
    juce::ComboBox blendModeSelector_;
    juce::Slider opacitySlider_;
    juce::Label opacityLabel_{"", "Opacity"};
    juce::Slider thresholdSlider_;
    juce::Label thresholdLabel_{"", "Threshold"};
    juce::Slider softnessSlider_;
    juce::Label softnessLabel_{"", "Softness"};
    juce::TextButton chromaColorBtn_{"Key Color"};
    juce::Label chromaColorLabel_{"", "Color"};
    juce::Slider chromaToleranceSlider_;
    juce::Label chromaToleranceLabel_{"", "Tolerance"};
    juce::Slider chromaSoftnessSlider_;
    juce::Label chromaSoftnessLabel_{"", "Softness"};

    // Playback section
    juce::Label playbackLabel_{"", "Playback"};
    juce::ToggleButton latchToggle_{"Latch"};
    juce::ToggleButton ignoreRandomToggle_{"Ignore Random"};
    juce::ComboBox randomBeatSelector_;
    juce::Label randomBeatLabel_{"", "Random Beats"};

    // Effects section
    juce::Label effectsLabel_{"", "Effects"};
    juce::ComboBox addEffectSelector_;
    juce::TextButton addEffectBtn_{"+ Add"};
    juce::TextButton clearEffectsBtn_{"Clear FX"};

    // Effects list display
    struct EffectRow
    {
        std::unique_ptr<juce::ToggleButton> enableBtn;
        std::unique_ptr<juce::Label> nameLabel;
        std::unique_ptr<juce::TextButton> removeBtn;
    };
    std::vector<EffectRow> effectRows_;
    void rebuildEffectRows();
};
