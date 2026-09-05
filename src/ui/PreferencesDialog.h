#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "ui/LookAndFeel.h"

// PreferencesDialog: settings dialog.
// Tabs: General, Video, About. (Only wired controls are exposed.)
// Settings are stored in a JSON file in the app's user data directory.
class PreferencesDialog : public juce::DialogWindow
{
public:
    PreferencesDialog(bool tooltipsEnabled, std::function<void(bool)> onTooltipToggled,
                      const juce::String& milkDropDir,
                      std::function<void(juce::String)> onMilkDropDirChanged);
    ~PreferencesDialog() override = default;

    void closeButtonPressed() override;

    // Show the dialog (centered on parent). `tooltipsEnabled` seeds the toggle
    // to the caller's current state; `onTooltipToggled` fires on every change.
    // `milkDropDir` seeds the Video tab's preset folder field; `onMilkDropDirChanged`
    // fires when the user commits a new (valid, or cleared) directory.
    static void show(juce::Component* parent, bool tooltipsEnabled,
                     std::function<void(bool)> onTooltipToggled,
                     const juce::String& milkDropDir,
                     std::function<void(juce::String)> onMilkDropDirChanged);

private:
    class Content : public juce::Component
    {
    public:
        Content(bool tooltipsEnabled, std::function<void(bool)> onTooltipToggled,
               const juce::String& milkDropDir,
               std::function<void(juce::String)> onMilkDropDirChanged);

        void paint(juce::Graphics& g) override;
        void resized() override;

        enum class Tab : int
        {
            General = 0, Video, About
        };
        void setActiveTab(Tab tab);

    private:
        Tab activeTab_ = Tab::General;
        static constexpr int kTabBarHeight = 30;

        // Tab buttons
        juce::TextButton generalBtn_{"General"};
        juce::TextButton videoBtn_{"Video"};
        juce::TextButton aboutBtn_{"About"};

        // General tab
        juce::Label tooltipLabel_{"", "Show Tooltips:"};
        juce::ToggleButton tooltipToggle_;

    public:
        std::function<void(bool enabled)> onTooltipToggled;
        std::function<void(juce::String dir)> onMilkDropDirChanged;
    private:

        // MilkDrop preset directory (in Video tab)
        juce::Label milkDropDirLabel_{"", "MilkDrop Presets:"};
        juce::TextEditor milkDropDirEdit_;
        juce::TextButton milkDropBrowseBtn_{"Browse..."};

        // About tab
        juce::Label versionLabel_;
        juce::Label creditsLabel_;

        // Content viewport
        juce::Viewport contentViewport_;
        std::unique_ptr<juce::Component> contentPanel_;

        void updateTabButtonColors();
        void showActiveTab();
        void layoutGeneralTab(juce::Rectangle<int> area);
        void layoutVideoTab(juce::Rectangle<int> area);
        void layoutAboutTab(juce::Rectangle<int> area);
        // Reads milkDropDirEdit_, fires onMilkDropDirChanged if the text is
        // empty (clears the pref) or an existing directory; silently
        // ignores an invalid typed path (matches FilesBrowser::pathBar_'s
        // onReturnKey convention).
        void commitMilkDropDir();
    };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PreferencesDialog)
};
