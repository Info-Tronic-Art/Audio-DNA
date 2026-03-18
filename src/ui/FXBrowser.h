#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "effects/EffectLibrary.h"
#include "ui/LookAndFeel.h"
#include <vector>

// FXBrowser: displays all 76+ effects organized by category (8 categories).
// Each category is collapsible. Effects can be dragged onto clip cells or
// into the inspector effect stack. Supports FX Presets nested under effects.
class FXBrowser : public juce::Component
{
public:
    FXBrowser();
    ~FXBrowser() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    // Set the effect library to browse
    void setEffectLibrary(EffectLibrary* lib);

    // Rebuild the display from the effect library
    void refresh();

    // Callback when user activates an effect (double-click or drag)
    std::function<void(const juce::String& effectName)> onEffectActivated;

private:
    EffectLibrary* effectLibrary_ = nullptr;

    // Category display
    struct CategoryInfo
    {
        juce::String name;
        juce::String displayName; // Full word, no abbreviation
        juce::Colour color;
        bool expanded = true;
    };

    struct EffectEntry
    {
        juce::String name;
        juce::String category;
        int categoryIndex = 0;
    };

    std::vector<CategoryInfo> categories_;
    std::vector<EffectEntry> effects_;

    // Scrollable content
    juce::Viewport viewport_;
    class FXListContent;
    std::unique_ptr<FXListContent> listContent_;

    // Search
    juce::TextEditor searchField_;

    void buildCategoryList();
    void toggleCategory(int catIndex);

    static constexpr int kSearchBarHeight = 22;
    static constexpr int kCategoryHeaderHeight = 22;
    static constexpr int kEffectRowHeight = 24;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FXBrowser)
};
