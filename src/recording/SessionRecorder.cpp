#include "SessionRecorder.h"

double SessionRecorder::getCurrentTime() const
{
    return juce::Time::getMillisecondCounterHiRes() / 1000.0;
}

void SessionRecorder::startRecording()
{
    const juce::ScopedLock sl(lock_);
    events_.clear();
    recordStartTime_ = getCurrentTime();
    recording_.store(true, std::memory_order_relaxed);
}

void SessionRecorder::stopRecording()
{
    recording_.store(false, std::memory_order_relaxed);
}

void SessionRecorder::recordParameterChange(uint32_t targetId, uint32_t paramIndex, float value)
{
    if (!recording_.load(std::memory_order_relaxed)) return;
    const juce::ScopedLock sl(lock_);
    Event e;
    e.timestamp = getCurrentTime() - recordStartTime_;
    e.type = EventType::ParameterChange;
    e.targetId = targetId;
    e.paramIndex = paramIndex;
    e.value = value;
    events_.push_back(e);
}

void SessionRecorder::recordClipTrigger(int layerIndex, int columnIndex)
{
    if (!recording_.load(std::memory_order_relaxed)) return;
    const juce::ScopedLock sl(lock_);
    Event e;
    e.timestamp = getCurrentTime() - recordStartTime_;
    e.type = EventType::ClipTrigger;
    e.layerIndex = layerIndex;
    e.columnIndex = columnIndex;
    events_.push_back(e);
}

void SessionRecorder::recordColumnTrigger(int columnIndex)
{
    if (!recording_.load(std::memory_order_relaxed)) return;
    const juce::ScopedLock sl(lock_);
    Event e;
    e.timestamp = getCurrentTime() - recordStartTime_;
    e.type = EventType::ColumnTrigger;
    e.columnIndex = columnIndex;
    events_.push_back(e);
}

void SessionRecorder::recordMacroChange(uint32_t macroIndex, float value)
{
    if (!recording_.load(std::memory_order_relaxed)) return;
    const juce::ScopedLock sl(lock_);
    Event e;
    e.timestamp = getCurrentTime() - recordStartTime_;
    e.type = EventType::MacroChange;
    e.paramIndex = macroIndex;
    e.value = value;
    events_.push_back(e);
}

void SessionRecorder::recordTransportChange(const std::string& action, float value)
{
    if (!recording_.load(std::memory_order_relaxed)) return;
    const juce::ScopedLock sl(lock_);
    Event e;
    e.timestamp = getCurrentTime() - recordStartTime_;
    e.type = EventType::TransportChange;
    e.action = action;
    e.value = value;
    events_.push_back(e);
}

void SessionRecorder::recordEffectToggle(const std::string& effectName, bool enabled)
{
    if (!recording_.load(std::memory_order_relaxed)) return;
    const juce::ScopedLock sl(lock_);
    Event e;
    e.timestamp = getCurrentTime() - recordStartTime_;
    e.type = EventType::EffectToggle;
    e.effectName = effectName;
    e.enabled = enabled;
    events_.push_back(e);
}

void SessionRecorder::recordCuepointJump(uint32_t clipId, int cuepointIndex, float position)
{
    if (!recording_.load(std::memory_order_relaxed)) return;
    const juce::ScopedLock sl(lock_);
    Event e;
    e.timestamp = getCurrentTime() - recordStartTime_;
    e.type = EventType::CuepointJump;
    e.targetId = clipId;
    e.paramIndex = static_cast<uint32_t>(cuepointIndex);
    e.value = position;
    events_.push_back(e);
}

// === Playback ===

void SessionRecorder::startPlayback()
{
    playbackTime_ = 0.0;
    playbackIndex_ = 0;
    playing_.store(true, std::memory_order_relaxed);
}

void SessionRecorder::stopPlayback()
{
    playing_.store(false, std::memory_order_relaxed);
}

std::vector<const SessionRecorder::Event*> SessionRecorder::advancePlayback(double dt)
{
    std::vector<const Event*> result;
    if (!playing_.load(std::memory_order_relaxed)) return result;

    playbackTime_ += dt;

    // Fire all events up to current playback time
    while (playbackIndex_ < events_.size() &&
           events_[playbackIndex_].timestamp <= playbackTime_)
    {
        result.push_back(&events_[playbackIndex_]);
        ++playbackIndex_;
    }

    // Auto-stop at end
    if (playbackIndex_ >= events_.size())
        playing_.store(false, std::memory_order_relaxed);

    return result;
}

// === Session management ===

int SessionRecorder::getNumEvents() const
{
    return static_cast<int>(events_.size());
}

