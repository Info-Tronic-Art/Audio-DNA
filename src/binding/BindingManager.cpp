#include "BindingManager.h"
#include <algorithm>

uint32_t BindingManager::addBinding(const Binding& binding)
{
    Binding b = binding;
    b.id = nextId_++;
    bindings_.push_back(b);
    return b.id;
}

bool BindingManager::removeBinding(uint32_t bindingId)
{
    for (size_t i = 0; i < bindings_.size(); ++i)
    {
        if (bindings_[i].id == bindingId)
        {
            bindings_.erase(bindings_.begin() + static_cast<ptrdiff_t>(i));
            return true;
        }
    }
    return false;
}

Binding* BindingManager::getBinding(uint32_t bindingId)
{
    for (auto& b : bindings_)
        if (b.id == bindingId) return &b;
    return nullptr;
}

const Binding* BindingManager::getBinding(uint32_t bindingId) const
{
    for (const auto& b : bindings_)
        if (b.id == bindingId) return &b;
    return nullptr;
}

Binding* BindingManager::getBindingAt(int index)
{
    if (index >= 0 && index < static_cast<int>(bindings_.size()))
        return &bindings_[static_cast<size_t>(index)];
    return nullptr;
}

void BindingManager::clearAll()
{
    bindings_.clear();
    relativeCCValues_.clear();
}

bool BindingManager::processKeyDown(int keyCode, bool shift, bool cmd, bool alt)
{
    if (bindingMode_)
    {
        if (captureCallback_)
            captureCallback_(Binding::InputType::Keyboard, keyCode, 0, shift, cmd, alt);
        return true;
    }

    for (const auto& b : bindings_)
    {
        if (!b.enabled) continue;
        if (b.inputType != Binding::InputType::Keyboard) continue;
        if (b.keyCode == keyCode
            && b.keyModShift == shift
            && b.keyModCmd == cmd
            && b.keyModAlt == alt)
        {
            if (actionCallback_)
                actionCallback_(b, 1.0f);
            return true;
        }
    }
    return false;
}

bool BindingManager::processKeyUp(int keyCode, bool shift, bool cmd, bool alt)
{
    if (bindingMode_) return false;

    for (const auto& b : bindings_)
    {
        if (!b.enabled) continue;
        if (b.inputType != Binding::InputType::Keyboard) continue;

        // For momentary mode, fire on key release with value 0
        // For toggle mode, key-up is ignored (action already fired on key-down)
        if (b.triggerMode != Binding::TriggerMode::Momentary) continue;

        if (b.keyCode == keyCode
            && b.keyModShift == shift
            && b.keyModCmd == cmd
            && b.keyModAlt == alt)
        {
            if (actionCallback_)
                actionCallback_(b, 0.0f);
            return true;
        }
    }
    return false;
}

bool BindingManager::processMidiNoteOn(int channel, int note, int velocity)
{
    if (bindingMode_)
    {
        if (captureCallback_)
            captureCallback_(Binding::InputType::MidiNote, note, 0, false, false, false);
        return true;
    }

    float normalizedVelocity = static_cast<float>(velocity) / 127.0f;

    for (const auto& b : bindings_)
    {
        if (!b.enabled) continue;
        if (b.inputType != Binding::InputType::MidiNote) continue;
        if (b.midiNote == note && (b.midiChannel == 0 || b.midiChannel == channel))
        {
            if (actionCallback_)
                actionCallback_(b, normalizedVelocity);
            return true;
        }
    }
    return false;
}

bool BindingManager::processMidiNoteOff(int channel, int note)
{
    if (bindingMode_) return false;

    for (const auto& b : bindings_)
    {
        if (!b.enabled) continue;
        if (b.inputType != Binding::InputType::MidiNote) continue;

        // For momentary mode, fire on note-off with value 0
        // For toggle mode, note-off is ignored
        if (b.triggerMode != Binding::TriggerMode::Momentary) continue;

        if (b.midiNote == note && (b.midiChannel == 0 || b.midiChannel == channel))
        {
            if (actionCallback_)
                actionCallback_(b, 0.0f);
            return true;
        }
    }
    return false;
}

bool BindingManager::processMidiCC(int channel, int cc, int value)
{
    if (bindingMode_)
    {
        if (captureCallback_)
            captureCallback_(Binding::InputType::MidiCC, 0, cc, false, false, false);
        return true;
    }

    for (const auto& b : bindings_)
    {
        if (!b.enabled) continue;
        if (b.inputType != Binding::InputType::MidiCC) continue;
        if (b.midiCC == cc && (b.midiChannel == 0 || b.midiChannel == channel))
        {
            float outputValue;
            if (b.ccMode == Binding::CCMode::Relative)
            {
                // Relative mode: value < 64 = decrement, > 64 = increment
                // Common encoding: 65 = +1, 63 = -1, 66 = +2, 62 = -2, etc.
                int key = (channel << 8) | cc;
                float current = 0.5f; // Default to middle
                auto it = relativeCCValues_.find(key);
                if (it != relativeCCValues_.end())
                    current = it->second;

                float delta = 0.0f;
                if (value > 64)
                    delta = static_cast<float>(value - 64) * b.ccStepSize;
                else if (value < 64)
                    delta = static_cast<float>(value - 64) * b.ccStepSize;
                // value == 64 means no change

                current = std::clamp(current + delta, 0.0f, 1.0f);
                relativeCCValues_[key] = current;
                outputValue = current;
            }
            else
            {
                // Absolute mode: 0-127 → 0.0-1.0
                outputValue = static_cast<float>(value) / 127.0f;
            }

            if (actionCallback_)
                actionCallback_(b, outputValue);
            return true;
        }
    }
    return false;
}

