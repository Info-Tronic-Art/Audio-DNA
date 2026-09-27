#include "output/OutputManager.h"
#include "render/Renderer.h"
#include "ui/MenuBarModel.h"

namespace output
{
OutputManager::OutputManager(SharedFrameSet& frames, Renderer& renderer)
    : frames_(frames), renderer_(renderer)
{
    rebuildState();
}

OutputManager::~OutputManager()
{
    shutdown();
}

DisplayInfo OutputManager::toDisplayInfo(const juce::Displays::Display& d)
{
    return { d.totalArea.getX(), d.totalArea.getY(), d.totalArea.getWidth(), d.totalArea.getHeight(), d.scale, d.isMain };
}

std::vector<DisplayInfo> OutputManager::currentDisplays()
{
    std::vector<DisplayInfo> out;
    for (const auto& d : juce::Desktop::getInstance().getDisplays().displays)
        out.push_back(toDisplayInfo(d));
    return out;
}

std::vector<bool> OutputManager::liveFlags(const std::vector<DisplayInfo>& displays) const
{
    std::vector<bool> flags(displays.size(), false);
    for (size_t i = 0; i < displays.size(); ++i)
        for (const auto& l : live_)
            if (l.target == displays[i])
                flags[i] = true;
    return flags;
}

bool OutputManager::isLive(int displayIndex) const
{
    const auto displays = currentDisplays();
    if (displayIndex < 0 || displayIndex >= static_cast<int>(displays.size()))
        return false;
    return liveFlags(displays)[static_cast<size_t>(displayIndex)];
}

int OutputManager::mainDisplayIndex() const
{
    const auto displays = currentDisplays();
    for (size_t i = 0; i < displays.size(); ++i)
        if (displays[i].isMain)
            return static_cast<int>(i);
    return displays.empty() ? -1 : 0;
}

void OutputManager::toggleDisplay(int displayIndex)
{
    if (isLive(displayIndex))
        closeDisplay(displayIndex);
    else
        openDisplay(displayIndex);
}

void OutputManager::openDisplay(int displayIndex)
{
    JUCE_ASSERT_MESSAGE_THREAD
    if (shutDown_)
        return;
    const auto& displays = juce::Desktop::getInstance().getDisplays().displays;
    if (displayIndex < 0 || displayIndex >= static_cast<int>(displays.size()))
        return;
    const auto& display = displays[static_cast<size_t>(displayIndex)];
    const DisplayInfo target = toDisplayInfo(display);
    for (const auto& l : live_)
        if (l.target == target)
            return;   // already live on this display

    auto window = std::make_unique<OutputWindow>(frames_);
    const OutputWindow* raw = window.get();
    window->onCloseRequested = [this, raw] { closeWindow(raw); };
    window->openOnDisplay(display);   // bounds, then visible; ordered front, never made key
    live_.push_back({ std::move(window), target });
    changed();
}

void OutputManager::closeDisplay(int displayIndex)
{
    JUCE_ASSERT_MESSAGE_THREAD
    const auto displays = currentDisplays();
    if (displayIndex < 0 || displayIndex >= static_cast<int>(displays.size()))
        return;
    for (size_t i = 0; i < live_.size(); ++i)
    {
        if (live_[i].target == displays[static_cast<size_t>(displayIndex)])
        {
            closeLive(i);
            changed();
            return;
        }
    }
}

void OutputManager::closeAll()
{
    JUCE_ASSERT_MESSAGE_THREAD
    if (live_.empty())
        return;
    while (!live_.empty())
        closeLive(live_.size() - 1);
    changed();
}

void OutputManager::closeWindow(const OutputWindow* window)
{
    for (size_t i = 0; i < live_.size(); ++i)
    {
        if (live_[i].window.get() == window)
        {
            closeLive(i);
            changed();
            return;
        }
    }
}

void OutputManager::closeLive(size_t index)
{
    auto window = std::move(live_[index].window);
    live_.erase(live_.begin() + static_cast<std::ptrdiff_t>(index));
    if (window != nullptr)
    {
        window->onCloseRequested = nullptr;
        window->setVisible(false);
        destroyLater(std::move(window));
    }
}

void OutputManager::destroyLater(std::unique_ptr<OutputWindow> window)
{
    // Never delete a window inside one of its own callbacks (closeButtonPressed -> onCloseRequested lands here).
    graveyard_.push_back(std::move(window));
    triggerAsyncUpdate();
}

void OutputManager::handleAsyncUpdate()
{
    graveyard_.clear();   // ~OutputWindow detaches its GL context first
}

void OutputManager::changed()
{
    const int n = liveCount();
    renderer_.setLiveOutputCount(n);   // the canvas tap runs only while an output is live
    rebuildState();
    if (onLiveCountChanged)
        onLiveCountChanged(n);
}

void OutputManager::populateMenu(juce::PopupMenu& menu) const
{
    using C = AudioDNAMenuBar::CommandID;
    const auto displays = currentDisplays();
    addOutputMenuItems(menu, buildOutputMenu(displays, liveFlags(displays), liveCount(),
                                             C::kOutputFullscreenBase, C::kOutputDisabled),
                       C::kOutputDisabled);
}

void OutputManager::rebuildState()
{
    using C = AudioDNAMenuBar::CommandID;
    const auto displays = currentDisplays();
    const auto items = buildOutputMenu(displays, liveFlags(displays), liveCount(),
                                       C::kOutputFullscreenBase, C::kOutputDisabled);
    juce::Array<juce::var> arr;
    for (size_t i = 0; i < displays.size(); ++i)
    {
        const auto& d = displays[i];
        auto* o = new juce::DynamicObject();
        o->setProperty("index", static_cast<int>(i));
        o->setProperty("x", d.x);
        o->setProperty("y", d.y);
        o->setProperty("w", d.w);
        o->setProperty("h", d.h);
        o->setProperty("scale", d.scale);
        o->setProperty("main", d.isMain);
        o->setProperty("live", items[i].ticked);
        o->setProperty("label", items[i].label);   // the exact Output-menu title of this display
        arr.add(juce::var(o));
    }
    const juce::var fresh(arr);   // a NEW array every time: a copy an HTTP thread holds is never mutated
    std::lock_guard<std::mutex> lock(stateMutex_);
    stateSnapshot_ = fresh;
}

juce::var OutputManager::stateVar() const
{
    std::lock_guard<std::mutex> lock(stateMutex_);
    return stateSnapshot_;
}

void OutputManager::shutdown()
{
    if (shutDown_)
        return;
    shutDown_ = true;
    onLiveCountChanged = nullptr;
    cancelPendingUpdate();
    for (auto& l : live_)
        if (l.window != nullptr)
            l.window->detachGL();
    for (auto& w : graveyard_)
        if (w != nullptr)
            w->detachGL();
    live_.clear();
    graveyard_.clear();
    renderer_.setLiveOutputCount(0);
}
} // namespace output
