#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_gui_extra/juce_gui_extra.h>
#include "keyboard/KeySlot.h"
#include "effects/EffectLibrary.h"

// KeyEditor: full-height panel for configuring a keyboard slot.
// Layout: left column (media/keying/playback), center (scrollable effects),
// right (image preview with background selector).
class KeyEditor : public juce::Component
{
public:
    KeyEditor(EffectLibrary& effectLibrary);

    void setKey(KeySlot* key);
    KeySlot* getKey() const { return currentKey_; }

    void paint(juce::Graphics& g) override;
    void resized() override;
    bool keyPressed(const juce::KeyPress& key) override;

    std::function<void(KeySlot& key)> onRequestImage;
    std::function<void()> onClose;

private:
    void refreshFromKey();
    void updatePreviewImage();

    EffectLibrary& effectLibrary_;
    KeySlot* currentKey_ = nullptr;

    // Header
    juce::Label titleLabel_;
    juce::TextButton closeButton_{"X"};

    // === Left column: Media + Keying + Playback ===
    juce::TextButton openImageBtn_{"Open Image"};
    juce::TextButton clearMediaBtn_{"Clear"};
    juce::Label mediaInfoLabel_;

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

    class ColorSwatch : public juce::Component, public juce::ChangeListener
    {
    public:
        juce::Colour colour{0, 255, 0};
        std::function<void(juce::Colour)> onColourChanged;
        void paint(juce::Graphics& g) override;
        void mouseDown(const juce::MouseEvent&) override;
        void changeListenerCallback(juce::ChangeBroadcaster* source) override;
    };
    ColorSwatch chromaColorSwatch_;
    juce::Label chromaColorLabel_{"", "Key Color"};
    juce::Slider chromaToleranceSlider_;
    juce::Label chromaToleranceLabel_{"", "Tolerance"};
    juce::Slider chromaSoftnessSlider_;
    juce::Label chromaSoftnessLabel_{"", "Softness"};

    juce::Label playbackLabel_{"", "Playback"};
    juce::ToggleButton latchToggle_{"Latch"};
    juce::ToggleButton ignoreRandomToggle_{"Ignore Random"};
    juce::Label randomBeatLabel1_{"", "Override Global"};
    juce::Label randomBeatLabel2_{"", "Random Beats"};
    juce::ComboBox randomBeatSelector_;

    // === Center column: Effects (scrollable) ===
    juce::Label effectsLabel_{"", "Effects"};
    juce::ComboBox addEffectSelector_;
    juce::TextButton clearEffectsBtn_{"Clear All Effects"};

    // Scrollable effects content
    class EffectsContent : public juce::Component
    {
    public:
        int getRequiredHeight() const { return requiredHeight_; }
        int requiredHeight_ = 0;
    };
    EffectsContent effectsContent_;
    juce::Viewport effectsViewport_;

    struct EffectRow
    {
        std::unique_ptr<juce::ToggleButton> enableBtn;
        std::unique_ptr<juce::Label> nameLabel;
        std::unique_ptr<juce::TextButton> removeBtn;
        struct ParamRow
        {
            std::unique_ptr<juce::Label> label;
            std::unique_ptr<juce::Slider> slider;
            std::unique_ptr<juce::TextButton> mapBtn;     // "M" button
            std::unique_ptr<juce::ComboBox> sourceCombo;   // Audio source selector
            std::unique_ptr<juce::ComboBox> curveCombo;    // Curve selector
            bool mappingExpanded = false;
        };
        std::vector<ParamRow> paramRows;
    };
    std::vector<EffectRow> effectRows_;
    void rebuildEffectRows();
    void layoutEffectsContent();

    // === Right column: Preview ===
    class PreviewComponent : public juce::Component
    {
    public:
        enum Background { Black, Grey, White, Checker };
        Background bg = Checker;
        juce::Image previewImage;
        void paint(juce::Graphics& g) override;
    };
    PreviewComponent preview_;
    juce::TextButton bgBlackBtn_{"B"};
    juce::TextButton bgGreyBtn_{"G"};
    juce::TextButton bgWhiteBtn_{"W"};
    juce::TextButton bgCheckerBtn_{"C"};
};
