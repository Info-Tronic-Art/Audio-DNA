#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "ui/LookAndFeel.h"
#include <vector>

// SourcesBrowser: procedural generator icons by category.
// 10 Tier 1 generators for launch, organized by type.
// Users drag sources onto clip cells to use as clip content.
class SourcesBrowser : public juce::Component
{
public:
    SourcesBrowser();
    ~SourcesBrowser() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    // Callback when user activates a source
    std::function<void(const juce::String& sourceName)> onSourceActivated;

private:
    struct SourceEntry
    {
        juce::String name;
        juce::String category;
        juce::Colour color;
    };

    struct CategoryInfo
    {
        juce::String name;
        juce::Colour color;
        bool expanded = true;
    };

    std::vector<CategoryInfo> categories_;
    std::vector<SourceEntry> sources_;

    juce::Viewport viewport_;
    class SourceListContent;
    std::unique_ptr<SourceListContent> listContent_;

    void buildSourceList();
    void toggleCategory(int catIndex);

    static constexpr int kCategoryHeaderHeight = 22;
    static constexpr int kSourceRowHeight = 28;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SourcesBrowser)
};