float BindingManager::getRelativeCCValue(int channel, int cc) const
{
    int key = (channel << 8) | cc;
    auto it = relativeCCValues_.find(key);
    if (it != relativeCCValues_.end())
        return it->second;
    return 0.5f;
}

// P24.10: Binding presets — serialization

juce::var BindingManager::toVar() const
{
    juce::Array<juce::var> arr;
    for (const auto& b : bindings_)
    {
        auto* obj = new juce::DynamicObject();
        obj->setProperty("inputType", static_cast<int>(b.inputType));
        obj->setProperty("keyCode", b.keyCode);
        obj->setProperty("keyModShift", b.keyModShift);
        obj->setProperty("keyModCmd", b.keyModCmd);
        obj->setProperty("keyModAlt", b.keyModAlt);
        obj->setProperty("midiChannel", b.midiChannel);
        obj->setProperty("midiNote", b.midiNote);
        obj->setProperty("midiCC", b.midiCC);
        obj->setProperty("action", static_cast<int>(b.action));
        obj->setProperty("triggerMode", static_cast<int>(b.triggerMode));
        obj->setProperty("ccMode", static_cast<int>(b.ccMode));
        obj->setProperty("ccStepSize", static_cast<double>(b.ccStepSize));
        obj->setProperty("targetMode", static_cast<int>(b.targetMode));
        obj->setProperty("targetClipId", static_cast<int>(b.targetClipId));
        obj->setProperty("velocityToOpacity", b.velocityToOpacity);
        obj->setProperty("targetLayerIndex", b.targetLayerIndex);
        obj->setProperty("targetColumn", b.targetColumn);
        obj->setProperty("targetDeckIndex", b.targetDeckIndex);
        obj->setProperty("targetEffectIndex", b.targetEffectIndex);
        obj->setProperty("targetMacroIndex", b.targetMacroIndex);
        obj->setProperty("enabled", b.enabled);
        arr.add(juce::var(obj));
    }
    auto* root = new juce::DynamicObject();
    root->setProperty("bindings", arr);
    root->setProperty("version", 1);
    return juce::var(root);
}

void BindingManager::fromVar(const juce::var& v)
{
    bindings_.clear();
    relativeCCValues_.clear();

    if (auto* root = v.getDynamicObject())
    {
        if (auto* arr = root->getProperty("bindings").getArray())
        {
            for (const auto& bVar : *arr)
            {
                if (auto* obj = bVar.getDynamicObject())
                {
                    Binding b;
                    b.id = nextId_++;
                    b.inputType = static_cast<Binding::InputType>(static_cast<int>(obj->getProperty("inputType")));
                    b.keyCode = static_cast<int>(obj->getProperty("keyCode"));
                    b.keyModShift = static_cast<bool>(obj->getProperty("keyModShift"));
                    b.keyModCmd = static_cast<bool>(obj->getProperty("keyModCmd"));
                    b.keyModAlt = static_cast<bool>(obj->getProperty("keyModAlt"));
                    b.midiChannel = static_cast<int>(obj->getProperty("midiChannel"));
                    b.midiNote = static_cast<int>(obj->getProperty("midiNote"));
                    b.midiCC = static_cast<int>(obj->getProperty("midiCC"));
                    b.action = static_cast<Binding::Action>(static_cast<int>(obj->getProperty("action")));
                    b.triggerMode = static_cast<Binding::TriggerMode>(static_cast<int>(obj->getProperty("triggerMode")));
                    b.ccMode = static_cast<Binding::CCMode>(static_cast<int>(obj->getProperty("ccMode")));
                    b.ccStepSize = static_cast<float>(static_cast<double>(obj->getProperty("ccStepSize")));
                    b.targetMode = static_cast<Binding::TargetMode>(static_cast<int>(obj->getProperty("targetMode")));
                    b.targetClipId = static_cast<uint32_t>(static_cast<int>(obj->getProperty("targetClipId")));
                    b.velocityToOpacity = static_cast<bool>(obj->getProperty("velocityToOpacity"));
                    b.targetLayerIndex = static_cast<int>(obj->getProperty("targetLayerIndex"));
                    b.targetColumn = static_cast<int>(obj->getProperty("targetColumn"));
                    b.targetDeckIndex = static_cast<int>(obj->getProperty("targetDeckIndex"));
                    b.targetEffectIndex = static_cast<int>(obj->getProperty("targetEffectIndex"));
                    b.targetMacroIndex = static_cast<int>(obj->getProperty("targetMacroIndex"));
                    b.enabled = static_cast<bool>(obj->getProperty("enabled"));
                    bindings_.push_back(b);
                }
            }
        }
    }
}

bool BindingManager::saveToFile(const juce::File& file) const
{
    auto json = juce::JSON::toString(toVar());
    return file.replaceWithText(json);
}

bool BindingManager::loadFromFile(const juce::File& file)
{
    auto json = file.loadFileAsString();
    if (json.isEmpty()) return false;
    auto parsed = juce::JSON::parse(json);
    if (parsed.isVoid()) return false;
    fromVar(parsed);
    return true;
}
