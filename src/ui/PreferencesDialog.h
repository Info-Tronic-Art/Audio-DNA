#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "ui/LookAndFeel.h"

// PreferencesDialog: 8-tab settings dialog.
// Tabs: General, Audio, Video, MIDI, Recording, Defaults, Feedback, About.
// Settings are stored in a JSON file in the app's user data directory.
class PreferencesDialog : public juce::DialogWindow
{
public:
    PreferencesDialog();
    ~PreferencesDialog() override = default;

    void closeButtonPressed() override;

    // Show the dialog modally (centered on parent)
    static void show(juce::Component* parent);

private:
    class Content : public juce::Component
    {
    public:
        Content();

        void paint(juce::Graphics& g) override;
        void resized() override;

        enum class Tab : int
        {
            General = 0, Audio, Video, MIDI,
            Recording, Defaults, Feedback, About
        };
        void setActiveTab(Tab tab);

    private:
        Tab activeTab_ = Tab::General;
        static constexpr int kTabBarHeight = 30;

        // Tab buttons
        juce::TextButton generalBtn_{"General"};
        juce::TextButton audioBtn_{"Audio"};
        juce::TextButton videoBtn_{"Video"};
        juce::TextButton midiBtn_{"MIDI"};
        juce::TextButton recordingBtn_{"Recording"};
        juce::TextButton defaultsBtn_{"Defaults"};
        juce::TextButton feedbackBtn_{"Feedback"};
        juce::TextButton aboutBtn_{"About"};

        // General tab
        juce::Label quitConfirmLabel_{"", "Confirm on quit:"};
        juce::ToggleButton quitConfirmToggle_;
        juce::Label tooltipLabel_{"", "Show Tooltips:"};
        juce::ToggleButton tooltipToggle_;

    public:
        std::function<void(bool enabled)> onTooltipToggled;
    private:

        // Audio tab
        juce::Label sampleRateLabel_{"", "Sample Rate:"};
        juce::ComboBox sampleRateSelector_;
        juce::Label bufferSizeLabel_{"", "Buffer Size:"};
        juce::ComboBox bufferSizeSelector_;
        juce::Label bpmRangeLabel_{"", "BPM Detection Range:"};
        juce::ComboBox bpmRangeSelector_;

        // Video tab
        juce::Label fpsTargetLabel_{"", "FPS Target:"};
        juce::ComboBox fpsTargetSelector_;
        juce::Label renderResLabel_{"", "Render Resolution:"};
        juce::ComboBox renderResSelector_;

        // About tab
        juce::Label versionLabel_;
        juce::Label creditsLabel_;

        // Content viewport
        juce::Viewport contentViewport_;
        std::unique_ptr<juce::Component> contentPanel_;

        void updateTabButtonColors();
        void showActiveTab();
        void layoutGeneralTab(juce::Rectangle<int> area);
        void layoutAudioTab(juce::Rectangle<int> area);
        void layoutVideoTab(juce::Rectangle<int> area);
        void layoutAboutTab(juce::Rectangle<int> area);
        void layoutPlaceholderTab(juce::Rectangle<int> area, const juce::String& tabName);
    };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PreferencesDialog)
};