double SessionRecorder::getDuration() const
{
    if (events_.empty()) return 0.0;
    return events_.back().timestamp;
}

void SessionRecorder::clear()
{
    const juce::ScopedLock sl(lock_);
    events_.clear();
    playbackIndex_ = 0;
    playbackTime_ = 0.0;
}

// === JSON serialization ===

bool SessionRecorder::saveToFile(const juce::File& file) const
{
    juce::Array<juce::var> eventArray;

    for (const auto& e : events_)
    {
        auto* obj = new juce::DynamicObject();
        obj->setProperty("t", e.timestamp);
        obj->setProperty("type", static_cast<int>(e.type));

        switch (e.type)
        {
            case EventType::ParameterChange:
                obj->setProperty("targetId", static_cast<int>(e.targetId));
                obj->setProperty("paramIndex", static_cast<int>(e.paramIndex));
                obj->setProperty("value", static_cast<double>(e.value));
                break;
            case EventType::ClipTrigger:
                obj->setProperty("layer", e.layerIndex);
                obj->setProperty("column", e.columnIndex);
                break;
            case EventType::ColumnTrigger:
                obj->setProperty("column", e.columnIndex);
                break;
            case EventType::MacroChange:
                obj->setProperty("macroIndex", static_cast<int>(e.paramIndex));
                obj->setProperty("value", static_cast<double>(e.value));
                break;
            case EventType::TransportChange:
                obj->setProperty("action", juce::String(e.action));
                obj->setProperty("value", static_cast<double>(e.value));
                break;
            case EventType::EffectToggle:
                obj->setProperty("effect", juce::String(e.effectName));
                obj->setProperty("enabled", e.enabled);
                break;
            case EventType::CuepointJump:
                obj->setProperty("clipId", static_cast<int>(e.targetId));
                obj->setProperty("cuepointIndex", static_cast<int>(e.paramIndex));
                obj->setProperty("position", static_cast<double>(e.value));
                break;
        }

        eventArray.add(juce::var(obj));
    }

    auto* root = new juce::DynamicObject();
    root->setProperty("version", 1);
    root->setProperty("events", eventArray);

    auto json = juce::JSON::toString(juce::var(root));
    return file.replaceWithText(json);
}

bool SessionRecorder::loadFromFile(const juce::File& file)
{
    auto json = juce::JSON::parse(file);
    auto* root = json.getDynamicObject();
    if (!root) return false;

    auto* eventArray = root->getProperty("events").getArray();
    if (!eventArray) return false;

    const juce::ScopedLock sl(lock_);
    events_.clear();

    for (const auto& ev : *eventArray)
    {
        auto* obj = ev.getDynamicObject();
        if (!obj) continue;

        Event e;
        e.timestamp = static_cast<double>(obj->getProperty("t"));
        e.type = static_cast<EventType>(static_cast<int>(obj->getProperty("type")));

        switch (e.type)
        {
            case EventType::ParameterChange:
                e.targetId = static_cast<uint32_t>(static_cast<int>(obj->getProperty("targetId")));
                e.paramIndex = static_cast<uint32_t>(static_cast<int>(obj->getProperty("paramIndex")));
                e.value = static_cast<float>(static_cast<double>(obj->getProperty("value")));
                break;
            case EventType::ClipTrigger:
                e.layerIndex = static_cast<int>(obj->getProperty("layer"));
                e.columnIndex = static_cast<int>(obj->getProperty("column"));
                break;
            case EventType::ColumnTrigger:
                e.columnIndex = static_cast<int>(obj->getProperty("column"));
                break;
            case EventType::MacroChange:
                e.paramIndex = static_cast<uint32_t>(static_cast<int>(obj->getProperty("macroIndex")));
                e.value = static_cast<float>(static_cast<double>(obj->getProperty("value")));
                break;
            case EventType::TransportChange:
                e.action = obj->getProperty("action").toString().toStdString();
                e.value = static_cast<float>(static_cast<double>(obj->getProperty("value")));
                break;
            case EventType::EffectToggle:
                e.effectName = obj->getProperty("effect").toString().toStdString();
                e.enabled = static_cast<bool>(obj->getProperty("enabled"));
                break;
            case EventType::CuepointJump:
                e.targetId = static_cast<uint32_t>(static_cast<int>(obj->getProperty("clipId")));
                e.paramIndex = static_cast<uint32_t>(static_cast<int>(obj->getProperty("cuepointIndex")));
                e.value = static_cast<float>(static_cast<double>(obj->getProperty("position")));
                break;
        }

        events_.push_back(std::move(e));
    }

    playbackIndex_ = 0;
    playbackTime_ = 0.0;
    return true;
}
