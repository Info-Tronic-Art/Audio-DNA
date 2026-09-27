#include "output/OutputManager.h"
#include "model/AppSettings.h"
#include "render/Renderer.h"
#include "ui/MenuBarModel.h"
#include <algorithm>
#include <iostream>

namespace output
{
OutputManager::OutputManager(SharedFrameSet& frames, Renderer& renderer)
    : frames_(frames), renderer_(renderer)
{
    lastSeen_ = currentDisplays();
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
    reconcileIfDisplaysChanged();   // act on the displays as they are now, not as the last tick saw them
    if (isLive(displayIndex))
        closeDisplay(displayIndex);
    else
        openDisplay(displayIndex);
}

bool OutputManager::openWindow(int displayIndex)
{
    const auto& displays = juce::Desktop::getInstance().getDisplays().displays;
    if (displayIndex < 0 || displayIndex >= displays.size())
        return false;
    const auto& display = displays.getReference(displayIndex);
    const DisplayInfo target = toDisplayInfo(display);
    for (const auto& l : live_)
        if (l.target == target)
            return false;   // already live on this display

    auto window = std::make_unique<OutputWindow>(frames_);
    const OutputWindow* raw = window.get();
    window->onCloseRequested = [this, raw] { closeWindow(raw); };
    window->onDisplaysChanged = [this] { scheduleReconcile(); };   // plan5 C3: the immediate hot-plug trigger
    window->openOnDisplay(display);   // bounds, then visible; ordered front, never made key
    live_.push_back({ std::move(window), target });
    return true;
}

void OutputManager::openDisplay(int displayIndex)
{
    JUCE_ASSERT_MESSAGE_THREAD
    if (shutDown_)
        return;
    reconcileIfDisplaysChanged();
    if (!openWindow(displayIndex))
        return;
    forgetInterrupted(live_.back().target);
    changed();
    persistWanted();
}

void OutputManager::closeDisplay(int displayIndex)
{
    JUCE_ASSERT_MESSAGE_THREAD
    reconcileIfDisplaysChanged();
    const auto displays = currentDisplays();
    if (displayIndex < 0 || displayIndex >= static_cast<int>(displays.size()))
        return;
    for (size_t i = 0; i < live_.size(); ++i)
    {
        if (live_[i].target == displays[static_cast<size_t>(displayIndex)])
        {
            closeLive(i);   // a MANUAL close: the target leaves the wanted set
            changed();
            persistWanted();
            return;
        }
    }
}

void OutputManager::closeAll()
{
    JUCE_ASSERT_MESSAGE_THREAD
    if (live_.empty() && interrupted_.empty())
        return;
    while (!live_.empty())
        closeLive(live_.size() - 1);
    interrupted_.clear();   // the panic also cancels every pending hot-plug reopen
    changed();
    persistWanted();
}

void OutputManager::closeWindow(const OutputWindow* window)
{
    for (size_t i = 0; i < live_.size(); ++i)
    {
        if (live_[i].window.get() == window)
        {
            closeLive(i);
            changed();
            persistWanted();
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
        // onCloseRequested / onDisplaysChanged are left set: this may run INSIDE one of those callbacks, and
        // resetting a std::function destroys the lambda that is executing. A late call finds nothing in live_
        // (close) or schedules a harmless idempotent reconcile.
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

void OutputManager::forgetInterrupted(const DisplayInfo& target)
{
    interrupted_.erase(std::remove(interrupted_.begin(), interrupted_.end(), target), interrupted_.end());
}

void OutputManager::handleAsyncUpdate()
{
    graveyard_.clear();   // ~OutputWindow detaches its GL context first
    if (reconcilePending_)
        reconcile();
}

// ---- plan5 C3: hot-plug ----

void OutputManager::scheduleReconcile()
{
    // Called from inside a window's parentSizeChanged(): only mark and post; the reconcile (which may close that
    // very window) runs on a later message-loop turn. AsyncUpdater coalesces: one pending update at most.
    if (shutDown_)
        return;
    reconcilePending_ = true;
    triggerAsyncUpdate();
}

bool OutputManager::reconcileIfDisplaysChanged()
{
    // JUCE refreshes this array only on the screen-change notification; reading it is a const-ref read, and this
    // comparison allocates nothing.
    const auto& displays = juce::Desktop::getInstance().getDisplays().displays;
    bool same = displays.size() == static_cast<int>(lastSeen_.size());
    for (int i = 0; same && i < displays.size(); ++i)
        same = toDisplayInfo(displays.getReference(i)) == lastSeen_[static_cast<size_t>(i)];
    if (same)
        return false;
    reconcile();
    return true;
}

void OutputManager::pollDisplays()
{
    JUCE_ASSERT_MESSAGE_THREAD
    if (shutDown_ || !pollEnabled_.load(std::memory_order_relaxed))
        return;
    pollTicks_.fetch_add(1, std::memory_order_relaxed);
    reconcileIfDisplaysChanged();
}

void OutputManager::reconcile()
{
    JUCE_ASSERT_MESSAGE_THREAD
    reconcilePending_ = false;
    if (shutDown_)
        return;
    reconciles_.fetch_add(1, std::memory_order_relaxed);

    const auto current = currentDisplays();
    std::vector<DisplayInfo> liveTargets;
    for (const auto& l : live_)
        liveTargets.push_back(l.target);
    const auto diff = diffOutputs(liveTargets, interrupted_, current, lastSeen_);
    lastSeen_ = current;

    // 1. A live output whose display moved or changed mode follows it (live indices unchanged).
    for (const auto& r : diff.toRebound)
    {
        auto& l = live_[static_cast<size_t>(r.from)];
        l.target = current[static_cast<size_t>(r.display)];
        l.window->setBounds(juce::Rectangle<int>(l.target.x, l.target.y, l.target.w, l.target.h));
    }

    // 2. Q6: an interrupted target whose display is back opens again (appended to live_: no live index moves).
    std::vector<int> reopened;
    for (const auto& o : diff.toOpen)
        if (openWindow(o.display))
            reopened.push_back(o.from);
    std::sort(reopened.rbegin(), reopened.rend());
    for (const int i : reopened)
        interrupted_.erase(interrupted_.begin() + i);

    // 3. A live output whose display is gone closes; its target is kept (interrupted) -- a hot-plug close is not a
    //    decision to stop that output. Highest index first, so the others stay valid.
    auto closing = diff.toClose;
    std::sort(closing.rbegin(), closing.rend());
    for (const int i : closing)
    {
        const auto target = live_[static_cast<size_t>(i)].target;
        if (std::find(interrupted_.begin(), interrupted_.end(), target) == interrupted_.end())
            interrupted_.push_back(target);
        closeLive(static_cast<size_t>(i));
    }

    // The menu ticks, the TopBar count and /api/state.outputs(.displays) are rebuilt on EVERY reconcile.
    if (diff.empty())
        rebuildState();
    else
        changed();
    persistWanted();
}

// ---- plan5 C3: persistence + restore ----

void OutputManager::attachSettings(const juce::File& settingsFile)
{
    JUCE_ASSERT_MESSAGE_THREAD
    settingsFile_ = settingsFile;
    saved_ = wantedFromVar(AppSettings(settingsFile_).read(AppSettings::kOutputs));   // loaded only: opens NOTHING (Q1)
    rebuildState();
}

void OutputManager::persistWanted()
{
    std::vector<DisplayInfo> wanted;
    for (const auto& l : live_)
        wanted.push_back(l.target);
    for (const auto& t : interrupted_)
        if (std::find(wanted.begin(), wanted.end(), t) == wanted.end())
            wanted.push_back(t);
    if (sameTargets(wanted, lastWanted_))
        return;   // never a write without a change (the poll never gets here with nothing changed)
    lastWanted_ = wanted;
    if (settingsFile_ == juce::File())
        return;
    if (AppSettings(settingsFile_).update(AppSettings::kOutputs, wantedToVar(wanted)))
        settingsWrites_.fetch_add(1, std::memory_order_relaxed);
    else
        std::cerr << "[OutputManager] could not write " << settingsFile_.getFullPathName() << std::endl;
}

std::vector<OutputDiff::Pair> OutputManager::restorePlan() const
{
    std::vector<OutputDiff::Pair> plan;
    const auto current = currentDisplays();
    auto used = liveFlags(current);   // a display that already shows an output is never opened twice
    for (size_t s = 0; s < saved_.size(); ++s)
    {
        if (const auto i = matchDisplay(saved_[s], current, used, MatchRule::Reopen))
        {
            used[static_cast<size_t>(*i)] = true;
            plan.push_back({ static_cast<int>(s), *i });
        }
    }
    return plan;
}

int OutputManager::restorableCount() const
{
    return static_cast<int>(restorePlan().size());
}

void OutputManager::restoreLast()
{
    JUCE_ASSERT_MESSAGE_THREAD
    restoreCalls_.fetch_add(1, std::memory_order_relaxed);
    if (shutDown_)
        return;
    reconcileIfDisplaysChanged();
    const auto plan = restorePlan();
    if (plan.empty())
        return;   // nothing saved, or none of its displays is connected and free: opens nothing
    std::vector<int> restored;
    for (const auto& p : plan)
    {
        if (openWindow(p.display))
        {
            forgetInterrupted(live_.back().target);
            restored.push_back(p.from);
        }
    }
    std::sort(restored.rbegin(), restored.rend());
    for (const int s : restored)
        saved_.erase(saved_.begin() + s);
    changed();
    persistWanted();
}

// ---- views ----

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
                                             C::kOutputFullscreenBase, C::kOutputDisabled,
                                             C::kOutputRestoreLast, restorableCount() > 0),
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
    interruptedCount_.store(static_cast<int>(interrupted_.size()), std::memory_order_relaxed);
    savedCount_.store(static_cast<int>(saved_.size()), std::memory_order_relaxed);
    restorable_.store(restorableCount(), std::memory_order_relaxed);
    const juce::var fresh(arr);   // a NEW array every time: a copy an HTTP thread holds is never mutated
    std::lock_guard<std::mutex> lock(stateMutex_);
    stateSnapshot_ = fresh;
}

juce::var OutputManager::stateVar() const
{
    std::lock_guard<std::mutex> lock(stateMutex_);
    return stateSnapshot_;
}

juce::var OutputManager::statsVar() const
{
    auto* o = new juce::DynamicObject();
    o->setProperty("poll_enabled", pollEnabled_.load(std::memory_order_relaxed));
    o->setProperty("poll_ticks", pollTicks_.load(std::memory_order_relaxed));
    o->setProperty("reconciles", reconciles_.load(std::memory_order_relaxed));
    o->setProperty("settings_writes", settingsWrites_.load(std::memory_order_relaxed));
    o->setProperty("restore_calls", restoreCalls_.load(std::memory_order_relaxed));
    o->setProperty("interrupted", interruptedCount_.load(std::memory_order_relaxed));
    o->setProperty("saved", savedCount_.load(std::memory_order_relaxed));
    o->setProperty("restorable", restorable_.load(std::memory_order_relaxed));
    return juce::var(o);
}

void OutputManager::setPollEnabled(bool enabled)
{
    pollEnabled_.store(enabled, std::memory_order_relaxed);
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
